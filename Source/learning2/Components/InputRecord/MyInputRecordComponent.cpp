// Fill out your copyright notice in the Description page of Project Settings.


#include "MyInputRecordComponent.h"
#include "Blueprint/UserWidget.h"
#include "InputRecordedDataTypes/RecordedDataDefines.h"

// Sets default values for this component's properties
UMyInputRecordComponent::UMyInputRecordComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	// ...
}

void UMyInputRecordComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bShowUI)
	{
		checkf(UIClass, TEXT("Need to specify UserWidget"));

		if (UI = CreateWidget<UUserWidget>(GetWorld(), UIClass))
		{
			UI->AddToViewport();
		}
	}
}

void UMyInputRecordComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (true/*State == ERecordState::Pending*/)
	{
		return;
	}
	else if (State == ERecordState::Rewinding)
	{
		if (CacheRecordData.Num() > 0)
		{
			CommitCacheRecordData();
		}

		Play();

		if (++CurrentTickPlaying; IsEnd())
		{
			SwitchState(ERecordState::Idle);
		}
	}
	else
	{
		if (State == ERecordState::Recording && CacheRecordData.Num() > 0)
		{
			CommitCacheRecordData();
			CurrentTick++;
		}

		if (true/*State == ERecordState::ReviewPlaying*/)
		{
			ReviewPlay();

			if (++CurrentTickPlaying; IsEnd())
			{
				SwitchState(ERecordState::Previewing);
			}
		}
	}
}

void UMyInputRecordComponent::BindOperation(const UInputAction* InputAction, FShowWillExecutingOperation ShowWillExecutingOperation, FExecuteOperation ExecuteOperation)
{
	if (InputAction)
	{
		FMyRecordBindData Data;
		Data.InputAction = InputAction;
		Data.ShowWillExecutingOperation = ShowWillExecutingOperation;
		Data.ExecuteOperation = ExecuteOperation;

		RecordBindData.AddUnique(MoveTemp(Data));
	}
}

void UMyInputRecordComponent::OnOperationExecuted(UMyRecordData* Data)
{
	if (State == ERecordState::Recording && Data && Data->InputAction && GetBindData(Data->InputAction))
	{
		Data->Tick = CurrentTick;

		CacheRecordData.Add(Data);
	}
}

void UMyInputRecordComponent::AddPendingPlan()
{
	if (State != ERecordState::Recording || State != ERecordState::Previewing) { return; }

	if (!GetBindData(nullptr))
	{

		FMyRecordBindData BindData;
		BindData.InputAction = nullptr;
		BindData.ShowWillExecutingOperation.BindUFunction(this, FName("ExecutePending"));
		BindData.ExecuteOperation.BindUFunction(this, FName("ShowWillExecutePending"));

		RecordBindData.AddUnique(MoveTemp(BindData));
	}

	UMyRecordData* Data = NewObject<UMyRecordData>(this);
	Data->Tick = CurrentTick;

	CacheRecordData.Insert(Data, 0);
}

void UMyInputRecordComponent::Play()
{
	TArrayView<TObjectPtr< UMyRecordData >> Data = GetRecordDataByTick(CurrentTickPlaying);

	for (auto& D : Data)
	{
		const FMyRecordBindData* BindData = GetBindData(D->InputAction);

		BindData->ExecuteOperation.ExecuteIfBound(D);
		BindData->ShowWillExecutingOperation.ExecuteIfBound(false);
	}
}

void UMyInputRecordComponent::Interrupt()
{
	if (State != ERecordState::Idle)
	{
		resetShowWillExecutingOperation(CurrentTickPlaying);
		SwitchState(ERecordState::Idle);
	}
}

void UMyInputRecordComponent::ReviewPlay()
{
	TArrayView<TObjectPtr< UMyRecordData >> Data = GetRecordDataByTick(CurrentTickPlaying);

	for (auto& D : Data)
	{
		const FMyRecordBindData* BindData = GetBindData(D->InputAction);

		BindData->ShowWillExecutingOperation.ExecuteIfBound(true);
	}
}

const FMyRecordBindData* UMyInputRecordComponent::GetBindData(const UInputAction* InputAction) const
{
	return RecordBindData.FindByPredicate([InputAction](const FMyRecordBindData& Data)
		{
			return Data.InputAction == InputAction;
		});
}

