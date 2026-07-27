#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WorldHeightEffectiveActorInterface.generated.h"

UINTERFACE(Blueprintable, MinimalAPI)
class UWorldHeightEffectiveActorInterface : public UInterface
{
	GENERATED_BODY()
};

class FOGOFWAR_API IWorldHeightEffectiveActorInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "Bounds")
	FOrientedBox GetBounds() const;
};