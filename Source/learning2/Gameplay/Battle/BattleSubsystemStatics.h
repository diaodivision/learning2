// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BattleSubsystemStatics.generated.h"

class UBattleSubsystem;

/**
 *
 */
UCLASS()
class LEARNING2_API UBattleSubsystemStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
    static UBattleSubsystem* GetBattleSubsystem(const UObject* WorldContextObject);
};
