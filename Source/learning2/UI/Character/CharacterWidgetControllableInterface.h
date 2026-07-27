// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CharacterWidgetTypes.h"

#include "CharacterWidgetControllableInterface.generated.h"

class UUserWidget;

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable, BlueprintType)
class UCharacterWidgetControllableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class LEARNING2_API ICharacterWidgetControllableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "UI")
	void ShowCharacterWidget(bool bIsShow);
	virtual void ShowCharacterWidget_Implementation(bool bIsShow) = 0;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "UI")
	bool IsCharacterWidgetVisible() const;
	virtual bool IsCharacterWidgetVisible_Implementation() const = 0;
};
