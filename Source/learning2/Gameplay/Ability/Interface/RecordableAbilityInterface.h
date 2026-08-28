// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//#include "CoreMinimal.h"
//#include "UObject/Interface.h"
//#include "GameplayEffect.h"
//#include "Abilities/GameplayAbility.h"
#include "InputRecordedDataTypes/RecordableInterface.h"
#include "InputRecordedDataTypes/RecordedDataDelegates.h"
#include "Abilities/GameplayAbility.h"
#include "RecordableAbilityInterface.generated.h"
//#include "RecordableAbilityInterface.generated.h"

//// This class does not need to be modified.
//UINTERFACE(Blueprintable, BlueprintType, MinimalAPI)
//class URecordableAbilityInterface : public URecordableInterface
//{
//	GENERATED_BODY()
//};
//
//class LEARNING2_API IRecordableAbilityInterface : public IRecordableInterface
//{
//	GENERATED_BODY()
//
//public:
//	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Recordable")
//	const UGameplayEffect* GetReverseCostGameplayEffect() const;
//};
class UGameplayEffect;

UENUM(BlueprintType)
enum class EReleaseRecordedDataPolicy : uint8
{
	ReleaseManual,
	AutoReleaseOnEndAbility
};

UCLASS(Blueprintable, BlueprintType, Abstract)
class LEARNING2_API URecordableAbilityBase : public UGameplayAbility, public IDurativeRecordableInterface
{
	GENERATED_BODY()

public:
	virtual FORCEINLINE EActionState GetActionState() const override { return EActionState::Inactive; }
	virtual FORCEINLINE ERecordableActionType GetActionType_Implementation() const { return ERecordableActionType::HasDuration; };
	virtual FORCEINLINE bool ShouldRecord_Implementation() const { return false; };
	virtual FORCEINLINE bool ShouldStopWhenFailToHandleRecordedData_Implementation() const { return true; }

protected:
	virtual FORCEINLINE void PreRecord() override {}
	virtual FORCEINLINE void Record(const FGameplayEventData* TriggerEventData) {}

	UE_DEPRECATED(5.7, "Record() without parameters is deprecated, use Record(const FGameplayEventData* TriggerEventData) instead.")
	virtual FORCEINLINE void Record() override final
	{
		// Example: 

		//if (ShouldRecord())
		//{
		//	URewindSubsystem* System = URewindSystemStatics::GetRewindSubsystem(this);
		//	if (!System) { return; }

		//	TUniquePtr<IRecordedDataObjectInterface> Data = MakeUnique<IRecordedDataObjectInterface>();

		//	if (!ActorInfo->AvatarActor.Get()) { return; }

		//FRecordedDataObjectHandle RecordedDataObjectHandle = System->Record(MoveTemp(Data), *AvatarActor, [this](const FRecordedDataObjectHandle& Handle) {Execute_PostRecord(this, Handle); });
		//}
		//else
		//{
		//}
	}
	virtual FORCEINLINE void PostRecord(const FRecordedDataObjectHandle& Handle) override { Execute_K2_PostRecord(this, Handle); }

	virtual FORCEINLINE void OnPreview(const bool bIsPreview, const IRecordedDataObjectInterface* InRecordedData) override {};

	virtual bool TryHandleRecordedData(TSharedPtr<IRecordedDataObjectInterface, ESPMode::NotThreadSafe> InRecordedData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Recordable")
	const UGameplayEffect* GetReverseCostGameplayEffect() const;
	virtual inline const UGameplayEffect* GetReverseCostGameplayEffect_Implementation() const { return nullptr; }

public:
	FORCEINLINE void SetAutoReleaseRecordedDataPolicy(const EReleaseRecordedDataPolicy InReleaseRecordedDataPolicy) { ReleaseRecordedDataPolicy = InReleaseRecordedDataPolicy; }
	FORCEINLINE EReleaseRecordedDataPolicy GetAutoReleaseRecordedDataPolicy() const { return ReleaseRecordedDataPolicy; }

protected:
	virtual void NotifyStartDurativeAction(TSharedPtr<IRecordedDataObjectInterface, ESPMode::NotThreadSafe> InRecordedData) override final;
	virtual void NotifyEndDurativeAction() override final;
	virtual void ReleaseRecordedData() override final;

protected:
	TSharedPtr<IRecordedDataObjectInterface, ESPMode::NotThreadSafe> RecordedData;

private:
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	EReleaseRecordedDataPolicy ReleaseRecordedDataPolicy{ EReleaseRecordedDataPolicy::AutoReleaseOnEndAbility };
};

//class FRecordableAbilityBase : public FDurativeRecordableBase
//{
//public:
//};