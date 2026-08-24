#pragma once

#include "Engine/DataTable.h"
#include "LevelTypes.generated.h"

class UTexture2D;
class UWorld;

UENUM(BlueprintType)
enum class ELevelType : uint8
{
    Menu,
    Battle,
};

USTRUCT(BlueprintType)
struct FLevelData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> LevelIcon{ nullptr };
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UWorld> LevelReference{ nullptr };
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ELevelType LevelType{ ELevelType::Battle };
};

namespace LevelTypeConst
{
    constexpr static const FStringView LevelDataPath{ TEXT("/Game/Level/Data/DT_LevelData.DT_LevelData") };
}