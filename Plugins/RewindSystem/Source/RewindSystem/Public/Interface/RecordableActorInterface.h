#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interface/FreezableInterface.h"
#include "RecordableActorInterface.generated.h"

UINTERFACE(Blueprintable, BlueprintType, MinimalAPI)
class URecordableActorInterface : public UFreezableInterface
{
	GENERATED_BODY()
};

class REWINDSYSTEM_API IRecordableActorInterface : public IFreezableInterface
{
	GENERATED_BODY()
};
