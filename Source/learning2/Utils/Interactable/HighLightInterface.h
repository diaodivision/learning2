// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "HighLightInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(Blueprintable, MinimalAPI)
class UHighLightInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class LEARNING2_API IHighLightInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(Blueprintcallable, BlueprintImplementableEvent, Category = "Display")
	void HighLightActor();

	UFUNCTION(Blueprintcallable, BlueprintImplementableEvent, Category = "Display")
	void UnhighLightActor();
};
