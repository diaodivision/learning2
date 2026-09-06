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
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

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

	FORCEINLINE bool IsFreezing() const { return bIsFreezeAllObject; }

protected:
	void ClearInvalidObject();

private:
	TSet<TWeakObjectPtr<UObject>> FreezableObjects;

	bool bIsFreezeAllObject{ false };
};