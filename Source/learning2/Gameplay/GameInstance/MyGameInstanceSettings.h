#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
// #include "Engine/DataTable.h"
#include "MyGameInstanceSettings.generated.h"

class ULoadingScreenWidget;

UCLASS(Config = Level, defaultconfig, meta = (DisplayName = "Level Settings", CategoryName = "Current Project|Levels"))
class LEARNING2_API UMyGameInstanceSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly)
    TSoftClassPtr<ULoadingScreenWidget> LoadingScreen;

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly)
    TSoftClassPtr<ULoadingScreenWidget> LoadingScreen_Covered;
    
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly)
    TArray<TSoftObjectPtr<UWorld>> NoCoveredLevelList;

public:
    // 1. 指定顶级容器：通常为 "Project"（项目设置）或 "Editor"
    virtual FORCEINLINE FName GetContainerName() const override { return FName("Project"); }

    // 2. 指定左侧大类：例如 "Plugins" 或 "Game"
    virtual FORCEINLINE FName GetCategoryName() const override { return FName("MyGameInstance"); }

    // 3. 指定左侧子项名称（Section）
    virtual FORCEINLINE FName GetSectionName() const override { return FName("MyGameInstance"); }
};