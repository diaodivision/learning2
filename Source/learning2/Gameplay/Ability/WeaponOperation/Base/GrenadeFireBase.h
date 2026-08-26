// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponFireAbilityBase.h"
#include "GrenadeFireBase.generated.h"

class AMyCharacterBase;
class AWeaponActorBase;

/**
 *
 */
UCLASS(Blueprintable, BlueprintType, Abstract)
class LEARNING2_API UGrenadeFireBase : public UWeaponFireAbilityBase
{
	GENERATED_BODY()

	void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual bool ShouldRecord_Implementation() const override;
	virtual FORCEINLINE bool ShouldStopWhenFailToHandleRecordedData_Implementation() const override { return true; }

	virtual void Record() override;

	virtual bool TryHandleRecordedData(TSharedPtr<IRecordedDataObjectInterface, ESPMode::NotThreadSafe> InRecordedData) override;

	virtual void OnPreview(const bool bIsPreview, const IRecordedDataObjectInterface* InRecordedData) override;

	virtual void FinishShoot() override;

protected:
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AMyCharacterBase> Instigator;
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AWeaponActorBase> Weapon;
	TOptional<FGameplayEventData> CachedTriggerEventData;
};