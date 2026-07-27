// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "Abilities/GameplayAbility.h"
#include "Ability/WeaponOperation/WeaponOperationTypes.h"
#include "Ability/MyGameplayAbilityBase.h"
#include "WeaponFireAbilityBase.generated.h"

class AWeaponActorBase;
/**
 *
 */
UCLASS(Blueprintable, BlueprintType, Abstract)
class LEARNING2_API UWeaponFireAbilityBase : public UMyGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UWeaponFireAbilityBase();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const override;

	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	UFUNCTION(BlueprintCallable)
	bool CanExecuteFire(const AWeaponActorBase* Weapon) const;

	inline FGameplayTag GetOnShootTag() const { return OnShootTag; }
	inline FGameplayTag GetShotCountTag() const { return ShotCountTag; }

	UFUNCTION(BlueprintCallable)
	void OnExecuteShoot() const;

	UFUNCTION(BlueprintCallable)
	bool CanExecuteShoot() const;

	virtual FORCEINLINE bool ShouldRecord_Implementation() const override { return false; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Tag")
	FGameplayTag FireCooldownTag{ WeaponOperationTypesPublic::Tag_FireRate.GetTag() };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Tag")
	FGameplayTag FireCostTag{ WeaponOperationTypesPublic::Tag_FireCost.GetTag() };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Tag")
	FGameplayTag WeaponSlotTag{ WeaponOperationTypesPublic::Tag_WeaponSlot.GetTag() };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Tag")
	FGameplayTag OnShootTag{ WeaponOperationTypesPublic::Tag_OnShoot.GetTag() };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Tag")
	FGameplayTag ShotCountTag{ WeaponOperationTypesPublic::Tag_ShotCount.GetTag() };
};