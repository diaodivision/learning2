// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAbilitySystemComponent.h"

UMyAbilitySystemComponent::UMyAbilitySystemComponent() : Super()
{
	bAutoActivate = true;
}

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