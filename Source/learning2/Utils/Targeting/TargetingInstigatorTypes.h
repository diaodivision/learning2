// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Delegates/DelegateCombinations.h"
#include "TargetingInstigatorTypes.generated.h"

UENUM(BlueprintType)
enum class ETargetingState : uint8
{
	Targeting,
	Confirm,
	Cancel
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnTargetingStateChangedDelegate, ETargetingState NewState);