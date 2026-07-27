// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UserWidgetBlueprintLibrary.generated.h"

struct FWidgetInteractiveOption;
struct FUICharacterHealthState;
struct FUICharacterInfo;

/**
 *
 */
UCLASS(Blueprintable)
class LEARNING2_API UUserWidgetBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe, DisplayName = "To Tick(Rate)"))
	static FORCEINLINE int32 ToTick_Rate(const float Rate, const int32 TickMax) { return Ceil_Tick(Rate * TickMax, TickMax); }

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe, DisplayName = "To Seconds(Rate)"))
	static FORCEINLINE float ToSeconds_Rate(const float Rate, const int32 TickMax) { return ToSeconds_Tick(Rate * TickMax, TickMax); }

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe, DisplayName = "To Seconds(Tick)"))
	static FORCEINLINE float ToSeconds_Tick(const int32 Tick, const int32 TickMax) { return Ceil_Tick(Tick, TickMax) / FrameRate; }

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe, DisplayName = "To Rate(Rate)"))
	static float ToRate_Tick(const int32 Tick, const int32 TickMax);

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe, DisplayName = "Ceil(Tick)"))
	static FORCEINLINE int32 Ceil_Tick(const int32 Tick, const int32 TickMax)
	{
		return Tick >= TickMax ? TickMax : FMath::FloorToInt32((Tick + TickStep - 1) / TickStep) * TickStep;
	}

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe, DisplayName = "Round(Tick)"))
	static FORCEINLINE int32 Round_Tick(const int32 Tick, const int32 TickMax)
	{
		constexpr int32 TickStepHalf{ static_cast<int32>(TickStep / 2.f) };
		return Tick >= TickMax ? TickMax : FMath::FloorToInt32((Tick + TickStepHalf) / TickStep) * TickStep;
	}

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe, DisplayName = "Ceil(Seconds)"))
	static FORCEINLINE float Ceil_Seconds(const float Seconds) { return FMath::CeilToInt32(Seconds / SecondsStep) * SecondsStep; }

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe, DisplayName = "Round(Seconds)"))
	static FORCEINLINE float Round_Seconds(const float Seconds) { return FMath::CeilToInt32((Seconds + SecondsStep / 2) / SecondsStep) * SecondsStep; }

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe, DisplayName = "Floor(Seconds)"))
	static FORCEINLINE float Floor_Seconds(const float Seconds) { return FMath::FloorToInt32(Seconds / SecondsStep) * SecondsStep; }

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe, DisplayName = "Ceil(Rate)"))
	static float Ceil_Rate(const float Rate, const int32 TickMax);

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	static FText FormatSecondsText(const float Seconds);

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	static FORCEINLINE float KeepNDecimalPlaces(const float Value, const int32 N/*Decimals*/)
	{
		const float Pow{ FMath::Pow(10.f,N) };
		return FMath::RoundToFloat(Value * Pow) / Pow;
	}

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe, DisplayName = "Nearly Equal (Rate)", Keywords = "== equal"))
	static FORCEINLINE bool NearlyEquals_RateRate(const float RateA, const float RateB, const int32 TickMax)
	{
		const int32 TickA{ Ceil_Tick(ToTick_Rate(RateA, TickMax), TickMax) };
		const int32 TickB{ Ceil_Tick(ToTick_Rate(RateB, TickMax), TickMax) };

		return TickA == TickB;
	}

	//UFUNCTION(BlueprintCallable)
	//static bool ActivateOption(UPARAM(ref) FWidgetInteractiveOption& Option);

private:
	//UFUNCTION(BlueprintPure)
	//static bool IsValid(const FWidgetInteractiveOption& Option);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Is Valid"))
	static bool IsValid_FUICharacterWeaponInfo(const FUICharacterWeaponInfo& Info);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Is Valid"))
	static bool IsValid_FUICharacterInfo(const FUICharacterInfo& Info);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Is Valid"))
	static bool IsValid_FUIPlayerCharacterInfo(const FUIPlayerCharacterInfo& Info);

	//UFUNCTION(BlueprintPure, meta = (DisplayName = "Equal(==)", Keywords = "== equal"))
	//static bool Equals(const FWidgetInteractiveOption& A, const FWidgetInteractiveOption& B);

	//UFUNCTION(BlueprintPure)
	//static bool IsActive(const FWidgetInteractiveOption& Option);

	UFUNCTION(BlueprintPure)
	static FString ToString(const FUICharacterHealthState& UICharacterHealthState);

public:
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	static FORCEINLINE float GetSecondsStep() { return SecondsStep; }

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	static FORCEINLINE int32 GetFrameRate() { return FrameRate; }

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	static FORCEINLINE int32 GetTickStep() { return TickStep; }

public:
	constexpr static float FrameRate{ 60 };

	constexpr static float SecondsStep{ .1f };

	constexpr static float TickStep{ FrameRate * SecondsStep };
};
