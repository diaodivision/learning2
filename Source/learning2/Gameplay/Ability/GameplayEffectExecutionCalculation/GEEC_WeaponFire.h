// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "Ability/WeaponOperation/WeaponOperationTypes.h"
#include "GEEC_WeaponFire.generated.h"

/**
 *
 */
UCLASS()
class LEARNING2_API UGEEC_WeaponFire : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UGEEC_WeaponFire();

	// ºËÐÄ¼ÆËãº¯Êý
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;


protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Tag")
	FGameplayTag FireCostTag = WeaponOperationTypesPublic::Tag_FireCost.GetTag();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Tag")
	FGameplayTag WeaponSlotTag = WeaponOperationTypesPublic::Tag_WeaponSlot.GetTag();
};
