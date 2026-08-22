// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "HUDTypes.h"
#include "GameplayEffectTypes.h"
#include "WidgetController.generated.h"

class UMyAbilitySystemComponent;
class UInputRecordComponent;
class AMyPlayerController;
class UMyAttributeSet;
class AMyCharacterBase;
class APlayerState;
class RewindSubsystem;

/**
 *
 */
UCLASS(Blueprintable)
class LEARNING2_API UWidgetController : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetUpWidgetController(FPossessedCharacterWidgetControllerContext Context);

	virtual void BroadcastInitialData();

	UFUNCTION(BlueprintCallable)
	virtual void OnSliderChanged(const float NewVolume) const;

	UFUNCTION(BlueprintCallable)
	virtual void OnButtonPressed(const ERecordState ButtonType) const;

protected:
	static void InitializeDelegates(FPossessedCharacterWidgetControllerContext& Context, UWidgetController* Self);
	static void DeinitializeDelegates(FPossessedCharacterWidgetControllerContext& Context, UWidgetController* Self);

	//UFUNCTION()
	//void HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);
	void OnCurrentHealthChanged(const FOnAttributeChangeData& OnAttributeChangeData);
	void OnMaxHealthChanged(const FOnAttributeChangeData& OnAttributeChangeData);
	UFUNCTION()
	void OnRewindSubsystemStateChanged(ERecordState OldState, ERecordState NewState);
	UFUNCTION()
	void OnInputRecordComponentTickChanged(int32 OldTick, int32 NewTick, int32 TickMax);
	UFUNCTION()
	void OnWeaponChanged(AWeaponActorBase* OldWeapon, AWeaponActorBase* NewWeapon);
	void OnWeaponAdded(AWeaponActorBase* Weapon);
	void OnWeaponRemoved(AWeaponActorBase* Weapon);
	void OnWeaponMagazineAmmoChanged(const FOnAttributeChangeData& OnAttributeChangeData);

	//void OnControlledWeaponMagazineAmmoChanged(int32 OldAmmo, int32 NewAmmo);
	//void OnControlledWeaponReserveAmmoChanged(int32 OldAmmo, int32 NewAmmo);

	URewindSubsystem* GetRewindSubsystem() const;

public:
	//UPROPERTY(BlueprintAssignable)
	//FOnPossessedPawnChanged OnPossessedPawnChanged;
	UPROPERTY(BlueprintAssignable)
	FOnPossessedCharacterChangedDelegate OnPossessedCharacterChangedDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnCurrentHealthChangedDelegate OnCurrentHealthChangedDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnMaxHealthChangedDelegate OnMaxHealthChangedDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnRewindSubsystemStateChangedDelegate OnRewindSubsystemStateChangedDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnInputRecordComponentTickChangedDelegate OnInputRecordComponentTickChangedDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnWeaponChangedDelegate OnWeaponChangedDelegate;
	//UPROPERTY(BlueprintAssignable)
	//FOnControlledWeaponChangedDelegate OnControlledWeaponChangedDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnWeaponMagazineAmmoChangedDelegate OnWeaponMagazineAmmoChangedDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnWeaponMagazineAmmoMaxChangedDelegate OnWeaponMagazineAmmoMaxChangedDelegate;
	//UPROPERTY(BlueprintAssignable)
	//FOnWeaponReserveAmmoChangedDelegate OnWeaponReserveAmmoChangedDelegate;


protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AMyPlayerController> PlayerController;

	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<UMyAttributeSet> AttributeSet;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<APlayerState> PlayerState;

	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<UMyAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<UInputRecordComponent> InputRecordComponent;

	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<APlayerCharacterBase> MyCharacter;
};
