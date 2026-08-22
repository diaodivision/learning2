#include "TopDownCameraActor.h"
#include "Components/InterpToMovementComponent.h"

void ATopDownCameraActor::SetForceTarget(const FVector& TargetLocation, const EForceMovementType ForceMovementType)
{
    if (ForceMovementType == EForceMovementType::Teleport)
    {
        if (InterpToMovementComponent) 
        { 
            OnInterpToStop();
        }
        SetActorLocation(TargetLocation);
    }
    else
    {
        if (!InterpToMovementComponent) 
        { 
            InterpToMovementComponent = NewObject<UInterpToMovementComponent>(this);
            InterpToMovementComponent->RegisterComponent();
            InterpToMovementComponent->bSweep = false;
            InterpToMovementComponent->BehaviourType = EInterpToBehaviourType::OneShot;
            InterpToMovementComponent->Duration = 0.5f;
        }

        InterpToMovementComponent->ResetControlPoints();
        
        InterpToMovementComponent->AddControlPointPosition(FVector::ZeroVector, true);
        const FVector WorldDelta = TargetLocation - GetActorLocation();
        const FVector LocalDelta = GetActorRotation().UnrotateVector(WorldDelta);
        InterpToMovementComponent->AddControlPointPosition(LocalDelta, true);

        InterpToMovementComponent->FinaliseControlPoints();
        InterpToMovementComponent->RestartMovement(1.0f);
        
        InterpToMovementComponent->OnInterpToStop.AddUniqueDynamic(this, &ATopDownCameraActor::OnInterpToStop);
    }
}

// void ATopDownCameraActor::SetForceTarget(const FVector& TargetLocation, const EForceMovementType ForceMovementType)
// {
//     if (ForceMovementType == EForceMovementType::Teleport)
//     {
//         if (InterpToMovementComponent) 
//         { 
//             InterpToMovementComponent->StopMovementImmediately();
//             InterpToMovementComponent = nullptr;
//         }
//         SetActorLocation(TargetLocation);
//     }
//     else
//     {
//         if (!InterpToMovementComponent) 
//         { 
//             InterpToMovementComponent = NewObject<UInterpToMovementComponent>(this);
//             InterpToMovementComponent->RegisterComponent();
//             InterpToMovementComponent->bSweep = false;
//         }
//         else 
//         { 
//         }
//         InterpToMovementComponent->ResetControlPoints();
        
//         InterpToMovementComponent->AddControlPointPosition(GetActorLocation(), false);
//         InterpToMovementComponent->AddControlPointPosition(TargetLocation, false);
//         InterpToMovementComponent->FinaliseControlPoints();
//         InterpToMovementComponent->RestartMovement();
//         InterpToMovementComponent->OnInterpToStop.AddUniqueDynamic(this, &ATopDownCameraActor::OnInterpToStop);
//     }
// }

void ATopDownCameraActor::OnInterpToStop(const FHitResult& ImpactResult, float Time)
{
    OnInterpToStop();
}

void ATopDownCameraActor::OnInterpToStop()
{
    if (!InterpToMovementComponent) { return; }
    InterpToMovementComponent->DestroyComponent();
    InterpToMovementComponent = nullptr;
}