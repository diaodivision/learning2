// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbility_Interact.h"
#include "AbilitySystemComponent.h"
#include "AbilityTask_WaitForInteractableTargets.h"
#include "Interactable/InteractionOption.h"
#include "GameFramework/PlayerController.h"

//UGameplayAbility_Interact::UGameplayAbility_Interact()
//{
//	// 设置为持续运行的能力，不会被其他能力打断
//	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
//}
//
//void UGameplayAbility_Interact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
//{
//	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
//
//	//UAbilitySystemComponent* ASC;
//}
//
//void UGameplayAbility_Interact::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
//{
//}
//
//void UGameplayAbility_Interact::UpdateInteractions(const TArray<FInteractionOption>& InteractiveOptions)
//{
//}
//
//void UGameplayAbility_Interact::TriggerInteraction()
//{
//}
