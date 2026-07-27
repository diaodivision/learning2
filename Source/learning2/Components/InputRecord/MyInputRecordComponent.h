// MyInputRecordComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Containers/ArrayView.h"
#include "Algo/BinarySearch.h"
#include "RecordDataManager.h"
#include "InputRecordedDataTypes/RecordedDataDefines.h"
#include "MyInputRecordComponent.generated.h"

class UInputAction;
//enum class ERecordState :uint8;

//UENUM()
//enum class ERecordState : uint8
//{
//	None,
//	Recording,
//	Reviewing,
//	ReviewPlaying,
//	Playing,
//	Pending
//};

UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class LEARNING2_API UMyInputRecordComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMyInputRecordComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Gameplay|Record")
	void BindOperation(const UInputAction* InputAction, FShowWillExecutingOperation ShowWillExecutingOperation, FExecuteOperation ExecuteOperation);

	UFUNCTION(BlueprintCallable, Category = "Gameplay|Record")
	void OnOperationExecuted(UMyRecordData* Data);

	UFUNCTION(BlueprintCallable, Category = "Gameplay|Record")
	void AddPendingPlan();

	UFUNCTION(BlueprintCallable, Category = "Gameplay|Record")
	void StartRecord() { SwitchState(ERecordState::Recording); }

	UFUNCTION(BlueprintCallable, Category = "Gameplay|Record")
	void StopRecord() { SwitchState(ERecordState::Previewing); }

	UFUNCTION(BlueprintCallable, Category = "Gameplay|Record")
	void StartPlay() { SwitchState(ERecordState::Rewinding); }

	UFUNCTION(BlueprintCallable, Category = "Gameplay|Record")
	void ReviewToTick(int32 Tick);

	UFUNCTION(BlueprintCallable, Category = "Gameplay|Record")
	void ShrinkRecordDataByTick(int32 StartTick);

	UFUNCTION(BlueprintCallable, Category = "Gameplay|Record")
	void Play();

	UFUNCTION(BlueprintCallable, Category = "Gameplay|Record")
	void Interrupt();

	UFUNCTION(BlueprintCallable, Category = "Gameplay|Record")
	void ReviewPlay();
protected:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Gameplay|Record")
	bool IsEnd() { return CurrentTickPlaying > GetMaxTickRecorded(); }

	//UFUNCTION(BlueprintCallable, Category = "Gameplay|Record")
	TArrayView<TObjectPtr< UMyRecordData >> GetRecordDataByTick(int32 Tick);

	UFUNCTION(BlueprintCallable, Category = "Gameplay|Record")
	void ShrinkRecordData(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Gameplay|Record")
	void SwitchState(ERecordState ToState);

	UFUNCTION(BlueprintCallable, Category = "Gameplay|Record")
	void resetShowWillExecutingOperation(int32 BeginTick);

	UFUNCTION(BlueprintCallable, Category = "Gameplay|Record")
	void CommitCacheRecordData();

	UFUNCTION(BlueprintCallable, Category = "Gameplay|Record")
	void ExecutePending(UMyRecordData* Data);

	UFUNCTION(BlueprintCallable, Category = "Gameplay|Record")
	void ShowWillExecutePending(bool bWillExecute);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	bool bShowUI{ true };

	UPROPERTY()
	TObjectPtr<UUserWidget> UI;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> UIClass;

private:
	UFUNCTION(BlueprintCallable, Category = "Gameplay|Record")
	int32 GetMaxTickRecorded() { return RecordData.Num() ? RecordData[RecordData.Num() - 1]->Tick : 0; }

	const FMyRecordBindData* GetBindData(const UInputAction* InputAction) const;

	UPROPERTY()
	TArray<FMyRecordBindData> RecordBindData;

	UPROPERTY()
	TArray<TObjectPtr<UMyRecordData>> RecordData;

	UPROPERTY()
	TArray<TObjectPtr<UMyRecordData>> CacheRecordData;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	ERecordState State{ ERecordState::Idle };
	bool bFromRecordingToReviewing{ true };
	int32 CurrentTick{ 0 };
	int32 CurrentTickReviewing{ 0 };
	int32 CurrentTickPlaying{ 0 };
};