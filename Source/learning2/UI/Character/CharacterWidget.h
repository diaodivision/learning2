// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
//#include "CharacterWidgetTypes.h"
#include "CharacterWidget.generated.h"

struct FUICharacterInfo;

/**
 *
 */
UCLASS(Blueprintable, BlueprintType)
class LEARNING2_API UCharacterWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent/*, meta = (DisplayName = "Update Character Widget Info")*/)
	void UpdateUICharacterInfo(const FUICharacterInfo& UICharacterInfo);

	UFUNCTION(BlueprintImplementableEvent/*, meta = (DisplayName = "Update Player Character Widget Info")*/)
	void UpdateUIPlayerCharacterInfo(const FUIPlayerCharacterInfo& UIPlayerCharacterInfo);

	UFUNCTION(BlueprintImplementableEvent/*, meta = (DisplayName = "Show Player Character Info")*/)
	void ShowCharacterUI(bool bIsShow);
};
