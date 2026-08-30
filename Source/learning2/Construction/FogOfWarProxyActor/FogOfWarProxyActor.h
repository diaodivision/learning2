// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FogOfWarProxyActor.generated.h"

class UFogOfWarComponent;
class UFogOfWarProxyWidgetComponent;

UCLASS(BlueprintType, Blueprintable)
class LEARNING2_API AFogOfWarProxyActor : public AActor
{
	GENERATED_BODY()

public:
	AFogOfWarProxyActor();

	virtual void SetActorHiddenInGame(bool bNewHidden) override;

protected:
	void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UFogOfWarComponent> FogOfWarComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UFogOfWarProxyWidgetComponent> FogOfWarProxyWidgetComponent;
};
