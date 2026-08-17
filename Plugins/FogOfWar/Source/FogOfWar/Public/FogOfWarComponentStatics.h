// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// #include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FogOfWarTypes.h"
#include "Engine/Texture2D.h"
#include "RenderGraphBuilder.h"
#include "TextureResource.h"
#include "FogOfWarComponentStatics.generated.h"

class AActor;
class USceneComponent;
class UFogOfWarSubsystem;
class UWorldHeightSubsystem;

// Whether to inline functions at all
#define FOGOFWAR_INLINE_ENABLED	(!UE_BUILD_DEBUG)

#if FOGOFWAR_INLINE_ENABLED
#define FOGOFWAR_INL_API
#else
#define FOGOFWAR_INL_API FOGOFWAR_API
#endif

/**
 *
 */
UCLASS(MinimalAPI)
class UFogOfWarComponentStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	using NumberType = int32;

public:
	static FOGOFWAR_INL_API FGridBoundsDataType MakeGridBoundsTypeFromActor(const AActor& Actor);

	static TOptional<FGridSizeType> GetGridSize(const EGridType GridType, const UObject* WorldContextObject);
	static TOptional<FIntPoint> GetScreenSize(const UObject* WorldContextObject);

	// UFUNCTION(BlueprintPure, meta = (DefaultToSelf = WorldContextObject))
	// static FVector GetIntersectionFromCameraToGround(const UObject* WorldContextObject);

	static TOptional<FBox2D> GetCameraFrustumGroundIntersections(const UObject* WorldContextObject, const double GroundHeight = 0.);
	static TOptional<FBox2D> GetLandBoundingBox(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "Bounds", meta = (DefaultToSelf = Actor))
	static bool GetActorOrientBox(FOrientedBox& OrientedBox, const AActor* Actor);

	UFUNCTION(BlueprintPure, Category = "Bounds", meta = (DefaultToSelf = Component))
	static bool GetComponentOrientBox(FOrientedBox& OrientedBox, const USceneComponent* Component);

	static FBox GetOrientedBoxAABB(const FOrientedBox& OrientedBox);
	static FOrientedBoxAABBAndQuat GetOrientedBoxAABBAndQuat(const FOrientedBox& OrientedBox);
	static FOrientedBoxAABBAndTransform GetOrientedBoxAABBAndTransform(const FOrientedBox& OrientedBox);
	static FOrientedBox GetAABBBoxOriented(const FBox& AABB);

	static TOptional<FIntPoint> GetGridPositionOnScreen(const FVector& WorldLocation, const UObject* WorldContextObject, const EAllowMinusPosition AllowMinusPosition = EAllowMinusPosition::No);
	static FogOfWarTypes::GridIndexType GetGridIndexOnScreen(const FVector& WorldLocation, const UObject* WorldContextObject);
	static TOptional<FIntPoint> GetGridPositionOnScreen(const FVector& WorldLocation, const FIntPoint& ScreenSize, const FBox2D& ScreenBoundingBox, const FBox2D& LandBoundingBox, const EAllowMinusPosition AllowMinusPosition = EAllowMinusPosition::No);
	static FogOfWarTypes::GridIndexType GetGridIndexOnScreen(const FVector& WorldLocation, const FIntPoint& ScreenSize, const FBox2D& ScreenBoundingBox, const FBox2D& LandBoundingBox);
	
	static FVector2D ProjectWorldDirectionToScreen(const FVector& WorldDirection);	
	
	static TOptional<FIntPoint> GetGridPosition(const FVector& WorldLocation, const UObject* WorldContextObject, const EAllowMinusPosition AllowMinusPosition = EAllowMinusPosition::No);
	static FogOfWarTypes::GridIndexType GetGridIndexOnWorld(const FVector& WorldLocation, const UObject* WorldContextObject, const EAllowMinusPosition AllowMinusPosition = EAllowMinusPosition::No);
	static FogOfWarTypes::GridIndexType GetGridIndexOnWorld(const FVector& WorldLocation, const UWorldHeightSubsystem& WorldHeightSubsystem, const EAllowMinusPosition AllowMinusPosition = EAllowMinusPosition::No);
	static TOptional<FIntPoint> GetGridPositionOnWorld(const FVector& WorldLocation, const UObject* WorldContextObject, const EAllowMinusPosition AllowMinusPosition = EAllowMinusPosition::No);
	static TOptional<FIntPoint> GetGridPositionOnWorld(const FVector& WorldLocation, const UWorldHeightSubsystem& WorldHeightSubsystem, const EAllowMinusPosition AllowMinusPosition = EAllowMinusPosition::No);
	static TOptional<FIntPoint> GetGridPositionOnWorld(const FVector2D& WorldLocation, const UObject* WorldContextObject, const EAllowMinusPosition AllowMinusPosition = EAllowMinusPosition::No);
	static TOptional<FIntPoint> GetGridPositionOnWorld(const FVector2D& WorldLocation, const UWorldHeightSubsystem& WorldHeightSubsystem, const EAllowMinusPosition AllowMinusPosition = EAllowMinusPosition::No);

	static UFogOfWarSubsystem* GetFogOfWarSubsystem(const UObject* WorldContextObject);
	static UWorldHeightSubsystem* GetWorldHeightSubsystem(const UObject* WorldContextObject);

public:
	template<class ParameterMemberType, class ArrayElementType>
	static void UploadStructedBuffer(ParameterMemberType& ParameterMember, uint32& ParameterMemberOfArrayNum, FRDGBuilder& GraphBuilder, const TArray<ArrayElementType>& Array, const TCHAR* Name);

private:
	[[nodiscard]] static bool IsPrime(NumberType N);
	[[nodiscard]] static bool IsPrime(NumberType A, NumberType i, NumberType N);
	[[nodiscard]] static bool IsPrimeMillerRabinTest(NumberType N, const int8 IterationNum = 50);
	[[nodiscard]] static FOGOFWAR_INL_API NumberType Witness(NumberType A, NumberType i, NumberType N);
};

#undef FOGOFWAR_INL_API

template<class ParameterMemberType, class ArrayElementType>
void UFogOfWarComponentStatics::UploadStructedBuffer(ParameterMemberType& ParameterMember, uint32& ParameterMemberOfArrayNum, FRDGBuilder& GraphBuilder, const TArray<ArrayElementType>& Array, const TCHAR* Name)
{
	FRDGBufferRef RDGBuffer = CreateStructuredBuffer(
		GraphBuilder,
		Name,
		sizeof(ArrayElementType),
		Array.Num(),
		Array.GetData(),
		sizeof(ArrayElementType) * Array.Num()
	);

	ParameterMember = GraphBuilder.CreateSRV(RDGBuffer);
	ParameterMemberOfArrayNum = Array.Num();
}

#if FOGOFWAR_INLINE_ENABLED
#define FOGOFWAR_ALLOW_INCLUDE_INL
#include "FogOfWarComponentStatics.inl"
#undef FOGOFWAR_ALLOW_INCLUDE_INL
#endif 