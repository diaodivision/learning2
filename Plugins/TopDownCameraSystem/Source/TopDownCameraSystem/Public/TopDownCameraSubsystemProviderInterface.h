#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TopDownCameraSubsystemProviderInterface.generated.h"

UINTERFACE(Blueprintable, BlueprintType, MinimalAPI)
class UTopDownCameraSubsystemProviderInterface : public UInterface
{
	GENERATED_BODY()
};

class TOPDOWNCAMERASYSTEM_API ITopDownCameraSubsystemProviderInterface : public IInterface
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent)
    bool ShouldCreateTopDownCameraSubsystem() const;
    virtual bool ShouldCreateTopDownCameraSubsystem_Implementation() const = 0;
};