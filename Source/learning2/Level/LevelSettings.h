#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
// #include "Engine/DataTable.h"
#include "LevelSettings.generated.h"

class UDataTable;

UCLASS(Config = Level, defaultconfig, meta = (DisplayName = "Level Settings"))
class LEARNING2_API ULevelSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<UDataTable> LevelDatas;
};