#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FogOfWarMaskedActorInterface.generated.h"

class UTexture2D;
class UMeshComponent;

UINTERFACE(Blueprintable, MinimalAPI)
class UFogOfWarMaskedActorInterface : public UInterface
{
	GENERATED_BODY()
};

class FOGOFWAR_API IFogOfWarMaskedActorInterface
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent)
    void UpdateFogOfWarTexture(UTexture2D* FogOfWarTexture);
    virtual void UpdateFogOfWarTexture_Implementation(UTexture2D* FogOfWarTexture) = 0;

public:
    static FString GetFogOfWarTextureParameterName();
    
    enum class EDoInitialize : uint8
    {
        No,
        Yes
    };
    static void UpdateFogOfWarTexture_DefaultImplementation(UTexture2D* FogOfWarTexture, UMeshComponent* SkeletalMeshComponent);
    static void UpdateFogOfWarTexture_DefaultImplementation(UTexture2D* FogOfWarTexture, UMeshComponent* SkeletalMeshComponent, bool& bIsSuccessfullyInitialized, const EDoInitialize DoInitialize = EDoInitialize::Yes);
};