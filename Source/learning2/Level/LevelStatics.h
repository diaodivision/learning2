// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LevelTypes.h"
#include "LevelStatics.generated.h"

struct FInteractionOption;

/**
 *
 */
UCLASS()
class LEARNING2_API ULevelStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category="Data|Level")
	static TArray<FLevelData> GetLevelDatas();
};
