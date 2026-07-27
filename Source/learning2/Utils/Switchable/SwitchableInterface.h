// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "SwitchableInterface.generated.h"

class ACharacter;
class USwitchableCollection;

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class USwitchableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class LEARNING2_API ISwitchableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnAdded(const USwitchableCollection* Container);
	virtual void OnAdded_Implementation(const USwitchableCollection* Container) = 0;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnRemoved(const USwitchableCollection* Container);
	virtual void OnRemoved_Implementation(const USwitchableCollection* Container) = 0;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnControl(UObject* InOwner);
	virtual void OnControl_Implementation(UObject* InOwner) = 0;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnControlReleased();
	virtual void OnControlReleased_Implementation() = 0;
};
