// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Components/WidgetComponent.h"
#include "CharacterWidgetComponent.generated.h"

struct FUICharacterInfo;

/**
 *
 */
UCLASS(Blueprintable)
class LEARNING2_API UCharacterWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UCharacterWidgetComponent();

	UFUNCTION(BlueprintCallable)
	void UpdateUICharacterInfo(const FUICharacterInfo& UICharacterInfo);

	UFUNCTION(BlueprintCallable)
	void UpdateUIPlayerCharacterInfo(const FUIPlayerCharacterInfo& UIPlayerCharacterInfo);

	UFUNCTION(BlueprintCallable)
	void ShowCharacterUI(bool bIsShow);
};
