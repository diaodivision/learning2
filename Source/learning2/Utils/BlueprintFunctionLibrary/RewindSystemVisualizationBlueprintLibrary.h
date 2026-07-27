// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RewindSystemVisualizationBlueprintLibrary.generated.h"

class USplineMeshComponent;
class AActor;

/**
 *
 */
UCLASS()
class LEARNING2_API URewindSystemVisualizationBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static USplineMeshComponent* NewSplineMeshComponentFromPool(AActor* InOuter/*const UObject* WorldContextObject*/);
	static void ReleaseSplineMeshComponentToPool(USplineMeshComponent& SplineMeshComponent);

	static UStaticMeshComponent* NewStaticMeshComponentFromPool(AActor* InOuter/*const UObject* WorldContextObject*/);
	static void ReleaseStaticMeshComponentToPool(UStaticMeshComponent& StaticMeshComponent);
};
