// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "Ability/WeaponOperation/WeaponOperationTypes.h"
#include "GEEC_WeaponReload.generated.h"

/**
 *
 */
UCLASS()
class LEARNING2_API UGEEC_WeaponReload : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UGEEC_WeaponReload();

	// ºËÐÄ¼ÆËãº¯Êý
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Tag")
	FGameplayTag AmountToReloadTag = WeaponOperationTypesPublic::Tag_AmountToReload.GetTag();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Tag")
	FGameplayTag WeaponSlotTag = WeaponOperationTypesPublic::Tag_WeaponSlot.GetTag();
};
