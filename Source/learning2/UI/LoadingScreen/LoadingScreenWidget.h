// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "LoadingScreenWidget.generated.h"

USTRUCT(BlueprintType)
struct FLoadingInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int64 CurrentPSOCount{ 0 };

    UPROPERTY(BlueprintReadWrite)
    int64 TotalPSOCount{ 0 };
};

/**
 *
 */
UCLASS(Blueprintable, BlueprintType)
class LEARNING2_API ULoadingScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Update Loading Info"))
    void UpdateLoadingInfo(const FLoadingInfo& LoadingInfo);
    UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Update Loading Info"))
    void K2_UpdateLoadingInfo(const FLoadingInfo& LoadingInfo);

    // UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    // TObjectPtr<UTextBlock> Count;

    // UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    // TObjectPtr<UTextBlock> Total;
};
