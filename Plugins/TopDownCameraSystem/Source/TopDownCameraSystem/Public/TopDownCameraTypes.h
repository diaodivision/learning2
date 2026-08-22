#pragma once

#include "TopDownCameraTypes.generated.h"

UENUM(BlueprintType)
enum class EForceMovementType : uint8
{
    Teleport,
    Interpolate
};