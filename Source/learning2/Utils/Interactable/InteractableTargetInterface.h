// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractionQuery.h"
//#include "InteractionOptionBuilder.h"

#include "InteractableTargetInterface.generated.h"

struct FInteractionOptionBuilder;

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UInteractableTargetInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class LEARNING2_API IInteractableTargetInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Cost)
	void GatherInteractionOptions(const FInteractionQuery& InteractQuery);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Display)
	void ShowOptions(const FInteractionQuery& InteractQuery);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Display)
	void HideOptions();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Display)
	bool IsShow() const;
};
