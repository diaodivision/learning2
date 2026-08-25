// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NavModifierComponent.h"
#include "MyNavModifierComponent.generated.h"

/**
 *
 */
UCLASS(ClassGroup = (Navigation), meta = (BlueprintSpawnableComponent), hidecategories = (Activation))
class LEARNING2_API UMyNavModifierComponent : public UNavModifierComponent
{
	GENERATED_BODY()

public:
	UMyNavModifierComponent();

private:
	virtual void GetNavigationData(FNavigationRelevantData& Data) const override;
};
