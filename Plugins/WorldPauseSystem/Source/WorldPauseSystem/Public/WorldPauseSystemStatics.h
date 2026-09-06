// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WorldPauseSystemStatics.generated.h"

class UWorldPauseSubsystem;

/**
 *
 */
UCLASS(MinimalAPI)
class UWorldPauseSystemStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure)
	static WORLDPAUSESYSTEM_API bool IsWorldFreezing(const UObject* WorldContextObject);

	static WORLDPAUSESYSTEM_API UWorldPauseSubsystem* GetWorldPauseSubsystem(const UObject* WorldContextObject);
};
