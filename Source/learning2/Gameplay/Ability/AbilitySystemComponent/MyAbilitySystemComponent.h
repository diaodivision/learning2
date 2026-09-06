// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Delegates/DelegateCombinations.h"
#include "GameplayEffect.h"
#include "Interface/FreezableInterface.h"
#include "MyAbilitySystemComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGiveGameplayAbilityDelegate, const FGameplayAbilitySpec&, AbilitySpec, const UMyAbilitySystemComponent*, AbilitySystemComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRemoveGameplayAbilityDelegate, const FGameplayAbilitySpec&, AbilitySpec, const UMyAbilitySystemComponent*, AbilitySystemComponent);

/**
 *
 */
UCLASS(BlueprintType)
class LEARNING2_API UMyAbilitySystemComponent : public UAbilitySystemComponent, public IFreezableInterface
{
	GENERATED_BODY()

public:
	UMyAbilitySystemComponent();

	virtual void Freeze_Implementation() override;
	virtual void Unfreeze_Implementation() override;
	virtual FORCEINLINE bool IsFreezing_Implementation() override { return bIsFreezing; };

protected:
	virtual void OnGiveAbility(FGameplayAbilitySpec& AbilitySpec) override;
	virtual void OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec) override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Ability")
	FOnGiveGameplayAbilityDelegate OnGiveGameplayAbilityDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Ability")
	FOnRemoveGameplayAbilityDelegate OnRemoveGameplayAbilityDelegate;

private:
	bool bIsFreezing{ false };

	bool bIsHandingUnfreeze{ false };

private:
	struct FActiveGameplayEffectRecord
	{
		const class UGameplayEffect* GE;
		// const float StartTime{ 0.f };
		const struct FGameplayEffectContextHandle ContextHandle;
		const float TimeRemaining{ 0.f };
	};

	TArray<FActiveGameplayEffectRecord> ActiveGameplayEffectRecords;
};
