// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterWidgetComponent.h"
#include "CharacterWidgetTypes.h"
#include "CharacterWidget.h"

void UCharacterWidgetComponent::UpdateUICharacterInfo(const FUICharacterInfo& UICharacterInfo)
{
	UCharacterWidget* CharacterWidget{ Cast<UCharacterWidget>(GetWidget()) };
	if (!CharacterWidget) { return; }

	CharacterWidget->UpdateUICharacterInfo(UICharacterInfo);
}

void UCharacterWidgetComponent::UpdateUIPlayerCharacterInfo(const FUIPlayerCharacterInfo& UIPlayerCharacterInfo)
{
	UCharacterWidget* CharacterWidget{ Cast<UCharacterWidget>(GetWidget()) };
	if (!CharacterWidget) { return; }

	CharacterWidget->UpdateUIPlayerCharacterInfo(UIPlayerCharacterInfo);
}

void UCharacterWidgetComponent::ShowCharacterUI(bool bIsShow)
{
	UCharacterWidget* CharacterWidget{ Cast<UCharacterWidget>(GetWidget()) };
	if (!CharacterWidget) { return; }

	CharacterWidget->ShowCharacterUI(bIsShow);
}