TArrayView<TObjectPtr< UMyRecordData >> UMyInputRecordComponent::GetRecordDataByTick(int32 Tick)
{
	if (RecordData.IsEmpty() || Tick > GetMaxTickRecorded())
	{
		return TArrayView<TObjectPtr< UMyRecordData >>();
	}

	int32 Begin = Algo::LowerBoundBy(RecordData, Tick,
		[](const TObjectPtr<UMyRecordData>& Data) { return Data->Tick; });

	if (RecordData[Begin]->Tick != Tick)
	{
		return TArrayView<TObjectPtr< UMyRecordData >>();
	}

	int32 End = Algo::UpperBoundBy(RecordData, Tick,
		[](const TObjectPtr<UMyRecordData>& Data) { return Data->Tick; });

	return MakeArrayView(RecordData.GetData() + Begin, End - Begin);
}

void UMyInputRecordComponent::ReviewToTick(int32 Tick)
{
	if (State != ERecordState::Recording || State != ERecordState::Previewing || State != ERecordState::Rewinding) { return; }

	if (Tick > GetMaxTickRecorded()) { return; }

	resetShowWillExecutingOperation(Tick + 1);
}

void UMyInputRecordComponent::SwitchState(ERecordState ToState)
{
	//switch (ToState)
	//{
	//case ERecordState::Idle:
	//	CurrentTick = CurrentTickReviewing = CurrentTickPlaying = 0;
	//	RecordData.Empty();
	//	break;
	//case ERecordState::Recording:
	//	if (State == ERecordState::Rewinding) { return; }

	//	if (State == ERecordState::Previewing)
	//	{
	//		if (CurrentTickReviewing != CurrentTick)
	//		{
	//			ShrinkRecordDataByTick(CurrentTickReviewing);
	//			resetShowWillExecutingOperation(CurrentTickReviewing);
	//		}
	//		CurrentTickReviewing = 0;
	//	}

	//	break;
	//case ERecordState::Reviewing:
	//	if (State != ERecordState::Recording) { return; }
	//	CurrentTickReviewing = CurrentTick;

	//	if (State == ERecordState::ReviewPlaying) { CurrentTickPlaying = 0; }

	//	break;
	//case ERecordState::ReviewPlaying:
	//	if (State != ERecordState::Reviewing) { return; }

	//	break;
	//case ERecordState::Playing:
	//	if (State == ERecordState::None) { return; }

	//	break;
	//case ERecordState::Pending:
	//	if (State != ERecordState::Playing) { return; }

	//	break;
	//}

	//State = ToState;
}

void UMyInputRecordComponent::ShrinkRecordDataByTick(int32 StartTick)
{
	if (State != ERecordState::Previewing || RecordData.IsEmpty() || StartTick > GetMaxTickRecorded()) { return; }

	int32 Begin = Algo::LowerBoundBy(RecordData, StartTick,
		[](const TObjectPtr<UMyRecordData>& Data) { return Data->Tick; });

	if (RecordData[Begin]->Tick != StartTick)
	{
		return;
	}

	ShrinkRecordData(Begin);
}

void UMyInputRecordComponent::ShrinkRecordData(int32 Index)
{
	if (Index < RecordData.Num())
	{
		RecordData.SetNum(Index, EAllowShrinking::No);
	}
}

void UMyInputRecordComponent::resetShowWillExecutingOperation(int32 StartTick)
{
	if (RecordData.IsEmpty() || StartTick > GetMaxTickRecorded()) { return; }

	int32 Begin = Algo::LowerBoundBy(RecordData, StartTick,
		[](const TObjectPtr<UMyRecordData>& Data) { return Data->Tick; });

	if (RecordData[Begin]->Tick != StartTick)
	{
		return;
	}

	for (auto i = Begin; i != RecordData.Num(); i++)
	{
		const FMyRecordBindData* BindData = GetBindData(RecordData[i]->InputAction);
		BindData->ShowWillExecutingOperation.ExecuteIfBound(false);
	}
}

void UMyInputRecordComponent::CommitCacheRecordData()
{
	RecordData.Append(CacheRecordData);
	CacheRecordData.Reset();
}

void UMyInputRecordComponent::ExecutePending(UMyRecordData* Data)
{
	//SwitchState(ERecordState::Pending);

	//todo
}

void UMyInputRecordComponent::ShowWillExecutePending(bool bWillExecute)
{
	//todo
}