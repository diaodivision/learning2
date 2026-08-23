// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TopDownCameraSystemStatics.generated.h"

class UTopDownCameraSubsystem;

/**
 *
 */
UCLASS(MinimalAPI)
class UTopDownCameraSystemStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static TOPDOWNCAMERASYSTEM_API UTopDownCameraSubsystem* GetTopDownCameraSubsystem(const UObject* WorldContextObject);
};
