#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FreezableInterface.generated.h"

UINTERFACE(Blueprintable, BlueprintType, MinimalAPI)
class UFreezableInterface : public UInterface
{
	GENERATED_BODY()
};

class WORLDPAUSESYSTEM_API IFreezableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Recordable")
	void Freeze();
	virtual void Freeze_Implementation() = 0;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Recordable")
	void Unfreeze();
	virtual void Unfreeze_Implementation() = 0;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Recordable")
	bool IsFreezing();
	virtual bool IsFreezing_Implementation() = 0;
};
