// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/GrenadeBulletBase.h"
#include "SmokeGrenadeBullet.generated.h"

class USphereComponent;
class UPrimitiveComponent;
class AActor;
struct FHitResult;

UCLASS(Blueprintable, BlueprintType)
class LEARNING2_API ASmokeGrenadeBullet : public AGrenadeBulletBase
{
	GENERATED_BODY()

	ASmokeGrenadeBullet();

protected:
	void CreateSphereComponent();

	virtual void OnReachLifeTime_Implementation() override;

	UFUNCTION()
	virtual void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<USphereComponent> SphereComponent;
};