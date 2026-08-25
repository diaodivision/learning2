// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Delegates/DelegateCombinations.h"
#include "MyAbilitySystemComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGiveGameplayAbilityDelegate, const FGameplayAbilitySpec&, AbilitySpec, const UMyAbilitySystemComponent*, AbilitySystemComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRemoveGameplayAbilityDelegate, const FGameplayAbilitySpec&, AbilitySpec, const UMyAbilitySystemComponent*, AbilitySystemComponent);

/**
 *
 */
UCLASS(BlueprintType)
class LEARNING2_API UMyAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UMyAbilitySystemComponent();

protected:
	virtual void OnGiveAbility(FGameplayAbilitySpec& AbilitySpec) override;
	virtual void OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec) override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Ability")
	FOnGiveGameplayAbilityDelegate OnGiveGameplayAbilityDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Ability")
	FOnRemoveGameplayAbilityDelegate OnRemoveGameplayAbilityDelegate;
};
