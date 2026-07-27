// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDTypes.h"
#include "MyUserWidget.generated.h"

class UWidgetController;
class UTexture2D;
class UImage;
struct FStreamableHandle;

/**
 *
 */
UCLASS(Blueprintable)
class LEARNING2_API UMyUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetWidgetController(UWidgetController* InWidgetController);

	//virtual void SetUpWidget(const FHUDData& Data);

	UFUNCTION(BlueprintCallable)
	void SetIconResource(TSoftObjectPtr<UTexture2D> InIconResource);

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void OnWidgetControllerSet();

	//UFUNCTION(BlueprintImplementableEvent)
	//void OnWidgetSet(const FHUDData& HUDData);

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void OnPossessedCharacterChanged(APlayerCharacterBase* OldCharacter, APlayerCharacterBase* NewCharacter);

	UFUNCTION(BlueprintImplementableEvent)
	void OnCurrentHealthChanged(float OldHealth, float NewHealth);

	UFUNCTION(BlueprintImplementableEvent)
	void OnMaxHealthChanged(float OldHealth, float NewHealth);

	UFUNCTION(BlueprintImplementableEvent)
	void OnRewindSubsystemStateChanged(ERecordState OldState, ERecordState NewState);

	UFUNCTION(BlueprintImplementableEvent)
	void OnInputRecordComponentTickChanged(int32 OldTick, int32 NewTick, int32 TickMax);

	UFUNCTION(BlueprintImplementableEvent)
	void OnWeaponChanged(AWeaponActorBase* NewWeapon, bool bIsControlled);

	UFUNCTION(BlueprintImplementableEvent)
	void OnWeaponMagazineAmmoChanged(EWeaponSlot WeaponSlot, int32 OldAmmo, int32 NewAmmo);

	UFUNCTION(BlueprintImplementableEvent)
	void OnWeaponMagazineAmmoMaxChanged(EWeaponSlot WeaponSlot, int32 OldAmmo, int32 NewAmmo);

	//virtual void OnControlledWeaponMagazineAmmoChanged(int32 OldAmmo, int32 NewAmmo);
	//UFUNCTION(BlueprintImplementableEvent)
	//void K2_OnControlledWeaponMagazineAmmoChanged(int32 OldAmmo, int32 NewAmmo);

	//virtual void OnControlledWeaponReserveAmmoChanged(int32 OldAmmo, int32 NewAmmo);
	//UFUNCTION(BlueprintImplementableEvent)
	//void K2_OnControlledWeaponReserveAmmoChanged(int32 OldAmmo, int32 NewAmmo);

//public:
//	UPROPERTY(BlueprintAssignable)
//	FOnPossessedPawnChanged OnPossessedPawnChanged;
//	UPROPERTY(BlueprintAssignable)
//	FOnCurrentHealthChangedDelegate OnCurrentHealthChangedDelegate;
//	UPROPERTY(BlueprintAssignable)
//	FOnMaxHealthChangedDelegate OnMaxHealthChangedDelegate;
//	UPROPERTY(BlueprintAssignable)
//	FOnRewindSubsystemStateChangedDelegate OnRewindSubsystemStateChangedDelegate;
//	UPROPERTY(BlueprintAssignable)
//	FOnControlledWeaponChangedDelegate OnControlledWeaponChangedDelegate;
//	UPROPERTY(BlueprintAssignable)
//	FOnWeaponAmmoChangedDelegate OnWeaponAmmoChangedDelegate;
//	//UPROPERTY(BlueprintAssignable)
//	//FOnControlledWeaponMagazineAmmoChangedDelegate OnControlledWeaponMagazineAmmoChangedDelegate;
//	//UPROPERTY(BlueprintAssignable)
//	//FOnControlledWeaponReserveAmmoChangedDelegate OnControlledWeaponReserveAmmoChangedDelegate;

protected:
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<UWidgetController> WidgetController;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> Icon;

	//UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UTexture2D> IconResource;

	TSharedPtr<FStreamableHandle> LoadingHandle;

private:
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = true))
	bool bIsBindOnPossessedCharacterChangedDelegate{ false };
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = true))
	bool bIsBindOnCurrentHealthChangedDelegate{ false };
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = true))
	bool bIsBindOnMaxHealthChangedDelegate{ false };
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = true))
	bool bIsBindOnRewindSubsystemStateChangedDelegate{ false };
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = true))
	bool bIsBindOnInputRecordComponentTickChangedDelegate{ false };
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = true))
	bool bIsBindOnWeaponChangedDelegate{ false };
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = true))
	bool bIsBindOnWeaponMagazineAmmoChangedDelegate{ false };
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = true))
	bool bIsBindOnWeaponMagazineAmmoMaxChangedDelegate{ false };
};
