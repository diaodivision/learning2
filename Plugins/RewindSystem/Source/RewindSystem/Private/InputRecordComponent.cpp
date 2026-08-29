// Fill out your copyright notice in the Description page of Project Settings.


#include "InputRecordComponent.h"
#include "RewindSubsystem.h"
#include "GameFramework/Character.h"
#include "Algo/BinarySearch.h"
#include "Interface/RecordableActorInterface.h"
#include "Containers/ArrayView.h"

// Sets default values for this component's properties
UInputRecordComponent::UInputRecordComponent() : Super()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	bAutoActivate = true;
	// ...
}

URecordedDataPreviewOperationDelegateWrapper* UInputRecordComponent::FindOrTryAddPreviewOperationDelegateWrapper(const FRecordedDataObjectHandle& RecordedDataObjectHandle)
{
	if (GetRecordState() != ERecordState::Recording || !RecordedDataObjectHandle.IsValid()) { return nullptr; }

	//const IRecordedDataObjectInterface* Data = GetRecordDataByHandle(RecordedDataObjectHandle);
	if (!IsRecordDataObjectExist(RecordedDataObjectHandle)) { return nullptr; }

	TObjectPtr<URecordedDataPreviewOperationDelegateWrapper>* WrapperPtr = RecordedDataPreviewOperationDelegateWrapperMap.Find(RecordedDataObjectHandle);
	if (!WrapperPtr)
	{
		WrapperPtr = &RecordedDataPreviewOperationDelegateWrapperMap.Add(RecordedDataObjectHandle, NewObject<URecordedDataPreviewOperationDelegateWrapper>(this));
	}

	return *WrapperPtr;
}

// Called when the game starts
void UInputRecordComponent::BeginPlay()
{
	Super::BeginPlay();

	if (URewindSubsystem* Subsystem{ URewindSystemStatics::GetRewindSubsystem(this) })
	{
		Subsystem->OnPostComponentInitialize(this);
	}
}

void UInputRecordComponent::OnRegister()
{
	Super::OnRegister();

	const ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner)
	{
		FMessageLog("Blueprint").Error(FText::FromString(FString::Printf(
			TEXT("Error: Component %s expected a ACharacter Owner! Current owner was %s, Component destroying!"),
			*GetName(),
			*GetOwner()->GetClass()->GetName()
		)));

		GetWorld()->GetTimerManager().SetTimerForNextTick([WeakThis = MakeWeakObjectPtr<UInputRecordComponent>(this)]() {
			if (!WeakThis.IsValid()) { return; }

			if (AActor* OwnerActor = WeakThis->GetOwner())
			{
				OwnerActor->Modify();
				WeakThis->Modify();

				OwnerActor->RemoveInstanceComponent(WeakThis.Get());
				WeakThis->DestroyComponent();

				OwnerActor->RemoveOwnedComponent(WeakThis.Get());
			}
			});

		return;
	}

	if (!Owner->Implements<URecordableActorInterface>())
	{
		FMessageLog("PIE").Warning(FText::FromString(FString::Printf(
			TEXT("Warning: Component %s expected a Owner that implements Interface URecordableActorInterface!"),
			*GetName()
		)));
	}
}

void UInputRecordComponent::OnOwnerPossess()
{

}

void UInputRecordComponent::OnOwnerUnPossess()
{
	if (GetRecordState() != ERecordState::Rewinding)
	{
		PreviewAllData();
	}
}

FRecordedDataObjectHandle UInputRecordComponent::Record(TUniquePtr<IRecordedDataObjectInterface> RecordedDataBase, TFunctionRef<void(const FRecordedDataObjectHandle&)> PostRecordCallable)
{
	if (!RecordedDataBase) { return {}; }

	RecordedDataBase->Tick = CurrentTick;
	RecordedDataBase->Handle = FRecordedDataObjectHandle{ RecordedDataBase->Tick, *this };

	//RecordedDataBase->PreviewOnHandlePayload(true);

	const ERecordableActionType RecordableActionType{ RecordedDataBase->RecordableActionType };
	const FRecordedDataObjectHandle& OutHandle = RecordedDataBase->Handle;

	PostRecordCallable(OutHandle);
	//if (TObjectPtr<URecordedDataPreviewOperationDelegateWrapper>* Wrapper = RecordedDataPreviewOperationDelegateWrapperMap.Find(OutHandle))
	//{
	//	(*Wrapper)->Broadcast(OutHandle, *RecordedDataBase, true);
	//}
	if (RecordedDataBase->PrepareToPreview())
	{
		RecordedDataBase->Preview(true);
		OnOperationPreviewDelegate.Broadcast(true, RecordedDataBase.Get());
	}

	RecordedDataList.Add(MoveTemp(RecordedDataBase));

	if (RecordableActionType == ERecordableActionType::HasDuration) { AdvanceCurrentTickOnRecordState(); }

	return OutHandle;
}

