#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "TopDownCameraSettings.generated.h"

UCLASS(Config = TopDownCamera, defaultconfig)
class TOPDOWNCAMERASYSTEM_API UTopDownCameraSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly)
    float OrthoWidth{ 1000.f };
    
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly)
    float OrthoNearClipPlane{ -5000.f };
    
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly)
    float OrthoFarClipPlane{ 1e7 };

public:
    // 1. 指定顶级容器：通常为 "Project"（项目设置）或 "Editor"
    virtual FORCEINLINE FName GetContainerName() const override { return FName("Project"); }

    // 2. 指定左侧大类：例如 "Plugins" 或 "Game"
    virtual FORCEINLINE FName GetCategoryName() const override { return FName("Plugins"); }

    // 3. 指定左侧子项名称（Section）
    virtual FORCEINLINE FName GetSectionName() const override { return FName("Top Down Camera"); }

#if WITH_EDITOR
    // 4. 指定面板顶部显示的大标题（支持本地化/带空格名称）
    virtual FORCEINLINE FText GetSectionText() const override { return NSLOCTEXT("Top Down Camera", "Top Down Camera Settings Name", "Top Down Camera"); }
    
    // 5. 指定下方描述文字
    virtual FORCEINLINE FText GetSectionDescription() const override { return NSLOCTEXT("Top Down Camera", "Top Down Camera Settings Desc", "Configure Top Down Camera module settings."); }
#endif
};