// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "ItemIconProviderInterface.generated.h"

class UTexture2D;

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable, BlueprintType)
class UItemIconProviderInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class LEARNING2_API IItemIconProviderInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, meta = (DisplayName = "Get Item Icon"))
	TSoftObjectPtr<UTexture2D> GetItemIcon() const;
	virtual TSoftObjectPtr<UTexture2D> GetItemIcon_Implementation() const = 0;
};

UINTERFACE(MinimalAPI, Blueprintable, BlueprintType)
class UAbilityIconProviderInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class LEARNING2_API IAbilityIconProviderInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, meta = (DisplayName = "Get Item Icon"))
	TSoftObjectPtr<UTexture2D> GetItemIcon(const UAbilitySystemComponent* ASC) const;
	virtual TSoftObjectPtr<UTexture2D> GetItemIcon_Implementation(const UAbilitySystemComponent* ASC) const = 0;
};