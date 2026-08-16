#include "FogOfWarComponent.h"
#include "GameFramework/Actor.h"
#include "FogOfWarSubsystem.h"
#include "FogOfWarComponentStatics.h"
#include "FogOfWarShaderTypes.ush"

UFogOfWarComponent::UFogOfWarComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFogOfWarComponent::BeginPlay()
{
	Super::BeginPlay();

	UFogOfWarSubsystem* Subsystem{ UFogOfWarComponentStatics::GetFogOfWarSubsystem(this)};
	if (ensure(Subsystem)) {Subsystem->OnPostComponentInitialize(this);}
}

TOptional<FFogOfWarData> UFogOfWarComponent::GetFogOfWarData() const
{
	const AActor* Owner = GetOwner();
	if (!Owner) { return NullOpt; }

	const FVector ForwardVector{ Owner->GetActorForwardVector() };

	FFogOfWarData Result;
	Result.ActorLocation = Owner->GetActorLocation();
	Result.ActorVisionLeft = ForwardVector.RotateAngleAxis(-VisionDegree, FVector::UpVector);
	Result.ActorVisionRight = ForwardVector.RotateAngleAxis(VisionDegree, FVector::UpVector);
	Result.Radius = VisionRadius;

	return Result;
}