// Fill out your copyright notice in the Description page of Project Settings.

#include "ActorWidgetTypes.h"
#include "Ability/MyGameplayAbilityBase.h"

//FWidgetInteractiveOption::FWidgetInteractiveOption(const FInteractionOption& Option, TSoftObjectPtr<UTexture2D> Icon) :
//	FInteractionOption(Option), Icon(Icon)
//{
//}
//
//void FWidgetInteractiveOption::SetActive(bool bWillActivate)
//{
//	if (!IsValid()) { return; }
//
//	if (UMyGameplayAbilityBase * GA{ Cast<UMyGameplayAbilityBase>(AbilityInstance.Get()) })
//	{
//		GA->SetActionState(bWillActivate ? EActionState::Active : EActionState::Inactive);
//	}
//}
//
//bool FWidgetInteractiveOption::IsActive() const
//{
//	if (!IsValid()) { return false; }
//
//	const UMyGameplayAbilityBase* GA{ Cast<UMyGameplayAbilityBase>(AbilityInstance.Get()) };
//	if (!GA) { return false; }
//
//	return GA->GetActionState() == EActionState::Active;
//}
//
//bool FWidgetInteractiveOption::operator==(const FWidgetInteractiveOption& Other) const
//{
//	return FInteractionOption::operator==(Other) && Icon == Other.Icon;
//}
