// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponActorBase.h"
//#include "GameplayEffectTypes.h"
#include "FirearmActorBase.generated.h"

class AFireArmBulletBase;
class UGameplayEffect;

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

protected:
	virtual AActor* SpawnBullet_Implementation() const override;

protected:
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bullet Class")
	//TSubclassOf<ABulletBase> BulletClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage Class")
	TSubclassOf<UGameplayEffect> DamageClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	TSubclassOf<UGameplayAbility> ReloadAbilityClass;

	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	FGameplayAbilitySpecHandle ReloadAbilityHandle;
};