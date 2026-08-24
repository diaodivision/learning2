#pragma once

#include "GameModeTypes.generated.h"

UENUM(BlueprintType)
enum class EGameEndResult : uint8
{
    NotEnd,
    Failed,
    Won,
};