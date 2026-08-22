#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BattleSubsystemProviderInterface.generated.h"

UINTERFACE(Blueprintable, BlueprintType, MinimalAPI)
class UBattleSubsystemProviderInterface : public UInterface
{
	GENERATED_BODY()
};

class LEARNING2_API IBattleSubsystemProviderInterface : public IInterface
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent)
    bool ShouldCreateBattleSubsystem() const;
    virtual bool ShouldCreateBattleSubsystem_Implementation() const = 0;
};
