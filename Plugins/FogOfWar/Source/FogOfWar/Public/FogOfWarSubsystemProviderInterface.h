#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FogOfWarSubsystemProviderInterface.generated.h"

UINTERFACE(Blueprintable, BlueprintType, MinimalAPI)
class UFogOfWarSubsystemProviderInterface : public UInterface
{
	GENERATED_BODY()
};

class FOGOFWAR_API IFogOfWarSubsystemProviderInterface : public IInterface
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent)
    bool ShouldCreateFogOfWarSubsystem() const;
    virtual bool ShouldCreateFogOfWarSubsystem_Implementation() const = 0;
};
