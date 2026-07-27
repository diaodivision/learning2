// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ActorWidgetTypes.h"
#include "ActorWidget.generated.h"

class UWidgetController;
class UTexture2D;
class UImage;
struct FStreamableHandle;
class UInteractionOptionBase;

/**
 *
 */
UCLASS(Blueprintable, BlueprintType)
class LEARNING2_API UActorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void UpdateInteractionOptions(TArray<UInteractionOptionBase*> InOptions);

	void SetOptionActive(bool bIsActive, const UInteractionOptionBase& InOption);

protected:
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Update Interaction Options"))
	void K2_UpdateInteractionOptions(const TArray<UInteractionOptionBase*>& InOptions);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, meta = (DisplayName = "Set Option Active"))
	void K2_SetOptionActive(bool bIsActive, UInteractionOptionBase* Option);

public:
	UFUNCTION(BlueprintCallable)
	void SetIconResource(TSoftObjectPtr<UTexture2D> InIconResource);

protected:
	//UPROPERTY(BlueprintReadOnly)
	TArray<TWeakObjectPtr<UInteractionOptionBase>> Options;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> Icon;

	TSoftObjectPtr<UTexture2D> IconResource;

	TSharedPtr<FStreamableHandle> LoadingHandle;
};
