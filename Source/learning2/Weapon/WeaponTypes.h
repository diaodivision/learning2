#pragma once

#include "WeaponTypes.generated.h"

class UDataTable;
class UCurveTable;
class UTexture2D;
class UGameplayAbility;

USTRUCT(BlueprintType)
struct FWeaponAttributeDataTable
{
	GENERATED_BODY()

	inline bool IsValid() const { return AttributeDataTable && AccuracyCurveTable; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Data Table")
	TObjectPtr<const UDataTable> AttributeDataTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Data Table")
	TObjectPtr<const UCurveTable> AccuracyCurveTable;
};

USTRUCT(BlueprintType)
struct FWeaponAttributeData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:

	FWeaponAttributeData() = default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Attribute")
	float Damage{ 0.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Attribute")
	float Penetration{ 0.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Attribute")
	int32 BulletSpawnsOnFire{ 1 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Attribute")
	int32 FireCost{ 0 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Attribute")
	float FireRate{ 0 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Attribute")
	float ReloadTime{ 0.f };

	UPROPERTY(BlueprintReadWrite, Category = "Weapon Attribute")
	int32 MagazineAmmo{ 0 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Attribute")
	int32 MagazineAmmoMax{ 0 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Attribute")
	int32 ChamberCapacity{ 0 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Attribute")
	int32 ReserveAmmo{ 0 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Attribute")
	bool bIsAutomaticWeapon{ false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Attribute")
	float MuzzleSpeed{ 0.f };

	UPROPERTY(BlueprintReadWrite, Category = "Weapon Attribute")
	float Accuracy{ 0.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Attribute")
	TSoftObjectPtr<UTexture2D> Icon{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Attribute")
	FString Description{};
};

UENUM(BlueprintType)
enum class EWeaponType :uint8
{
	UnArmed,
	AR4,
	KA_Val,
	KA47,
	KA74U,
	P320,
	Shotgun,
	SmokeGrenade,
	StunGrenade
};

UENUM(BlueprintType)
enum class EWeaponSlot :uint8
{
	None = 0 UMETA(Hidden, DisplayName = "None (Do Not Use)", ToolTip = "This value is deprecated"),
	Ammo1,
	Ammo2,
	Ammo3,
	Ammo4,
	Ammo5,
	MAX UMETA(Hidden, DisplayName = "None (Do Not Use)", ToolTip = "This value is show the max value of this enum class")
};

namespace WeaponTypePublic
{
	inline bool IsValid(EWeaponSlot WeaponSlot) { return WeaponSlot > EWeaponSlot::None && WeaponSlot < EWeaponSlot::MAX; }
}

USTRUCT(BlueprintType)
struct FSpawnGrenadeParameters
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FVector GrenadeSpawnsLocation{ FVector::ZeroVector };

	UPROPERTY(BlueprintReadWrite)
	FVector GrenadeTargetLocation{ FVector::ZeroVector };
};

UENUM(BlueprintType)
enum class EWeaponAbilityInstancingPolicy : uint8
{
	InstancedOnAddition,
	InstancedOnPossession
};

USTRUCT(BlueprintType)
struct FWeaponAbilityInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<UGameplayAbility> AbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EWeaponAbilityInstancingPolicy InstancingPolicy{ EWeaponAbilityInstancingPolicy::InstancedOnPossession };

	FORCEINLINE bool IsValid() const { return operator bool(); }

	bool operator==(const FWeaponAbilityInfo& Other) const;

	operator bool() const;

	friend uint32 GetTypeHash(const FWeaponAbilityInfo& Request)
	{
		return GetTypeHash(Request.AbilityClass);
	}
};