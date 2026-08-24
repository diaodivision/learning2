// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputRecordedDataTypes/RecordableInterface.h"
#include "InputRecordedDataTypes/RecordedDataDefines.h"
#include "InputRecordedDataTypes/RecordedDataDelegates.h"
#include "RewindSystemStatics.h"
#include "Delegates/DelegateCombinations.h"
#include "InputRecordComponent.generated.h"

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FShowWillExecuteOperationDelegate, const FRecordedDataObjectHandle&, Handle, bool, bWillExecute);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnInputRecordComponentTickChangedDelegate, int32, OldTick, int32, NewTick, int32, NewTickMax);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class REWINDSYSTEM_API UInputRecordComponent : public UActorComponent
{
	GENERATED_BODY()
	friend class URewindSubsystem;

public:
	// Sets default values for this component's properties
	UInputRecordComponent();
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//virtual void NotifyEndPreivewState();
	//virtual void OnStateStart(ERecordState NewState);
	//virtual void OnStateEnd(ERecordState OldState);

	inline RewindSystemTickType GetCurrentTick() const { return CurrentTick; }
	inline RewindSystemTickType GetTickMax() const
	{
		if (RecordedDataList.IsEmpty() || !RecordedDataList.Last().IsValid()) { return REWIND_TICK_NONE; }

		return RecordedDataList.Last()->Tick;
	}

	UFUNCTION(BlueprintCallable, Category = "Recordable")
	inline ERecordState GetRecordState() const { return URewindSystemStatics::GetRewindSubsystemState(this); }

	UFUNCTION(BlueprintCallable, Category = "RewindSystem|Delegate")
	[[nodiscard]] URecordedDataPreviewOperationDelegateWrapper* FindOrTryAddPreviewOperationDelegateWrapper(const FRecordedDataObjectHandle& RecordedDataObjectHandle);

	IRecordedDataObjectInterface* GetRecordDataByHandle(const FRecordedDataObjectHandle& RecordedDataObjectHandle);

	UFUNCTION(BlueprintCallable, Category = "RewindSystem|Handle")
	FORCEINLINE bool IsRecordDataObjectExist(const FRecordedDataObjectHandle& RecordedDataObjectHandle) const
	{
		return !!const_cast<UInputRecordComponent*>(this)->GetRecordDataByHandle(RecordedDataObjectHandle);
	}

	virtual FORCEINLINE void CancelRewindingState()
	{
		if (GetRecordState() != ERecordState::Rewinding) { return; }
		OnStateChanged(GetRecordState(), ERecordState::Idle);
	}

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void OnRegister() override;

	virtual void OnOwnerPossess();
	virtual void OnOwnerUnPossess();

	virtual FRecordedDataObjectHandle Record(TUniquePtr<IRecordedDataObjectInterface> RecordedDataBase, TFunctionRef<void(const FRecordedDataObjectHandle&)> PostRecordCallable = [](const auto& Handle) {});
	virtual void HandleRecordedData();
	virtual void RemoveFromTick(const RewindSystemTickType Tick);
	int32 FindPreviousValidIndex(const int32 Index);
	void RemoveInvalidDataNearToIndex(const int32 Index);

	virtual void PreviewDataOfTick(RewindSystemTickType Tick);
	void PreviewToTick(RewindSystemTickType Tick);

	void PreviewAllData();
	void UndoPreviewAllData();

	void AdvanceCurrentTickOnRecordState();
	virtual void OnStateChanged(const ERecordState OldState, const ERecordState NewState);

	TArrayView<TUniquePtr<IRecordedDataObjectInterface>> GetRecordDataByTick(RewindSystemTickType Tick);
	TArrayView<TUniquePtr<IRecordedDataObjectInterface>> GetRecordDataByTickRange(RewindSystemTickType BeginTick, RewindSystemTickType EndTick);

	virtual void HandleDataOfTick(RewindSystemTickType Tick);

	void PreviewData(TArrayView<TUniquePtr<IRecordedDataObjectInterface>> DataList, bool bIsPreview);

	void ClearRecordedData(ERecordState NewState);
	inline void ResetCurrentTick(ERecordState NewState)
	{
		SetCurrentTick(GetStartTick());
	}

	FORCEINLINE RewindSystemTickType GetStartTick(const ERecordState State) const 
	{ 
		RewindSystemTickType Result{ REWIND_TICK_NONE };
		if (State == ERecordState::Recording || State == ERecordState::RecordPause || State == ERecordState::Rewinding)
		{
			Result = static_cast<RewindSystemTickType>(1);
		}
		return Result;
	}

	FORCEINLINE RewindSystemTickType GetStartTick() const { return GetStartTick(GetRecordState()); }

	//virtual void FreezeActorMotivation(AActor* Actor, bool bIsFreeze);
	inline void SetCurrentTick(const RewindSystemTickType Tick)
	{
		const RewindSystemTickType OldTick{ CurrentTick };
		CurrentTick = FMath::Max(REWIND_TICK_NONE, Tick);

		OnInputRecordComponentTickChangedDelegate.Broadcast(OldTick, FMath::Min(CurrentTick, GetTickMax()), GetTickMax());
	}

public:
	FOnInputRecordComponentTickChangedDelegate OnInputRecordComponentTickChangedDelegate;

	FOnOperationPreviewDelegate OnOperationPreviewDelegate;

protected:
	TArray<TUniquePtr<IRecordedDataObjectInterface>> RecordedDataList;

	UPROPERTY()
	TMap<FRecordedDataObjectHandle, TObjectPtr<URecordedDataPreviewOperationDelegateWrapper>> RecordedDataPreviewOperationDelegateWrapperMap;

	TOptional<TWeakPtr<IRecordedDataObjectInterface, ESPMode::NotThreadSafe>> RecordedDataToken;

private:
	RewindSystemTickType CurrentTick{ REWIND_TICK_NONE };
};
