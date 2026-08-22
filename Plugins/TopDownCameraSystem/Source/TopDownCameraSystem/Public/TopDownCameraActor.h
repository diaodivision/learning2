// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "TopDownCameraTypes.h"
#include "TopDownCameraActor.generated.h"

class UInterpToMovementComponent;

UCLASS()
class ATopDownCameraActor : public ACameraActor
{
    GENERATED_BODY()

public:
    virtual void SetForceTarget(const FVector& TargetLocation, const EForceMovementType ForceMovementType);

protected:
    UFUNCTION()
    virtual void OnInterpToStop(const FHitResult& ImpactResult, float Time);
    void OnInterpToStop();

private:
    UPROPERTY()
    TObjectPtr<UInterpToMovementComponent> InterpToMovementComponent;
};
