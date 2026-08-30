// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "Delegates/DelegateCombinations.h"
#include "FogOfWarProxyWidgetComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnWidgetVisibilityChangedDelegate, const ESlateVisibility InVisibility);

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

protected:
    void BeginPlay() override;

    virtual void OnWidgetVisibilityChanged(const ESlateVisibility InVisibility);

public:
    FOnWidgetVisibilityChangedDelegate OnWidgetVisibilityChangedDelegate;
};
