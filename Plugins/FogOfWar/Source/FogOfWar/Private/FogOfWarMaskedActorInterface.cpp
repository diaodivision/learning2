#include "FogOfWarMaskedActorInterface.h"
#include "Engine/StaticMesh.h"
#include "FogOfWarTypes.h"
#include "Components/MeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

FString IFogOfWarMaskedActorInterface::GetFogOfWarTextureParameterName()
{
    return FString{ FogOfWarConst::FogOfWarTextureParameterName };
}

void IFogOfWarMaskedActorInterface::UpdateFogOfWarTexture_DefaultImplementation(UTexture2D* FogOfWarTexture, UMeshComponent* SkeletalMeshComponent)
{
	const TArray<UMaterialInterface*> MaterialInterfaces{ SkeletalMeshComponent->GetMaterials() };
	for (UMaterialInterface* MaterialInterface : MaterialInterfaces)
	{
		UMaterialInstanceDynamic* MaterialInstanceDynamic{ Cast<UMaterialInstanceDynamic>(MaterialInterface) };
		if (!MaterialInstanceDynamic) { continue; }
		MaterialInstanceDynamic->SetTextureParameterValue(*GetFogOfWarTextureParameterName(), FogOfWarTexture);
	}
}

void IFogOfWarMaskedActorInterface::UpdateFogOfWarTexture_DefaultImplementation(UTexture2D* FogOfWarTexture, UMeshComponent* SkeletalMeshComponent, bool& bIsSuccessfullyInitialized, const EDoInitialize DoInitialize)
{
    bIsSuccessfullyInitialized = false;
    if (!FogOfWarTexture || !SkeletalMeshComponent) { return; }

    if (DoInitialize == EDoInitialize::Yes)
	{
        const TArray<UMaterialInterface*> MaterialInterfaces{ SkeletalMeshComponent->GetMaterials() };
        for (int32 i{0}; i < MaterialInterfaces.Num(); i++)
		{
			UMaterialInterface* MaterialInterface{ MaterialInterfaces[i] };
			if (!MaterialInterface) { continue; }
			SkeletalMeshComponent->CreateDynamicMaterialInstance(i, MaterialInterface);
		}
    
		bIsSuccessfullyInitialized = true;
	}
	
    UpdateFogOfWarTexture_DefaultImplementation(FogOfWarTexture, SkeletalMeshComponent);
}