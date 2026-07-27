// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactable/InteractionOption.h"
#include "ActorWidgetTypes.generated.h"

class UTexture2D;
class UMyGameplayAbilityBase;


USTRUCT(BlueprintType)
struct FWidgetInteractiveOption
{
	GENERATED_BODY()
};
//USTRUCT(BlueprintType)
//struct FWidgetInteractiveOption : public FInteractionOption
//{
//	GENERATED_BODY()
//
//	FWidgetInteractiveOption()
//	{
//#if !WITH_EDITOR
//		ensureMsgf(false, TEXT("Default ctor should not be used directly"));
//#endif
//	}
//
//	FWidgetInteractiveOption(const FInteractionOption& Option, TSoftObjectPtr<UTexture2D> Icon);
//
//	inline bool IsValid() const { return FInteractionOption::IsValid() && !Icon.IsNull(); }
//
//	void SetActive(bool bWillActivate);
//
//	bool IsActive() const;
//
//	//virtual FString GetReferencerName() const override { return TEXT("FWidgetInteractiveOption"); }
//
//	bool operator==(const FWidgetInteractiveOption& Other) const;
//
//	UPROPERTY(BlueprintReadWrite)
//	TSoftObjectPtr<UTexture2D> Icon;
//};