#include "FogOfWarComponent.h"
#include "FogOfWarComputeShader.h"
#include "WorldHeightSubsystem.h"
#include "FogOfWarTypes.h"
#include "FogOfWarSubsystem.h"

UFogOfWarComponent::UFogOfWarComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFogOfWarComponent::BeginPlay()
{
	Super::BeginPlay();

	UFogOfWarSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UFogOfWarSubsystem>(GetWorld()->GetFirstLocalPlayerFromController());
	Subsystem->OnPostComponentInitialize(this);
}

void UFogOfWarComponent::InitializeComputeShader(int32 InTextureWidth, int32 InTextureHeight)
{
	TextureWidth = InTextureWidth;
	TextureHeight = InTextureHeight;
}

bool UFogOfWarComponent::GetFogOfWarData(FFogOfWarData& Data) const
{
	AActor* Owner = GetOwner();
	if (!Owner) { return false; }

	FVector ForwardVector{ Owner->GetActorForwardVector() };

	Data.ActorLocation = Owner->GetActorLocation();
	Data.ActorVisionLeft = ForwardVector.RotateAngleAxis(-VisionDegree, FVector::UpVector);
	Data.ActorVisionRight = ForwardVector.RotateAngleAxis(VisionDegree, FVector::UpVector);
	Data.Radius = VisionRadius;

	return true;
}
