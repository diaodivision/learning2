// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FogOfWarProxyActor.generated.h"

class UFogOfWarComponent;

UCLASS(BlueprintType, Blueprintable)
class LEARNING2_API AFogOfWarProxyActor : public AActor
{
	GENERATED_BODY()

public:
	AFogOfWarProxyActor();

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UFogOfWarComponent> FogOfWarComponent;
};
