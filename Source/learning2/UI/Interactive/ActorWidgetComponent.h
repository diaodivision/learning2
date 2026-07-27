// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Components/WidgetComponent.h"
#include "Interactable/InteractionOption.h"
#include "ActorWidgetComponent.generated.h"

class UAbilitySystemComponent;

/**
 *
 */
UCLASS(Blueprintable)
class LEARNING2_API UActorWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable)
	void UpdateInteractionOptions(TArray<UInteractionOptionBase*> RawOptions);
};
