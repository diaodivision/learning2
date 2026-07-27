// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "Ability/AttributeSet/MyAttributeSet.h"
#include "GEEC_WeaponDamage.generated.h"

/**
 *
 */
UCLASS()
class LEARNING2_API UGEEC_WeaponDamage : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UGEEC_WeaponDamage();

	// ºËÐÄ¼ÆËãº¯Êý
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	float CalculateFinalDamage(float BaseDamage, float WeaponPenetration, float TargetArmorThickness) const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Tag")
	FGameplayTag DamageTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Tag")
	FGameplayTag WeaponPenetrationTag;
};
