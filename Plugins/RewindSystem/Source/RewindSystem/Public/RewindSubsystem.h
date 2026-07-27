// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Delegates/DelegateCombinations.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Tickable.h"
#include "InputRecordedDataTypes/RecordedDataDefines.h"
#include "InputRecordedDataTypes/RecordableInterface.h"
#include "Interface/RecordableActorInterface.h"
#include "RewindSubsystem.generated.h"

class AActor;
class APawn;
class UInputRecordComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRewindSubsystemStateChangedDelegate, ERecordState, OldState, ERecordState, NewState);

/**
 *
 */
UCLASS()
class REWINDSYSTEM_API URewindSubsystem : public ULocalPlayerSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return !IsTemplate(); }//²»ÊÇCDO²ÅTick
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(URewindSubsystem, STATGROUP_Tickables); }

	UFUNCTION(BlueprintCallable, Category = "Rewind Subsystem")
	virtual void OnPostComponentInitialize(UInputRecordComponent* Component);

	inline ERecordState GetCurrentState() const { return CurrentState; };

	FRecordedDataObjectHandle Record(TUniquePtr<IRecordedDataObjectInterface> RecordedData, const AActor& Owner, TFunctionRef<void(const FRecordedDataObjectHandle&)> PostRecordCallable = [](const auto& Handle) {}) const;

	UFUNCTION(BlueprintCallable, Category = "Rewind Subsystem")
	virtual void SwitchState(ERecordState NewState);

	UFUNCTION(BlueprintCallable, Category = "Rewind Subsystem")
	void K2_ToTick(UInputRecordComponent* InComponent, const int32 Tick);
	virtual void ToTick(UInputRecordComponent& InComponent, const RewindSystemTickType Tick);

	UFUNCTION(BlueprintCallable, Category = "Rewind Subsystem")
	void K2_RemoveFromTick(UInputRecordComponent* InComponent, const int32 Tick);
	virtual void RemoveFromTick(UInputRecordComponent& InComponent, const RewindSystemTickType Tick);

	UInputRecordComponent* GetInputRecordComponentFromOwner(const AActor& Owner);

protected:
	virtual void InitializeDelegates();
	virtual void DeinitializeDelegates();
	virtual void InitializeInputRecordComponentMap();

	virtual void OnIdleState();
	virtual void OnRecordPauseState();
	virtual void OnRecordingState();
	virtual void OnPreviewPauseState();
	virtual void OnPreviewingState();
	virtual void OnRewindingState();

	virtual void OnIdleStateStart();
	virtual void OnRecordPauseStateStart();
	virtual void OnRecordingStateStart();
	virtual void OnPreviewPauseStateStart();
	virtual void OnPreviewingStateStart();
	virtual void OnRewindingStateStart();

	virtual void OnIdleStateEnd();
	virtual void OnRecordPauseStateEnd();
	virtual void OnRecordingStateEnd();
	virtual void OnPreviewPauseStateEnd();
	virtual void OnPreviewingStateEnd();
	virtual void OnRewindingStateEnd();

#if WITH_EDITOR
	virtual void OnActorAdded(AActor* Actor);
	virtual void OnActorDeleted(AActor* Actor);
#endif

	void PauseAllActorsExceptPlayer();
	void UnpauseAllActors();

	UFUNCTION()
	virtual void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

	virtual void OnRewindSubsystemStateChanged(const ERecordState OldState, const ERecordState NewState);

	TArray<TWeakObjectPtr<UInputRecordComponent>> GetInputRecordComponents();

	void SetSubsystemTickEnabled(bool bIsEnable);

	inline bool IsActorRecordable(const AActor& Actor) const { return Actor.Implements<URecordableActorInterface>(); }

	inline virtual bool IsSwitchValid(const ERecordState NewState) const
	{
		bool bIsSwitchValid{ true };

		switch (CurrentState)
		{
		case ERecordState::Idle:
			if (NewState != ERecordState::RecordPause && NewState != ERecordState::Recording)
			{
				bIsSwitchValid = false;
			}
			break;

		case ERecordState::RecordPause:
			break;

		case ERecordState::Recording:
			//if (NewState == ERecordState::Previewing)
			//{
			//	bIsSwitchValid = false;
			//}
			break;

		case ERecordState::PreviewPause:
			if (NewState == ERecordState::RecordPause)
			{
				bIsSwitchValid = false;
			}
			break;

		case ERecordState::Previewing:
			//if (NewState == ERecordState::Recording)
			//{
			//	bIsSwitchValid = false;
			//}
			break;

		case ERecordState::Rewinding:
			if (NewState != ERecordState::Idle)
			{
				bIsSwitchValid = false;
			}
			break;
		}

		return bIsSwitchValid;
	}

public:
	FOnRewindSubsystemStateChangedDelegate OnRewindSubsystemStateChangedDelegate;

private:
	//TArray<TWeakObjectPtr<UInputRecordComponent>> InputRecordComponents;

	TMap<TWeakObjectPtr<AActor>, TWeakObjectPtr<UInputRecordComponent>> InputRecordComponentMap;

	ERecordState CurrentState{ ERecordState::Idle };
};