void UInputRecordComponent::HandleRecordedData()
{
	if (RecordedDataToken.IsSet() && RecordedDataToken.GetValue().IsValid()) { return; }
	if (CurrentTick > GetTickMax()) { return; }

	RecordedDataToken.Reset();

	PreviewData(GetRecordDataByTick(CurrentTick), false);
	HandleDataOfTick(CurrentTick);
	SetCurrentTick(CurrentTick + 1);
}

void UInputRecordComponent::RemoveFromTick(const RewindSystemTickType Tick)
{
	const int32 Begin = Algo::LowerBoundBy(RecordedDataList, Tick,
		[](const TUniquePtr<IRecordedDataObjectInterface>& Data) { return Data->Tick; });

	if (Begin == RecordedDataList.Num() || RecordedDataList[Begin]->Tick != Tick) { return; }

	{
		for (int32 i = Begin; i < RecordedDataList.Num(); i++)
		{
			if (RecordedDataList[i] && RecordedDataPreviewOperationDelegateWrapperMap.Contains(RecordedDataList[i]->Handle))
			{
				RecordedDataPreviewOperationDelegateWrapperMap.Remove(RecordedDataList[i]->Handle);
			}
		}

		RecordedDataList.SetNum(Begin);
	}

	if (Begin > 0)
	{
		RemoveInvalidDataNearToIndex(Begin - 1);
	}
}

int32 UInputRecordComponent::FindPreviousValidIndex(const int32 Index)
{
	if (RecordedDataList.IsEmpty() || !RecordedDataList.IsValidIndex(Index)) { return INDEX_NONE; }

	for (int32 i = Index; i >= 0; --i)
	{
		if (RecordedDataList[i].IsValid())
		{
			return i;
		}
	}

	return INDEX_NONE;
}

void UInputRecordComponent::RemoveInvalidDataNearToIndex(const int32 Index)
{
	if (RecordedDataList.IsEmpty() || !RecordedDataList.IsValidIndex(Index)) { return; }

	const int32 PreviousValidIndex = FindPreviousValidIndex(Index);
	if (PreviousValidIndex == INDEX_NONE)
	{
		RecordedDataList.Empty();
		RecordedDataPreviewOperationDelegateWrapperMap.Empty();
	}
	else
	{
		for (int32 i = PreviousValidIndex + 1; i < RecordedDataList.Num(); i++)
		{
			if (RecordedDataList[i] && RecordedDataPreviewOperationDelegateWrapperMap.Contains(RecordedDataList[i]->Handle))
			{
				RecordedDataPreviewOperationDelegateWrapperMap.Remove(RecordedDataList[i]->Handle);
			}
		}

		RecordedDataList.SetNum(PreviousValidIndex + 1);
	}
	//RecordedDataList.SetNum(PreviousValidIndex == INDEX_NONE ? 0 : PreviousValidIndex + 1);
}

void UInputRecordComponent::PreviewDataOfTick(RewindSystemTickType Tick)
{
	if (CurrentTick == Tick) { return; }

	const RewindSystemTickType Start = FMath::Min(CurrentTick, Tick);
	const RewindSystemTickType End = FMath::Max(CurrentTick, Tick);

	PreviewData(GetRecordDataByTickRange(Start + 1, End), Tick > CurrentTick);

	SetCurrentTick(Tick);
}

void UInputRecordComponent::PreviewToTick(RewindSystemTickType Tick)
{
	PreviewDataOfTick(Tick);
}

void UInputRecordComponent::PreviewAllData()
{
	PreviewData(GetRecordDataByTickRange(0, GetTickMax()), true);
}

void UInputRecordComponent::UndoPreviewAllData()
{
	PreviewData(GetRecordDataByTickRange(RecordedDataList.IsEmpty() ? 0 : RecordedDataList[0]->Tick, GetTickMax()), false);
}

void UInputRecordComponent::AdvanceCurrentTickOnRecordState()
{
	////CurrentTick++;
	//if (RecordedDataList.IsEmpty())
	//{
	//	ResetCurrentTick(GetRecordState());
	//	return;
	//}

	RemoveInvalidDataNearToIndex(RecordedDataList.Num() - 1);

	if (RecordedDataList.IsEmpty())
	{
		ResetCurrentTick(GetRecordState());
	}
	else if (GetCurrentTick() == RecordedDataList.Last()->Tick)
	{
		SetCurrentTick(RecordedDataList.Last()->Tick + 1);
	}
}

