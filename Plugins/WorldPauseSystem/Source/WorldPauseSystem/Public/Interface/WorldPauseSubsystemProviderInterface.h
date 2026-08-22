#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WorldPauseSubsystemProviderInterface.generated.h"

UINTERFACE(Blueprintable, BlueprintType, MinimalAPI)
class UWorldPauseSubsystemProviderInterface : public UInterface
{
	GENERATED_BODY()
};

class WORLDPAUSESYSTEM_API IWorldPauseSubsystemProviderInterface : public IInterface
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent)
    bool ShouldCreateWorldPauseSubsystem() const;
    virtual bool ShouldCreateWorldPauseSubsystem_Implementation() const = 0;
};
