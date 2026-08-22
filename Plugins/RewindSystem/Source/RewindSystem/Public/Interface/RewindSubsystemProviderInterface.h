#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RewindSubsystemProviderInterface.generated.h"

UINTERFACE(Blueprintable, BlueprintType, MinimalAPI)
class URewindSubsystemProviderInterface : public UInterface
{
	GENERATED_BODY()
};

class REWINDSYSTEM_API IRewindSubsystemProviderInterface : public IInterface
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent)
    bool ShouldCreateRewindSubsystem() const;
    virtual bool ShouldCreateRewindSubsystem_Implementation() const = 0;
};
