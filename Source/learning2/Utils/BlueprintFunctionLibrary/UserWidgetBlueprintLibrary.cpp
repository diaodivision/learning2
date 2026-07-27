// Fill out your copyright notice in the Description page of Project Settings.

#include "UserWidgetBlueprintLibrary.h"
#include "InteractiveOptionBlueprintLibrary.h"
#include "Kismet/KismetTextLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Interactive/ActorWidgetTypes.h"
#include "Character/CharacterWidgetTypes.h"

float UUserWidgetBlueprintLibrary::ToRate_Tick(const int32 Tick, const int32 TickMax)
{
	return UKismetMathLibrary::SafeDivide(Ceil_Tick(Tick, TickMax), TickMax);
}

float UUserWidgetBlueprintLibrary::Ceil_Rate(const float Rate, const int32 TickMax)
{
	return UKismetMathLibrary::SafeDivide(ToTick_Rate(Rate, TickMax), TickMax);
}

FText UUserWidgetBlueprintLibrary::FormatSecondsText(const float Seconds)
{
	//UKismetTextLibrary::conv
	const static float ErrorTolerance{ SecondsStep / 2 };
	const int32 Decimals{ FMath::IsNearlyEqual(Seconds, FMath::RoundToFloat(Seconds), ErrorTolerance) ? 0 : 1 };

	return UKismetTextLibrary::Conv_DoubleToText(/*Ceil_Seconds(Seconds)*/Seconds, ERoundingMode::HalfToEven, false, true, 1, 324, Decimals, Decimals);
}

//bool UUserWidgetBlueprintLibrary::IsValid(const FWidgetInteractiveOption& Option)
//{
//	return Option.IsValid();
//}

bool UUserWidgetBlueprintLibrary::IsValid_FUICharacterWeaponInfo(const FUICharacterWeaponInfo& Info)
{
	return Info.IsValid();
}

bool UUserWidgetBlueprintLibrary::IsValid_FUICharacterInfo(const FUICharacterInfo& Info)
{
	return Info.IsValid();
}

bool UUserWidgetBlueprintLibrary::IsValid_FUIPlayerCharacterInfo(const FUIPlayerCharacterInfo& Info)
{
	return Info.bIsValid;
}

//bool UUserWidgetBlueprintLibrary::Equals(const FWidgetInteractiveOption& A, const FWidgetInteractiveOption& B)
//{
//	return A == B;
//}
//
//bool UUserWidgetBlueprintLibrary::IsActive(const FWidgetInteractiveOption& Option)
//{
//	return Option.IsActive();
//}

FString UUserWidgetBlueprintLibrary::ToString(const FUICharacterHealthState& UICharacterHealthState)
{
	return UICharacterHealthState.ToString();
}

//bool UUserWidgetBlueprintLibrary::ActivateOption(UPARAM(ref) FWidgetInteractiveOption& Option)
//{
//	return UInteractiveOptionBlueprintLibrary::ActivateOption(Option);
//}
