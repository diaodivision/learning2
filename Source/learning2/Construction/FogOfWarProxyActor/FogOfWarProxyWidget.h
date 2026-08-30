// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FogOfWarProxyWidget.generated.h"

class AFogOfWarProxyActor;

/**
 *
 */
UCLASS(Blueprintable, BlueprintType)
class LEARNING2_API UFogOfWarProxyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void ShowWidget(const ESlateVisibility InVisibility, AFogOfWarProxyActor* InFogOfWarProxyActor);

	virtual void SetVisibility(ESlateVisibility InVisibility) override;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Visibility Changed"))
	void K2_OnVisibilityChanged(const ESlateVisibility InVisibility);

private:
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<AFogOfWarProxyActor> FogOfWarProxyActor;
};
