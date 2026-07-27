// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAttributeSet.h"

UMyAttributeSet::UMyAttributeSet(
	float MaxHealth,
	float CurrentHealth,
	int32 MagazineAmmo1,
	int32 ReserveAmmo1,
	int32 MagazineAmmo2,
	int32 ReserveAmmo2,
	int32 MagazineAmmo3,
	int32 ReserveAmmo3,
	int32 MagazineAmmo4,
	int32 ReserveAmmo4,
	int32 MagazineAmmo5,
	int32 ReserveAmmo5,
	float FireRate,
	float ReloadTime,
	float ArmorThickness
) : MaxHealth(MaxHealth),
CurrentHealth(CurrentHealth),
MagazineAmmo1(MagazineAmmo1),
ReserveAmmo1(ReserveAmmo1),
MagazineAmmo2(MagazineAmmo2),
ReserveAmmo2(ReserveAmmo2),
MagazineAmmo3(MagazineAmmo3),
ReserveAmmo3(ReserveAmmo3),
MagazineAmmo4(MagazineAmmo4),
ReserveAmmo4(ReserveAmmo4),
MagazineAmmo5(MagazineAmmo5),
ReserveAmmo5(ReserveAmmo5),
FireRate(FireRate),
ReloadTime(ReloadTime),
ArmorThickness(ArmorThickness)
{
};

void UMyAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.1f);
	}

	if (Attribute == GetCurrentHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}

	if (Attribute == GetMagazineAmmo1Attribute())
	{
		//NewValue = FMath::Max(0, NewValue);
		NewValue = NewValue;
	}
	if (Attribute == GetReserveAmmo1Attribute())
	{
		NewValue = FMath::Max(0, NewValue);
	}

	if (Attribute == GetMagazineAmmo2Attribute())
	{
		NewValue = FMath::Max(0, NewValue);
	}
	if (Attribute == GetReserveAmmo2Attribute())
	{
		NewValue = FMath::Max(0, NewValue);
	}

	if (Attribute == GetMagazineAmmo3Attribute())
	{
		NewValue = FMath::Max(0, NewValue);
	}
	if (Attribute == GetReserveAmmo3Attribute())
	{
		NewValue = FMath::Max(0, NewValue);
	}

	if (Attribute == GetMagazineAmmo4Attribute())
	{
		NewValue = FMath::Max(0, NewValue);
	}
	if (Attribute == GetReserveAmmo4Attribute())
	{
		NewValue = FMath::Max(0, NewValue);
	}

	if (Attribute == GetMagazineAmmo5Attribute())
	{
		NewValue = FMath::Max(0, NewValue);
	}
	if (Attribute == GetReserveAmmo5Attribute())
	{
		NewValue = FMath::Max(0, NewValue);
	}

	if (Attribute == GetFireRateAttribute())
	{
		NewValue = FMath::Max(0.f, NewValue);
	}

	if (Attribute == GetReloadTimeAttribute())
	{
		NewValue = FMath::Max(.1f, NewValue);
	}

	if (Attribute == GetArmorThicknessAttribute())
	{
		NewValue = FMath::Max(.1f, NewValue);
	}
}

void UMyAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

}

bool UMyAttributeSet::IsMagazineAmmoAttribute(const FGameplayAttribute& Attribute)
{
	return Attribute.AttributeName.StartsWith("MagazineAmmo");
}

bool UMyAttributeSet::IsReserveAmmoAttribute(const FGameplayAttribute& Attribute)
{
	return Attribute.AttributeName.StartsWith("ReserveAmmo");
}
