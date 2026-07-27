// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "MyAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
/**
 *
 */
UCLASS(BlueprintType)
class LEARNING2_API UMyAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

	UMyAttributeSet() = default;
	UMyAttributeSet(
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
	);

protected:
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

public:
	static bool IsMagazineAmmoAttribute(const FGameplayAttribute& Attribute);
	static bool IsReserveAmmoAttribute(const FGameplayAttribute& Attribute);

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UMyAttributeSet, MaxHealth);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Attributes")
	FGameplayAttributeData CurrentHealth;
	ATTRIBUTE_ACCESSORS(UMyAttributeSet, CurrentHealth);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Attributes")
	FGameplayAttributeData MagazineAmmo1;
	ATTRIBUTE_ACCESSORS(UMyAttributeSet, MagazineAmmo1);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Attributes")
	FGameplayAttributeData ReserveAmmo1;
	ATTRIBUTE_ACCESSORS(UMyAttributeSet, ReserveAmmo1);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Attributes")
	FGameplayAttributeData MagazineAmmo2;
	ATTRIBUTE_ACCESSORS(UMyAttributeSet, MagazineAmmo2);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Attributes")
	FGameplayAttributeData ReserveAmmo2;
	ATTRIBUTE_ACCESSORS(UMyAttributeSet, ReserveAmmo2);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Attributes")
	FGameplayAttributeData MagazineAmmo3;
	ATTRIBUTE_ACCESSORS(UMyAttributeSet, MagazineAmmo3);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Attributes")
	FGameplayAttributeData ReserveAmmo3;
	ATTRIBUTE_ACCESSORS(UMyAttributeSet, ReserveAmmo3);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Attributes")
	FGameplayAttributeData MagazineAmmo4;
	ATTRIBUTE_ACCESSORS(UMyAttributeSet, MagazineAmmo4);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Attributes")
	FGameplayAttributeData ReserveAmmo4;
	ATTRIBUTE_ACCESSORS(UMyAttributeSet, ReserveAmmo4);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Attributes")
	FGameplayAttributeData MagazineAmmo5;
	ATTRIBUTE_ACCESSORS(UMyAttributeSet, MagazineAmmo5);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Attributes")
	FGameplayAttributeData ReserveAmmo5;
	ATTRIBUTE_ACCESSORS(UMyAttributeSet, ReserveAmmo5);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Attributes")
	FGameplayAttributeData FireRate;
	ATTRIBUTE_ACCESSORS(UMyAttributeSet, FireRate);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Attributes")
	FGameplayAttributeData ReloadTime;
	ATTRIBUTE_ACCESSORS(UMyAttributeSet, ReloadTime);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Attributes")
	FGameplayAttributeData ArmorThickness;
	ATTRIBUTE_ACCESSORS(UMyAttributeSet, ArmorThickness);
};
