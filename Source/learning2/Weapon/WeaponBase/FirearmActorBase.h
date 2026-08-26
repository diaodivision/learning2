// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponActorBase.h"
//#include "GameplayEffectTypes.h"
#include "FirearmActorBase.generated.h"

class AFireArmBulletBase;
class UGameplayEffect;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShootCooldownFinishedDelegate);

UCLASS(Blueprintable, BlueprintType)
class LEARNING2_API AFirearmActorBase : public AWeaponActorBase
{
	GENERATED_BODY()

	friend class UWeaponActorBlueprintLibrary;

public:
	virtual void OnControl_Implementation(UObject* InOwner) override;
	virtual void OnControlReleased_Implementation() override;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Ability")
	virtual bool Reload() const override;

	virtual void NotifyExecutingShoot();
	virtual void NotifyShootFinish();
	virtual void NotifyShootCooldownFinished() const;

	virtual bool ExecuteFireOnce() override;


protected:
	virtual AActor* SpawnBullet_Implementation() const override;

	virtual void InstantiateAbilityOnBeginPlay() override;

public:
	UPROPERTY(BlueprintAssignable)
	FOnShootCooldownFinishedDelegate OnShootCooldownFinishedDelegate;

protected:
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bullet Class")
	//TSubclassOf<ABulletBase> BulletClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	FWeaponAbilityInfo ReloadAbilityClass;

	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	FGameplayAbilitySpecHandle ReloadAbilityHandle;
};