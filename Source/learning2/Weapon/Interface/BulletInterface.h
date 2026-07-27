#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "BulletInterface.generated.h"

class UObject;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UBulletInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class LEARNING2_API IBulletInterface
{
	GENERATED_BODY()

	friend class UWeaponActorBlueprintLibrary;

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Initialize")
	void InitializeBulletData(const UObject* InData);
	virtual void InitializeBulletData_Implementation(const UObject* InData) = 0;

protected:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Initialize")
	void PostInitializedBulletData();
	virtual void PostInitializedBulletData_Implementation() = 0;
};