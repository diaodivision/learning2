#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WorldHeightSubsystemProviderInterface.generated.h"

UINTERFACE(Blueprintable, BlueprintType, MinimalAPI)
class UWorldHeightSubsystemProviderInterface : public UInterface
{
	GENERATED_BODY()
};

class FOGOFWAR_API IWorldHeightSubsystemProviderInterface : public IInterface
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent)
    bool ShouldCreateWorldHeightSubsystem() const;
    virtual bool ShouldCreateWorldHeightSubsystem_Implementation() const = 0;
};
