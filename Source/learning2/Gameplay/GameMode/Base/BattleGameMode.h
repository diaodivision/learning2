#pragma once

#include "MyGameModeBase.h"
#include "BattleGameMode.generated.h"

#define SUBSYSTEM_PROVIDER_INTERFACE_IMPLEMENTATION(SUBSYSTEM_NAME, CONDITION) \
    virtual FORCEINLINE bool ShouldCreate##SUBSYSTEM_NAME##_Implementation() const override { return CONDITION; }

UCLASS(Blueprintable, BlueprintType)
class LEARNING2_API ABattleGameMode : public AMyGameModeBase
{
    GENERATED_BODY()

    SUBSYSTEM_PROVIDER_INTERFACE_IMPLEMENTATION(FogOfWarSubsystem, true);
    SUBSYSTEM_PROVIDER_INTERFACE_IMPLEMENTATION(WorldHeightSubsystem, true);
    SUBSYSTEM_PROVIDER_INTERFACE_IMPLEMENTATION(RewindSubsystem, true);
    SUBSYSTEM_PROVIDER_INTERFACE_IMPLEMENTATION(TopDownCameraSubsystem, true);
    SUBSYSTEM_PROVIDER_INTERFACE_IMPLEMENTATION(WorldPauseSubsystem, true);
    SUBSYSTEM_PROVIDER_INTERFACE_IMPLEMENTATION(BattleSubsystem, true);
};

#undef SUBSYSTEM_PROVIDER_INTERFACE_IMPLEMENTATION