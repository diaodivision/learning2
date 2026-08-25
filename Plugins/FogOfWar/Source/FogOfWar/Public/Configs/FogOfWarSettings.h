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

UCLASS(Config = FogOfWar, defaultconfig)
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

public:
    // 1. 指定顶级容器：通常为 "Project"（项目设置）或 "Editor"
    virtual FORCEINLINE FName GetContainerName() const override { return FName("Project"); }

    // 2. 指定左侧大类：例如 "Plugins" 或 "Game"
    virtual FORCEINLINE FName GetCategoryName() const override { return FName("Plugins"); }

    // 3. 指定左侧子项名称（Section）
    virtual FORCEINLINE FName GetSectionName() const override { return FName("FogOfWar"); }

#if WITH_EDITOR
    // 4. 指定面板顶部显示的大标题（支持本地化/带空格名称）
    virtual FORCEINLINE FText GetSectionText() const override { return NSLOCTEXT("Fog Of War", "Fog Of War Settings Name", "Fog Of War"); }
    
    // 5. 指定下方描述文字
    virtual FORCEINLINE FText GetSectionDescription() const override { return NSLOCTEXT("Fog Of War", "Fog Of War Settings Desc", "Configure Fog of War module settings."); }
#endif
};