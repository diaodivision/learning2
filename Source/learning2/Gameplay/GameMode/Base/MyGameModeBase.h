#pragma once

#include "GameFramework/GameModeBase.h"
#include "FogOfWarSubsystemProviderInterface.h"
#include "WorldHeightSubsystemProviderInterface.h"
#include "Interface/RewindSubsystemProviderInterface.h"
#include "TopDownCameraSubsystemProviderInterface.h"
#include "Interface/WorldPauseSubsystemProviderInterface.h"
#include "Battle/Interface/BattleSubsystemProviderInterface.h"
#include "MyGameModeBase.generated.h"

#define SUBSYSTEM_PROVIDER_INTERFACE_IMPLEMENTATION(SUBSYSTEM_NAME, CONDITION) \
    virtual FORCEINLINE bool ShouldCreate##SUBSYSTEM_NAME##_Implementation() const override { return CONDITION; }

UCLASS(Abstract, BlueprintType)
class LEARNING2_API AMyGameModeBase : 
    public AGameModeBase,
    public IFogOfWarSubsystemProviderInterface,
    public IWorldHeightSubsystemProviderInterface,
    public IRewindSubsystemProviderInterface,
    public ITopDownCameraSubsystemProviderInterface,
    public IWorldPauseSubsystemProviderInterface,
    public IBattleSubsystemProviderInterface
{
    GENERATED_BODY()

    SUBSYSTEM_PROVIDER_INTERFACE_IMPLEMENTATION(FogOfWarSubsystem, false);
    SUBSYSTEM_PROVIDER_INTERFACE_IMPLEMENTATION(WorldHeightSubsystem, false);
    SUBSYSTEM_PROVIDER_INTERFACE_IMPLEMENTATION(RewindSubsystem, false);
    SUBSYSTEM_PROVIDER_INTERFACE_IMPLEMENTATION(TopDownCameraSubsystem, false);
    SUBSYSTEM_PROVIDER_INTERFACE_IMPLEMENTATION(WorldPauseSubsystem, false);
    SUBSYSTEM_PROVIDER_INTERFACE_IMPLEMENTATION(BattleSubsystem, false);
};

#undef SUBSYSTEM_PROVIDER_INTERFACE_IMPLEMENTATION