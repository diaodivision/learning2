// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAbilitySystemComponent.h"

void UMyAbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnGiveAbility(AbilitySpec);

	OnGiveGameplayAbilityDelegate.Broadcast(AbilitySpec, this);
}

void UMyAbilitySystemComponent::OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	OnRemoveGameplayAbilityDelegate.Broadcast(AbilitySpec, this);

	Super::OnRemoveAbility(AbilitySpec);
}