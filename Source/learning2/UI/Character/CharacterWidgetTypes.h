//Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterWidgetTypes.generated.h"

class UTexture2D;

USTRUCT(BlueprintType)
struct FUICharacterHealthState final
{
	GENERATED_BODY()

	FORCEINLINE FString ToString() const
	{
		if (HealthMax * .3f > CurrentHealth || FMath::IsNearlyEqual(CurrentHealth, HealthMax)) { return TEXT("Serious Injury"); }
		else if (HealthMax * .6f > CurrentHealth) { return TEXT("Injury"); }
		return {};
	}

	FORCEINLINE FString ToDescription() const { return ToString(); };

	UPROPERTY(BlueprintReadWrite)
	float CurrentHealth{ 0.f };

	UPROPERTY(BlueprintReadWrite)
	float HealthMax{ 0.f };
};

USTRUCT(BlueprintType)
struct FUICharacterWeaponInfo final
{
	GENERATED_BODY()

	bool IsValid()const { return !CurrentWeaponIcon.IsNull(); }

	UPROPERTY(BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> CurrentWeaponIcon;

	UPROPERTY(BlueprintReadWrite)
	int32 MagazineAmmo{ 0 };

	UPROPERTY(BlueprintReadWrite)
	int32 MagazineAmmoMax{ 0 };
};

USTRUCT(BlueprintType)
struct FUICharacterInfo final
{
	GENERATED_BODY()

	//FUICharacterInfo() = default;

	//FUICharacterInfo(TSoftObjectPtr<UTexture2D> CurrentWeaponIcon) : CurrentWeaponIcon(CurrentWeaponIcon) {}

	inline bool IsValid() const { return !CurrentWeaponIcon.IsNull(); }

	//bool operator==(const FUICharacterInfo& Other) const { return Icon == Other.Icon; }

	UPROPERTY(BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> CurrentWeaponIcon;

	UPROPERTY(BlueprintReadWrite)
	FString CurrentWeaponDescription;
};

USTRUCT(BlueprintType)
struct FUIPlayerCharacterInfo final
{
	GENERATED_BODY()

	FUIPlayerCharacterInfo() = default;
	explicit FUIPlayerCharacterInfo(
		const FUICharacterHealthState& UICharacterHealthState,
		const FUICharacterWeaponInfo& ControlledWeaponInfo,
		const FString& ControlledWeaponDescription,
		const TArray<FUICharacterWeaponInfo>& UICharacterWeaponInfos
	) :
		UICharacterHealthState(UICharacterHealthState),
		ControlledWeaponInfo(ControlledWeaponInfo),
		ControlledWeaponDescription(ControlledWeaponDescription),
		UICharacterWeaponInfos(UICharacterWeaponInfos),
		bIsValid(true)
	{
	}

	UPROPERTY(BlueprintReadWrite)
	FUICharacterHealthState UICharacterHealthState;

	UPROPERTY(BlueprintReadWrite)
	FUICharacterWeaponInfo ControlledWeaponInfo;

	UPROPERTY(BlueprintReadWrite)
	FString ControlledWeaponDescription;

	UPROPERTY(BlueprintReadWrite)
	TArray<FUICharacterWeaponInfo> UICharacterWeaponInfos;

	bool bIsValid{ false };
};