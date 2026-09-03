// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WeaponTypes.h"
#include "BehaviorTreeStatics.generated.h"

class AActor;
class ACharacter;

/**
 *
 */
UCLASS()
class LEARNING2_API UBehaviorTreeStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static void SetSelfActor(AActor* SelfActor);

	UFUNCTION(BlueprintCallable)
	static void SetEnemyCharacter(ACharacter* Enemy, AActor* Actor);

	UFUNCTION(BlueprintPure)
	static ACharacter* GetEnemyCharacter(AActor* Actor);

	UFUNCTION(BlueprintCallable)
	static void SetWeaponMagazineAmmo(AActor* Actor, const int32 Ammo, const EWeaponSlot Slot);

	UFUNCTION(BlueprintCallable)
	static void SetTargetLocation(const FVector& TargetLocation, AActor* Actor);

	static TOptional<EWeaponSlot> GetControlledWeaponSlot(AActor* Actor);

	UFUNCTION(BlueprintPure)
	static bool GetTargetLocation(FVector& TargetLocation, AActor* Actor);

	UFUNCTION(BlueprintCallable)
	static void SetControlledWeaponSlot(const EWeaponSlot& WeaponSlot, AActor* Actor);

	UFUNCTION(BlueprintCallable)
	static void SetSensedEnemyCharacter(ACharacter* Enemy, AActor* Actor);

	UFUNCTION(BlueprintCallable)
	static void SetWeaponMaxMagazineAmmo(AActor* Actor, const int32 AmmoMax, const EWeaponSlot Slot);

private:
	UFUNCTION(BlueprintPure)
	static bool GetControlledWeaponSlot(EWeaponSlot& WeaponSlot, AActor* Actor);
};
