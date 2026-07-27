// Fill out your copyright notice in the Description page of Project Settings.


#include "RewindSubsystem.h"
#include "InputRecordComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include <limits>
#include "WorldPauseSubsystem.h"

void URewindSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	//Collection.InitializeDependency<UWorldPauseSubsystem>();

	UE_LOG(LogTemp, Warning, TEXT("URewindSubsystem::Initialize"));

	InitializeInputRecordComponentMap();
	InitializeDelegates();

	OnIdleStateStart();
}

void URewindSubsystem::Deinitialize()
{
	Super::Deinitialize();
	UE_LOG(LogTemp, Warning, TEXT("URewindSubsystem::Deinitialize"));

	DeinitializeDelegates();
}

void URewindSubsystem::Tick(float DeltaTime)
{
	static float Accu{ 0 };
	static ERecordState OldState = { CurrentState };
	Accu += DeltaTime;
	if (Accu > 1 || OldState != CurrentState)
	{
		UE_LOG(LogTemp, Warning, TEXT("Record CurrentState: %d"), CurrentState);
		UE_LOG(LogTemp, Warning, TEXT("Components.Num(): %d"), InputRecordComponentMap.Num());
		Accu = 0;

	};

	switch (CurrentState)
	{
	case ERecordState::Idle:
		OnIdleState();
		break;

	case ERecordState::RecordPause:
		OnRecordPauseState();
		break;

	case ERecordState::Recording:
		OnRecordingState();
		break;

	case ERecordState::PreviewPause:
		OnPreviewPauseState();
		break;

	case ERecordState::Previewing:
		OnPreviewingState();
		break;

	case ERecordState::Rewinding:
		OnRewindingState();
		break;
	}
}

void URewindSubsystem::OnPostComponentInitialize(UInputRecordComponent* Component)
{
	if (AActor* Owner = Component->GetOwner(); Owner && IsActorRecordable(*Owner))
	{
		InputRecordComponentMap.Add(Owner, Component);
	}
}

//#include "InputRecordedDataTypes/InputRecordedDataTemplates.h"
//#include "InputRecordedDataTypes/RecordedDataTypes.h"

FRecordedDataObjectHandle URewindSubsystem::Record(TUniquePtr<IRecordedDataObjectInterface> RecordedData, const AActor& Owner, TFunctionRef<void(const FRecordedDataObjectHandle&)> PostRecordCallable) const
{
	//UE_LOG(LogTemp, Warning, TEXT("TTypeInfo<FLocationData>::GetClassID(): %d"), TTypeInfo<FLocationData>::GetClassID());
	//UE_LOG(LogTemp, Warning, TEXT("TTypeInfo<FRotationData>::GetClassID(): %d"), TTypeInfo<FRotationData>::GetClassID());
	////UE_LOG(LogTemp, Warning, TEXT("TTypeInfo<FRecordedCombinableAbilityData>::GetClassID(): %d"), TTypeInfo<FRecordedCombinableAbilityData>::GetClassID());
	//UE_LOG(LogTemp, Warning, TEXT("RecordedData->GetClassID(): %d"), RecordedData->GetClassID());


	const TWeakObjectPtr<UInputRecordComponent>* InputRecordComponent = InputRecordComponentMap.Find(&Owner);

	if (!ensureAlwaysMsgf(InputRecordComponent, TEXT("InputRecordComponent(Owner Name: %s) is not under managed by RewindSubsystem"), *GetNameSafe(&Owner)))
	{
		return FRecordedDataObjectHandle{};
	}

	if (!InputRecordComponent->IsValid()) { return FRecordedDataObjectHandle{}; }

	return (*InputRecordComponent)->Record(MoveTemp(RecordedData), PostRecordCallable);
}

