#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/TextureDefines.h"
#include "RenderGraphUtils.h"
#include "FogOfWarTypes.h"
#include "FogOfWarSettings.generated.h"

class UMaterialInterface;

UENUM(BlueprintType)
enum class FogOfWarPixelFormat : uint8
{
    PF_G8
};

UENUM(BlueprintType)
enum class FogOfWarTextureFilter : uint8
{
    TF_Default
};

UCLASS(Config = FogOfWar, defaultconfig, meta = (DisplayName = "Fog Of War Settings"))
class FOGOFWAR_API UFogOfWarSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<UMaterialInterface> FogOfWarMaterial;
    
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly)
	FogOfWarPixelFormat PixelFormat{ FogOfWarPixelFormat::PF_G8 };

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly)
    FogOfWarTextureFilter FilterMethod{ FogOfWarTextureFilter::TF_Default };
    
    UPROPERTY(Config, VisibleAnywhere, BlueprintReadOnly)
	int32 kThreadsX{ FogOfWarConst::kThreadsX };
    UPROPERTY(Config, VisibleAnywhere, BlueprintReadOnly)
	int32 kThreadsY{ FogOfWarConst::kThreadsY };
    UPROPERTY(Config, VisibleAnywhere, BlueprintReadOnly)
	int32 kThreadsZ{ FogOfWarConst::kThreadsZ };
    
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "8", ClampMax = "2048", UIMin = "8", UIMax = "2048", Delta = "8"))
	int32 kTextureWidth{ 1024 };
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "8", ClampMax = "8192", UIMin = "8", UIMax = "2048", Delta = "8"))
	int32 kTextureHeight{ 1024 };
    
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly)
	FString FogOfWarTextureParameterName{ TEXT("FogOfWarTexture") };

public:
    FORCEINLINE static EPixelFormat GetPixelFormatFromEnum(FogOfWarPixelFormat PixelFormat)
    {
        switch (PixelFormat)
        {
        case FogOfWarPixelFormat::PF_G8:
            return EPixelFormat::PF_G8;
        default:
            return EPixelFormat::PF_B8G8R8A8;
        }
    }

    FORCEINLINE static TextureFilter GetTextureFilterFromEnum(FogOfWarTextureFilter FilterMethod)
    {
        switch (FilterMethod)
        {
        case FogOfWarTextureFilter::TF_Default:
            return TF_Default;
        default:
            return TF_Default;
        }
    }
};