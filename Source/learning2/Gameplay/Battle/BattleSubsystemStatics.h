// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BattleSubsystemStatics.generated.h"

class UBattleSubsystem;

/**
 *
 */
UCLASS(MinimalAPI)
class UBattleSubsystemStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
    static LEARNING2_API UBattleSubsystem* GetBattleSubsystem(const UObject* WorldContextObject);
};