void URewindSubsystem::InitializeDelegates()
{
#if WITH_EDITOR
	GEngine->OnLevelActorAdded().AddUObject(this, &URewindSubsystem::OnActorAdded);
	GEngine->OnLevelActorDeleted().AddUObject(this, &URewindSubsystem::OnActorDeleted);
#endif

	GetWorld()->GetTimerManager().SetTimerForNextTick([WeakThis = MakeWeakObjectPtr<URewindSubsystem>(this)]() {
		if (!WeakThis.IsValid()) { return; }

		WeakThis->GetWorld()->GetFirstPlayerController()->OnPossessedPawnChanged.AddDynamic(WeakThis.Get(), &URewindSubsystem::OnPossessedPawnChanged);
		});
}

void URewindSubsystem::DeinitializeDelegates()
{
#if WITH_EDITOR
	GEngine->OnLevelActorAdded().RemoveAll(this);
	GEngine->OnLevelActorDeleted().RemoveAll(this);
#endif

	UWorld* World = GetWorld();
	if (!World) { return; }

	AController* Controller = World->GetFirstPlayerController();
	if (!Controller) { return; }

	Controller->OnPossessedPawnChanged.RemoveAll(this);
}

void URewindSubsystem::InitializeInputRecordComponentMap()
{
	TArray<AActor*> RecordableActors;
	//UGameplayStatics::GetAllActorsOfClass(this, ACharacter::StaticClass(), Actors);
	UGameplayStatics::GetAllActorsWithInterface(this, URecordableActorInterface::StaticClass(), RecordableActors);

	for (AActor* Actor : RecordableActors)
	{
		if (Actor && IsActorRecordable(*Actor))
		{
			if (UInputRecordComponent* Component = GetInputRecordComponentFromOwner(*Actor))
			{
				InputRecordComponentMap.Add(Actor, Component);
			}
		}
	}
}

void URewindSubsystem::OnIdleState()
{
}

void URewindSubsystem::OnRecordPauseState()
{
}

void URewindSubsystem::OnRecordingState()
{
	UE_LOG(LogTemp, Warning, TEXT("URewindSubsystem::OnRecordingState"));

	for (const TWeakObjectPtr<UInputRecordComponent>& Component : GetInputRecordComponents())
	{
		Component->AdvanceCurrentTickOnRecordState();
	}
}

void URewindSubsystem::OnPreviewPauseState()
{
}

void URewindSubsystem::OnPreviewingState()
{
	const AActor* PlayerActor = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerActor) { return; }

	if (TWeakObjectPtr<UInputRecordComponent>* Component = InputRecordComponentMap.Find(PlayerActor); Component && Component->IsValid())
	{
		const RewindSystemTickType CurrentTick = (*Component)->GetCurrentTick();

		(*Component)->PreviewToTick(CurrentTick + 1);
	}
}


void URewindSubsystem::OnRewindingState()
{
	int32 TerminatedComponentCount{ 0 };

	TArray<TWeakObjectPtr<UInputRecordComponent>> Components = GetInputRecordComponents();
	for (const TWeakObjectPtr<UInputRecordComponent>& Component : Components)
	{
		if (Component->GetCurrentTick() <= Component->GetTickMax())
		{
			Component->HandleRecordedData();
		}
		else
		{
			TerminatedComponentCount++;
		}
	}

	if (TerminatedComponentCount == Components.Num())
	{
		SwitchState(ERecordState::Idle);
	}
}