void UInputRecordComponent::ClearRecordedData(ERecordState NewState)
{
	RecordedDataList.Empty();
	RecordedDataPreviewOperationDelegateWrapperMap.Empty();
	RecordedDataToken.Reset();
	ResetCurrentTick(NewState);
}

void UInputRecordComponent::OnStateChanged(const ERecordState OldState, const ERecordState NewState)
{
	switch (OldState)
	{
	case ERecordState::Idle:
		break;

	case ERecordState::PreviewPause:
		break;

	case ERecordState::Previewing:
		break;

	case ERecordState::RecordPause:
		break;

	case ERecordState::Recording:
		break;

	case ERecordState::Rewinding:
		break;
	}

	if (/*OldState != ERecordState::Rewinding && */NewState == ERecordState::Idle) { UndoPreviewAllData(); }

	ResetCurrentTick(NewState);
	if (NewState == ERecordState::PreviewPause || NewState == ERecordState::Previewing) { SetCurrentTick(GetTickMax()); }

	switch (NewState)
	{
	case ERecordState::Idle:
		ClearRecordedData(NewState);
		break;

	case ERecordState::RecordPause:
		PreviewAllData();
		break;

	case ERecordState::Recording:
		PreviewAllData();
		break;

	case ERecordState::PreviewPause:

		break;

	case ERecordState::Previewing:
		UndoPreviewAllData();
		break;

	case ERecordState::Rewinding:
		PreviewAllData();
		break;
	}
}

IRecordedDataObjectInterface* UInputRecordComponent::GetRecordDataByHandle(const FRecordedDataObjectHandle& RecordedDataObjectHandle)
{
	if (RecordedDataList.IsEmpty() || !RecordedDataObjectHandle.IsValid()) { return nullptr; }
	if (!RecordedDataList[0] || RecordedDataList[0]->Handle.GetHandle() > RecordedDataObjectHandle.GetHandle()) { return nullptr; }

	TArrayView<TUniquePtr<IRecordedDataObjectInterface>> DataList = GetRecordDataByTick(RecordedDataObjectHandle.GetTick());
	for (const TUniquePtr<IRecordedDataObjectInterface>& DataPtr : DataList)
	{
		if (DataPtr->Handle == RecordedDataObjectHandle) { return DataPtr.Get(); }
	}

	return nullptr;
}

//void UInputRecordComponent::OnStateStart(ERecordState NewState)
//{
//}
//
//void UInputRecordComponent::OnStateEnd(ERecordState OldState)
//{
//
//}

//void UInputRecordComponent::NotifyEndPreivewState()
//{
//	ResetCurrentTick();
//}

void UInputRecordComponent::HandleDataOfTick(RewindSystemTickType Tick)
{
	TArrayView<TUniquePtr<IRecordedDataObjectInterface>> DataList = GetRecordDataByTick(Tick);

	const int32 Begin = Algo::UpperBoundBy(RecordedDataList, Tick,
		[](const TUniquePtr<IRecordedDataObjectInterface>& Data) { return Data->Tick; });

	for (TUniquePtr<IRecordedDataObjectInterface>& Data : DataList)
	{
		//if (ensureAlways(Data.IsValid()))
		if (!Data.IsValid()) { continue; }

		bool bHandlePayloadSuccessful{ false };
		const bool bShouldStopRewindWhenHandleThisUnsuccessful{ Data->ShouldStopRewindWhenHandleThisUnsuccessful() };

		TSharedPtr<IRecordedDataObjectInterface, ESPMode::NotThreadSafe> SharedPtr{ Data.Release() };
		RecordedDataToken = SharedPtr;	
		if (SharedPtr->PrepareToHandleRecordedData()) { bHandlePayloadSuccessful = SharedPtr->ConsumeAndTryHandlePayload(SharedPtr); }

		if (!bHandlePayloadSuccessful && bShouldStopRewindWhenHandleThisUnsuccessful)
		{
			//todo
		}

	}

	for (int i = 0; i < Begin && i < RecordedDataList.Num(); i++)
	{
		if (RecordedDataList[i] && RecordedDataPreviewOperationDelegateWrapperMap.Contains(RecordedDataList[i]->Handle))
		{
			RecordedDataPreviewOperationDelegateWrapperMap.Remove(RecordedDataList[i]->Handle);
		}
	}

	RecordedDataList.RemoveAt(0, Begin, EAllowShrinking::No);
}

