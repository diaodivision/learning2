// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "FogOfWarProxyWidgetComponent.generated.h"

/**
 *
 */
UCLASS(Blueprintable)
class LEARNING2_API UFogOfWarProxyWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UFogOfWarProxyWidgetComponent();

    UFUNCTION(BlueprintCallable)
	void ShowWidget(const bool bIsShow);
};
