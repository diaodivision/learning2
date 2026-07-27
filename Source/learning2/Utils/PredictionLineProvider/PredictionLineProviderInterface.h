// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "PredictionLineProviderInterface.generated.h"

class ABulletBase;
class APawn;

USTRUCT(BlueprintType)
struct FPredictionLineParams
{
	GENERATED_BODY();

	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<ABulletBase> BulletClass;

	UPROPERTY(BlueprintReadWrite)
	APawn* Instigator;
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UPredictionLineProviderInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class LEARNING2_API IPredictionLineProviderInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, Category = "Prediction Line")
	virtual void ShowPredictionLine(const FPredictionLineParams& Params) = 0;

	UFUNCTION(BlueprintCallable, Category = "Prediction Line")
	virtual void HidePredictionLine() = 0;

	UFUNCTION(BlueprintCallable, Category = "Prediction Line")
	virtual bool IsPredictionLineVisible() const = 0;
};