void URewindSubsystem::SwitchState(ERecordState NewState)
{
	if (!IsSwitchValid(NewState))
	{
		UE_LOG(LogTemp, Warning, TEXT("URewindSubsystem::SwitchState: cannot switch CurrentState from %d to %d"), CurrentState, NewState);

		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("URewindSubsystem::SwitchState: CurrentState From %d To %d"), CurrentState, NewState);

	const ERecordState OldState = CurrentState;
	CurrentState = NewState;

	switch (OldState)
	{
	case ERecordState::Idle:
		OnIdleStateEnd();
		break;

	case ERecordState::RecordPause:
		OnRecordPauseStateEnd();
		break;

	case ERecordState::Recording:
		OnRecordingStateEnd();
		break;

	case ERecordState::PreviewPause:
		OnPreviewPauseStateEnd();
		break;

	case ERecordState::Previewing:
		OnPreviewingStateEnd();
		break;

	case ERecordState::Rewinding:
		OnRewindingStateEnd();
		break;
	}

	switch (NewState)
	{
	case ERecordState::Idle:
		OnIdleStateStart();
		break;

	case ERecordState::RecordPause:
		OnRecordPauseStateStart();
		break;

	case ERecordState::Recording:
		OnRecordingStateStart();
		break;

	case ERecordState::PreviewPause:
		OnPreviewPauseStateStart();
		break;

	case ERecordState::Previewing:
		OnPreviewingStateStart();
		break;

	case ERecordState::Rewinding:
		OnRewindingStateStart();
		break;
	}

	OnRewindSubsystemStateChanged(OldState, NewState);
}

void URewindSubsystem::K2_ToTick(UInputRecordComponent* InComponent, const int32 Tick)
{
	{
		const uint64 InputTickMax = Tick;
		constexpr uint64 RewindSystemTickTypeMax = std::numeric_limits<RewindSystemTickType>::max();

		if (!ensureAlwaysMsgf(InputTickMax > RewindSystemTickTypeMax, TEXT("Input Tick = %d was greater than std::numeric_limits<RewindSystemTickType>() = %d"), InputTickMax, RewindSystemTickTypeMax)) { return; }
	}

	if (InComponent)
	{
		ToTick(*InComponent, Tick);
	}
}

void URewindSubsystem::ToTick(UInputRecordComponent& InComponent, const RewindSystemTickType Tick)
{
	if (CurrentState == ERecordState::Idle || CurrentState == ERecordState::Rewinding) { return; }
	if (Tick > InComponent.GetTickMax()) { return; }

	AActor* Owner = InComponent.GetOwner();
	if (!Owner) { return; }


	if (TWeakObjectPtr<UInputRecordComponent>* Component = InputRecordComponentMap.Find(Owner);  Component && Component->IsValid())
	{
		(*Component)->PreviewToTick(Tick);
	}
}

void URewindSubsystem::K2_RemoveFromTick(UInputRecordComponent* InComponent, const int32 Tick)
{
	{
		const uint64 InputTickMax = Tick;
		constexpr uint64 RewindSystemTickTypeMax = std::numeric_limits<RewindSystemTickType>::max();

		if (!ensureAlwaysMsgf(InputTickMax > RewindSystemTickTypeMax, TEXT("Input Tick = %d was greater than std::numeric_limits<RewindSystemTickType>() = %d"), InputTickMax, RewindSystemTickTypeMax)) { return; }
	}

	RemoveFromTick(*InComponent, Tick);
}

void URewindSubsystem::RemoveFromTick(UInputRecordComponent& InComponent, const RewindSystemTickType Tick)
{
	InComponent.RemoveFromTick(Tick);
}

UInputRecordComponent* URewindSubsystem::GetInputRecordComponentFromOwner(const AActor& Owner)
{
	TWeakObjectPtr<UInputRecordComponent>* InputRecordComponentPtr{ InputRecordComponentMap.Find(&Owner) };
	return InputRecordComponentPtr ? InputRecordComponentPtr->Get() : nullptr;
}

void URewindSubsystem::OnIdleStateStart()
{
	SetSubsystemTickEnabled(false);
}

void URewindSubsystem::OnRecordPauseStateStart()
{
	SetSubsystemTickEnabled(false);

	PauseAllActorsExceptPlayer();
}

void URewindSubsystem::OnRecordingStateStart()
{
	SetSubsystemTickEnabled(true);

	PauseAllActorsExceptPlayer();
}

void URewindSubsystem::OnPreviewPauseStateStart()
{
	SetSubsystemTickEnabled(false);

	PauseAllActorsExceptPlayer();
}

