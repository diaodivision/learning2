// Fill out your copyright notice in the Description page of Project Settings.

//#pragma once
//
//#include "CoreMinimal.h"
//#include "Kismet/BlueprintFunctionLibrary.h"
//#include "FogOfWarComponentStatics.generated.h"
//
//class AActor;
//
//UENUM(BlueprintType)
//enum class ECorner : uint8
//{
//	LeftDown,
//	LeftTop,
//	RightDown,
//	RightTop
//};
//
//USTRUCT(BlueprintType)
//struct FWorldLocationOnScreen2
//{
//	GENERATED_BODY()
//
//	FWorldLocationOnScreen2() = default;
//	explicit FWorldLocationOnScreen2(FVector2D MinPosition, FVector2D MaxPosition, FVector2D ForwardVector = FVector2D{ FVector::ForwardVector }, FVector2D RightVector = FVector2D{ FVector::RightVector });
//
//	bool IsValid() const;
//
//	FVector2D GetCorner(ECorner Corner) const;
//
//	inline FVector2D ForwardVector() const { return RightVector.GetRotated(-90); };
//
//	UPROPERTY(BlueprintReadWrite)
//	FVector2D LeftDownLocation;
//
//	UPROPERTY(BlueprintReadWrite)
//	double Width{ -1 };
//
//	UPROPERTY(BlueprintReadWrite)
//	double Height{ -1 };
//
//	UPROPERTY(BlueprintReadWrite)
//	FVector2D RightVector{ FVector::RightVector };
//};
//
///**
// *
// */
//UCLASS(MinimalAPI)
//class UFogOfWarComponentStatics : public UBlueprintFunctionLibrary
//{
//	GENERATED_BODY()
//
//public:
//	UFUNCTION(BlueprintPure, Category = "Transformation")
//	static LEARNING2_API FWorldLocationOnScreen2 GetActorWorldLocationsOnScreen(const AActor* Actor);
//
//	UFUNCTION(BlueprintPure, Category = "Transformation")
//	static LEARNING2_API FVector2D GetCorner(const FWorldLocationOnScreen2& WorldLocation, ECorner Corner)
//	{
//		return WorldLocation.GetCorner(Corner);
//	}
//
//	UFUNCTION(BlueprintPure, Category = "World Location On Screen", meta = (DisplayName = "Is Valid"))
//	static LEARNING2_API bool IsWorldLocationOnScreen2Valid(const FWorldLocationOnScreen2& WorldLocation)
//	{
//		return WorldLocation.IsValid();
//	}
//
//	UFUNCTION(BlueprintPure, Category = "Camera")
//	static LEARNING2_API bool IsPositionOnScreen(const FVector2D& ScreenPosition);
//
//	UFUNCTION(BlueprintCallable, Category = "Test")
//	static LEARNING2_API void DrawOnScreen(APlayerController* Controller, FVector2f DrawCenter, int32 HalfSize, TArray<FColor>& ColorArray);
//};
