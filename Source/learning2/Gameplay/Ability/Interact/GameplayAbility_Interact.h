// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Interactable/InteractionOption.h"
#include "GameplayAbility_Interact.generated.h"

/**
 *
 */
UCLASS(Blueprintable)
class LEARNING2_API UGameplayAbility_Interact : public UGameplayAbility
{
	GENERATED_BODY()
};
//UCLASS(Blueprintable)
//class LEARNING2_API UGameplayAbility_Interact : public UGameplayAbility
//{
//	GENERATED_BODY()
//
//public:
//	UGameplayAbility_Interact();
//
//	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
//
//	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
//
//	UFUNCTION(BlueprintCallable)
//	void UpdateInteractions(const TArray<FInteractionOption>& InteractiveOptions);
//
//	UFUNCTION(BlueprintCallable)
//	void TriggerInteraction();
//
//protected:
//	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
//	float InteractionScanRate{ .1f };
//
//	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
//	TArray<FInteractionOption> CurrentOptions;
//
//	UPROPERTY(BlueprintReadOnly)
//	int32 CurrentOptionIndex{ 0 };
//};
