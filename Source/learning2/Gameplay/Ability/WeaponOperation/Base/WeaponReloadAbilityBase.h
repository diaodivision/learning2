// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Ability/WeaponOperation/WeaponOperationTypes.h"
#include "WeaponReloadAbilityBase.generated.h"

class AWeaponActorBase;

/**
 *
 */
UCLASS(Blueprintable, BlueprintType, Abstract)
class LEARNING2_API UWeaponReloadAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UWeaponReloadAbilityBase();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const override;

	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	UFUNCTION(BlueprintCallable)
	bool CanExecuteReload(const AWeaponActorBase* Weapon) const;

protected:
	UFUNCTION(BlueprintCallable)
	inline void EndAbilityWithCancelState(const bool bWasCancelled)
	{
		constexpr bool bReplicateEndAbility{ true };
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicateEndAbility, bWasCancelled);
	}

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Tag")
	FGameplayTag ReloadTimeTag{ WeaponOperationTypesPublic::Tag_ReloadTime.GetTag() };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Tag")
	FGameplayTag AmountToReloadTag{ WeaponOperationTypesPublic::Tag_AmountToReload.GetTag() };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Tag")
	FGameplayTag WeaponSlotTag{ WeaponOperationTypesPublic::Tag_WeaponSlot.GetTag() };
};