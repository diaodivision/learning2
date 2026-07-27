#pragma once

#include "CoreMinimal.h"
//#include "Delegates/DelegateCombinations.h"
#include "Subsystems/WorldSubsystem.h"
#include "WorldPauseSubsystem.generated.h"

UCLASS()
class WORLDPAUSESYSTEM_API UWorldPauseSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

	//virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	//virtual void Deinitialize() override;


public:
	virtual void OnFreezableObjectRegistered(UObject* FreezableObject);
	virtual void OnFreezableObjectUnregistered(UObject* FreezableObject);

	UFUNCTION(BlueprintCallable, Category = "World Pause")
	virtual void FreeObject(UObject* FreezableObject);
	UFUNCTION(BlueprintCallable, Category = "World Pause")
	virtual void UnfreeObject(UObject* FreezableObject);

	UFUNCTION(BlueprintCallable, Category = "World Pause")
	virtual void FreezeAllObject();
	UFUNCTION(BlueprintCallable, Category = "World Pause")
	virtual void UnfreezeAllObject();

protected:
	void ClearInvalidObject();

private:
	TSet<TWeakObjectPtr<UObject>> FreezableObjects;

	bool bIsFreezeAllObject{ false };
};