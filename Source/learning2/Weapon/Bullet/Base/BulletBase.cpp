#include "BulletBase.h"
#include "Components/MeshComponent.h"
#include "FogOfWarSubsystem.h"
#include "FogOfWarComponentStatics.h"

void ABulletBase::UpdateFogOfWarTexture_Implementation(UTexture2D* FogOfWarTexture)
{
    const EDoInitialize DoInitialize{ bIsFogOfWarMaskInitialized ? EDoInitialize::No : EDoInitialize::Yes };
    UpdateFogOfWarTexture_DefaultImplementation(FogOfWarTexture, FindComponentByClass<UMeshComponent>(), bIsFogOfWarMaskInitialized, DoInitialize);
}

void ABulletBase::BeginPlay()
{
    Super::BeginPlay();

    if (UFogOfWarSubsystem* FogOfWarSubsystem{ UFogOfWarComponentStatics::GetFogOfWarSubsystem(this) })
	{
		FogOfWarSubsystem->OnFogOfWarTextureUpdatedDelegate.AddUniqueDynamic(this, &ABulletBase::UpdateFogOfWarTexture);
	}
}

void ABulletBase::EndPlay(EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

	if (UFogOfWarSubsystem* FogOfWarSubsystem{ UFogOfWarComponentStatics::GetFogOfWarSubsystem(this) })
	{
		FogOfWarSubsystem->OnFogOfWarTextureUpdatedDelegate.RemoveAll(this);
	}
}