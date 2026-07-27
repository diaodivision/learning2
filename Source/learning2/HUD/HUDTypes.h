#pragma once

//#include "CoreMinimal.h"
#include "Delegates/DelegateCombinations.h"
#include "RewindSubsystem.h"
#include "GameFramework/Controller.h"
#include "Character/PlayerCharacterBase.h"
#include "InputRecordComponent.h"
#include "GameFramework/PlayerState.h"
#include "HUDTypes.generated.h"

class APawn;
class AWeaponActorBase;
class UTexture2D;
class AMyPlayerController;
class UMyAttributeSet;
class UMyAbilitySystemComponent;
class UInputRecordComponent;
class APlayerCharacterBase;

//class APlayerCharacterBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPossessedCharacterChangedDelegate, APlayerCharacterBase*, OldCharacter, APlayerCharacterBase*, NewCharacter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCurrentHealthChangedDelegate, float, OldHealth, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMaxHealthChangedDelegate, float, OldHealth, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponChangedDelegate, AWeaponActorBase*, NewWeapon, bool, bIsControlled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWeaponMagazineAmmoChangedDelegate, EWeaponSlot, WeaponSlot, int32, OldAmmo, int32, NewAmmo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWeaponMagazineAmmoMaxChangedDelegate, EWeaponSlot, WeaponSlot, int32, OldAmmo, int32, NewAmmo);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWeaponReserveAmmoChangedDelegate, EWeaponSlot, WeaponSlot, float, OldAmmo, float, NewAmmo);

USTRUCT(BlueprintType)
struct FHUDWeaponData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	EWeaponSlot WeaponSlot{ EWeaponSlot::None };

	UPROPERTY(BlueprintReadWrite)
	int32 MagazineAmmo{ 0 };

	//UPROPERTY(BlueprintReadWrite)
	//int32 ReserveAmmo{ 0 };
	UPROPERTY(BlueprintReadWrite)
	int32 MagazineAmmoMax{ 0 };

	UPROPERTY(BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;
};

USTRUCT(BlueprintType)
struct FHUDData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	EWeaponSlot ControlledWeapon{ EWeaponSlot::None };

	UPROPERTY(BlueprintReadWrite)
	FHUDWeaponData Weapon1;

	UPROPERTY(BlueprintReadWrite)
	FHUDWeaponData Weapon2;

	UPROPERTY(BlueprintReadWrite)
	FHUDWeaponData Weapon3;

	UPROPERTY(BlueprintReadWrite)
	FHUDWeaponData Weapon4;

	UPROPERTY(BlueprintReadWrite)
	FHUDWeaponData Weapon5;
};

USTRUCT(BlueprintType)
struct FPossessedCharacterWidgetControllerContext
{
	GENERATED_BODY()

	FPossessedCharacterWidgetControllerContext() = default;
	FPossessedCharacterWidgetControllerContext(APlayerCharacterBase* Character);
	FPossessedCharacterWidgetControllerContext(
		AMyPlayerController* PlayerController,
		UMyAttributeSet* AttributeSet,
		APlayerState* PlayerState,
		UMyAbilitySystemComponent* AbilitySystemComponent,
		UInputRecordComponent* InputRecordComponent,
		APlayerCharacterBase* MyCharacter
	);

	FORCEINLINE bool IsValid() const
	{
		return PlayerController.IsValid() &&
			AttributeSet.IsValid() &&
			PlayerState.IsValid() &&
			AbilitySystemComponent.IsValid() &&
			InputRecordComponent.IsValid() &&
			MyCharacter.IsValid();
	}

	TWeakObjectPtr<AMyPlayerController> PlayerController;

	TWeakObjectPtr<UMyAttributeSet> AttributeSet;

	TWeakObjectPtr<APlayerState> PlayerState;

	TWeakObjectPtr<UMyAbilitySystemComponent> AbilitySystemComponent;

	TWeakObjectPtr<UInputRecordComponent> InputRecordComponent;

	TWeakObjectPtr<APlayerCharacterBase> MyCharacter;
};

//FOnRewindSubsystemStateChangedDelegate			@see #include "RewindSubsystem.h"
//FOnPossessedPawnChanged							@see #include "GameFramework/Controller.h"
//FOnControlledWeaponChangedDelegate				@see #include "Character/Base/MyCharacterBase.h"
//FOnControlledWeaponMagazineAmmoChangedDelegate	@see #include "Character/Base/MyCharacterBase.h"
//FOnControlledWeaponReserveAmmoChangedDelegate		@see #include "Character/Base/MyCharacterBase.h"


//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMagazineAmmo1ChangedDelegate, int32, OldAmmo, int32, NewAmmo);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReserveAmmo1ChangedDelegate, int32, OldAmmo, int32, NewAmmo);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMagazineAmmo2ChangedDelegate, int32, OldAmmo, int32, NewAmmo);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReserveAmmo2ChangedDelegate, int32, OldAmmo, int32, NewAmmo);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMagazineAmmo3ChangedDelegate, int32, OldAmmo, int32, NewAmmo);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReserveAmmo3ChangedDelegate, int32, OldAmmo, int32, NewAmmo);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMagazineAmmo4ChangedDelegate, int32, OldAmmo, int32, NewAmmo);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReserveAmmo4ChangedDelegate, int32, OldAmmo, int32, NewAmmo);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMagazineAmmo5ChangedDelegate, int32, OldAmmo, int32, NewAmmo);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReserveAmmo5ChangedDelegate, int32, OldAmmo, int32, NewAmmo);