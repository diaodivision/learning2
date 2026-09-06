// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponFireAbilityBase.h"
#include "WeaponFireBase.generated.h"

class AMyCharacterBase;
class AWeaponActorBase;

/**
 *
 */
UCLASS(Blueprintable, BlueprintType, Abstract)
class LEARNING2_API UWeaponFireBase : public UWeaponFireAbilityBase
{
	GENERATED_BODY()

public:
	bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	virtual void ExecuteFire(const FGameplayTag Tag, const int32 NewCount);
	virtual void ExecuteFire_Internal();

	virtual void FinishShoot() override;

	virtual void OnWaitForTagTimeOut();

protected:
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<const AMyCharacterBase> Instigator;
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AWeaponActorBase> Weapon;
	TOptional<FGameplayEventData> CachedTriggerEventData;

	FDelegateHandle RegisterGameplayTagEventHandle;
	FTimerHandle WaitForTagTimeOutHandle;
	FTimerHandle FireLoopHandle;
};