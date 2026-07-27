// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Components/ActorComponent.h"
#include "RecordedLocationVisualizationTypes.h"
#include "RecordedLocationVisualizationComponent.generated.h"

class ARecordedLocationVisualizer;

/**
 *
 */
UCLASS(Blueprintable)
class LEARNING2_API URecordedLocationVisualizationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void AddRecordedLocation(const RewindSystemTickType Tick, const FVector& Location);
	virtual void RemoveRecordedLocation(const RewindSystemTickType Tick);

protected:
	virtual void BeginPlay() override;

	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

private:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TWeakObjectPtr<ARecordedLocationVisualizer> RecordedLocationVisualizer;

	TArray<FRecordedLocationVisualizationData> Datas;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true, UIMin = "0", ClampMin = "0"))
	int32 VisualizationStep{ 60 };
};