void UInputRecordComponent::PreviewData(TArrayView<TUniquePtr<IRecordedDataObjectInterface>> DataList, bool bIsPreview)
{
	if (bIsPreview)
	{
		for (TUniquePtr<IRecordedDataObjectInterface>& Data : DataList)
		{
			//Data->PreviewOnHandlePayload(true);
			if (!Data || !Data->PrepareToPreview()) { continue; }

			//const FRecordedDataObjectHandle& Handle{ Data->Handle };
			//if (TObjectPtr<URecordedDataPreviewOperationDelegateWrapper>* Wrapper = RecordedDataPreviewOperationDelegateWrapperMap.Find(Handle))
			//{
			//	(*Wrapper)->Broadcast(Handle, *Data, true);
			//}
			Data->Preview(true);

			OnOperationPreviewDelegate.Broadcast(true, Data.Get());
		}
	}
	else
	{
		for (auto It = DataList.rbegin(); It != DataList.rend(); ++It)
		{
			//(*It)->PreviewOnHandlePayload(false);
			TUniquePtr<IRecordedDataObjectInterface>& Data{ *It };

			if (!Data || !Data->PrepareToPreview()) { continue; }

			//const FRecordedDataObjectHandle& Handle = Data->Handle;

			//if (TObjectPtr<URecordedDataPreviewOperationDelegateWrapper>* Wrapper = RecordedDataPreviewOperationDelegateWrapperMap.Find(Handle))
			//{
			//	(*Wrapper)->Broadcast(Handle, *Data, false);
			//}
			Data->Preview(false);
			OnOperationPreviewDelegate.Broadcast(false, Data.Get());
		}
	}
}

//void UInputRecordComponent::FreezeActorMotivation(AActor* Actor, bool bIsFreeze)
//{
//
//}

TArrayView<TUniquePtr<IRecordedDataObjectInterface>> UInputRecordComponent::GetRecordDataByTick(RewindSystemTickType Tick)
{
	return GetRecordDataByTickRange(Tick, Tick);
	//TArrayView<TUniquePtr<IRecordedDataObjectInterface>> Result;

	////return Result;
	//if (RecordedDataList.Num() == 0 || RecordedDataList.Last()->Tick < Tick) { return Result; }

	//int32 Begin = Algo::LowerBoundBy(RecordedDataList, Tick,
	//	[](const TUniquePtr<IRecordedDataObjectInterface>& Data) { return Data->Tick; });

	//int32 End = Algo::UpperBoundBy(RecordedDataList, Tick,
	//	[](const TUniquePtr<IRecordedDataObjectInterface>& Data) { return Data->Tick; });

	//if (Begin == INDEX_NONE || End == INDEX_NONE) { return Result; }

	//if (!ensureAlways(RecordedDataList[Begin]->Tick == Tick && RecordedDataList[Begin]->Tick == RecordedDataList[End - 1]->Tick)) { return Result; }

	//Result = MakeArrayView(RecordedDataList.GetData() + Begin, End - Begin);

	//return Result;
}

TArrayView<TUniquePtr<IRecordedDataObjectInterface>> UInputRecordComponent::GetRecordDataByTickRange(RewindSystemTickType BeginTick, RewindSystemTickType EndTick)
{
	TArrayView<TUniquePtr<IRecordedDataObjectInterface>> Result;
	if (FMath::Min(BeginTick, EndTick) < 0) { return Result; }

	if (RecordedDataList.IsEmpty()) { return Result; }

	BeginTick = FMath::Min(BeginTick, GetTickMax());
	EndTick = FMath::Min(EndTick, GetTickMax());

	if (BeginTick > EndTick) { return Result; }

	const int32 Begin = Algo::LowerBoundBy(RecordedDataList, BeginTick,
		[](const TUniquePtr<IRecordedDataObjectInterface>& Data) { return Data->Tick; });

	const int32 End = Algo::UpperBoundBy(RecordedDataList, EndTick,
		[](const TUniquePtr<IRecordedDataObjectInterface>& Data) { return Data->Tick; });

	//if (Begin == INDEX_NONE || End == INDEX_NONE) { return Result; }
	if (Begin == RecordedDataList.Num()) { return Result; }

	if (RecordedDataList[Begin]->Tick != BeginTick || RecordedDataList[End - 1]->Tick != EndTick) { return Result; }

	Result = MakeArrayView(RecordedDataList.GetData() + Begin, End - Begin);

	return Result;
}

//// Called every frame
//void UInputRecordComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
//{
//	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
//
//	// ...
//}