void URewindSubsystem::OnPreviewingStateStart()
{
	SetSubsystemTickEnabled(true);

	PauseAllActorsExceptPlayer();
}

void URewindSubsystem::OnRewindingStateStart()
{
	SetSubsystemTickEnabled(true);
}

void URewindSubsystem::OnIdleStateEnd()
{

}

void URewindSubsystem::OnRecordPauseStateEnd()
{
	UnpauseAllActors();
}

void URewindSubsystem::OnRecordingStateEnd()
{
	UnpauseAllActors();
}

void URewindSubsystem::OnPreviewPauseStateEnd()
{
	UnpauseAllActors();
}

void URewindSubsystem::OnPreviewingStateEnd()
{
	UnpauseAllActors();
}

void URewindSubsystem::OnRewindingStateEnd()
{
}

#if WITH_EDITOR
void URewindSubsystem::OnActorAdded(AActor* Actor)
{
	//if (Actor && IsActorRecordable(*Actor))
	//{
	//	if (UInputRecordComponent* Component = GetInputRecordComponentFromOwner(*Actor))
	//	{
	//		InputRecordComponentMap.Add(Actor, Component);
	//	}
	//}
}

void URewindSubsystem::OnActorDeleted(AActor* Actor)
{
	if (InputRecordComponentMap.Find(Actor))
	{
		InputRecordComponentMap.Remove(Actor);
	}
}
#endif

void URewindSubsystem::PauseAllActorsExceptPlayer()
{
	UWorldPauseSubsystem* WorldPauseSubsystem{ GetWorld()->GetSubsystem<UWorldPauseSubsystem>() };
	if (!WorldPauseSubsystem) { return; }

	WorldPauseSubsystem->FreezeAllObject();
	WorldPauseSubsystem->UnfreeObject(GetWorld()->GetFirstPlayerController()->GetPawn());
}

void URewindSubsystem::UnpauseAllActors()
{
	UWorldPauseSubsystem* WorldPauseSubsystem{ GetWorld()->GetSubsystem<UWorldPauseSubsystem>() };
	if (!WorldPauseSubsystem) { return; }

	WorldPauseSubsystem->UnfreezeAllObject();
}

void URewindSubsystem::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	if (OldPawn)
	{
		if (TWeakObjectPtr<UInputRecordComponent>* Component = InputRecordComponentMap.Find(OldPawn); Component && Component->IsValid())
		{
			(*Component)->OnOwnerUnPossess();
		}
	}

	if (NewPawn)
	{
		if (TWeakObjectPtr<UInputRecordComponent>* Component = InputRecordComponentMap.Find(NewPawn); Component && Component->IsValid())
		{
			(*Component)->OnOwnerPossess();
		}
	}
}

void URewindSubsystem::OnRewindSubsystemStateChanged(const ERecordState OldState, const ERecordState NewState)
{
	for (TWeakObjectPtr<UInputRecordComponent> Component : GetInputRecordComponents())
	{
		//Component->OnStateEnd(OldState);
		//Component->OnStateStart(NewState);
		Component->OnStateChanged(OldState, NewState);
	}

	OnRewindSubsystemStateChangedDelegate.Broadcast(OldState, NewState);
}

TArray<TWeakObjectPtr<UInputRecordComponent>> URewindSubsystem::GetInputRecordComponents()
{
	TArray<TWeakObjectPtr<UInputRecordComponent>> Result;

	for (auto It = InputRecordComponentMap.CreateIterator(); It; ++It)
	{
		if (It->Key.IsValid() && It->Value.IsValid())
		{
			Result.Add(It->Value);
		}
		else
		{
			It.RemoveCurrent();
		}
	}

	return Result;
}

void URewindSubsystem::SetSubsystemTickEnabled(bool bIsEnable)
{
	SetTickableTickType(bIsEnable ? ETickableTickType::Always : ETickableTickType::Never);
}

