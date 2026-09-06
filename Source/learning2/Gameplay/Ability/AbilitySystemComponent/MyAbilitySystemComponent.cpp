// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAbilitySystemComponent.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayEffectTypes.h"

UMyAbilitySystemComponent::UMyAbilitySystemComponent() : Super()
{
	bAutoActivate = true;
}

void UMyAbilitySystemComponent::Freeze_Implementation()
{
	if (bIsHandingUnfreeze) { return; }
	ActiveGameplayEffectRecords.RemoveAll([](const FActiveGameplayEffectRecord& Record){ return FMath::IsNearlyZero(Record.TimeRemaining); });

	const float CurrentTime{ static_cast<float>(GetWorld()->GetTimeSeconds()) };
	const FActiveGameplayEffectsContainer& Container{ GetActiveGameplayEffects() };
	for (const FActiveGameplayEffectHandle Handle : Container.GetAllActiveEffectHandles())
	{
		const FActiveGameplayEffect* GE{ Container.GetActiveGameplayEffect(Handle) };
		if (!GE) { continue; }

		const float TimeRemaining{ FMath::Max(0.f, GE->GetTimeRemaining(CurrentTime)) };
		if (FMath::IsNearlyZero(TimeRemaining)) { continue; }
		ActiveGameplayEffectRecords.Add({ GE->Spec.Def, GE->Spec.GetContext(), TimeRemaining });
	}

	bIsFreezing = true;
}

void UMyAbilitySystemComponent::Unfreeze_Implementation()
{
	if (bIsHandingUnfreeze) { return; }
	bIsHandingUnfreeze = true;
	ActiveGameplayEffectRecords.RemoveAll([](const FActiveGameplayEffectRecord& Record){ return FMath::IsNearlyZero(Record.TimeRemaining); });
	for (const FActiveGameplayEffectRecord& Record : ActiveGameplayEffectRecords)
	{
		const FGameplayEffectSpecHandle Handle{ MakeOutgoingSpec(Record.GE->GetClass(), 1.f, Record.ContextHandle) };
		if (!Handle.IsValid()) { continue; }

		Handle.Data.Get()->SetDuration(Record.TimeRemaining, true);
		ApplyGameplayEffectSpecToSelf(*Handle.Data.Get());
	}

	ActiveGameplayEffectRecords.Empty();
	bIsFreezing = false;
	bIsHandingUnfreeze = false;
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