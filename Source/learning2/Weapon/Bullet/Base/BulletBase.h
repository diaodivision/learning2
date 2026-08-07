// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/BulletInterface.h"
#include "Interface/FreezableInterface.h"
#include "BulletBase.generated.h"

class UGameplayEffect;

UCLASS(Abstract, BlueprintType, Blueprintable)
class LEARNING2_API ABulletBase : public AActor, public IBulletInterface, public IFreezableInterface
{
	GENERATED_BODY()

public:
	virtual FORCEINLINE void InitializeBulletData_Implementation(const UObject*) override {}
	virtual FORCEINLINE void PostInitializedBulletData_Implementation() override {}
	virtual FORCEINLINE void Freeze_Implementation() override {}
	virtual FORCEINLINE void Unfreeze_Implementation() override {}
	virtual FORCEINLINE bool IsFreezing_Implementation() override { return false; }

protected:
	UPROPERTY(BlueprintReadOnly, meta = (ExposeOnSpawn = "true"), Category = "Damage")
	TSubclassOf<UGameplayEffect> EffectClass;
};
