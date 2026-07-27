// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FogOfWarTypes.h"
#include "RenderGraphBuilder.h"
#include <concepts>
#include <type_traits>
#include "FogOfWarComponentStatics.generated.h"

class AActor;
template<typename T>
concept HLSLCompatible =
std::is_same_v<T, FVector2f> ||
std::is_same_v<T, FIntPoint>;

template<typename T>
concept HLSLTextureCompatible =
std::is_same_v<T, uint8>;

class USceneComponent;

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
	UFUNCTION(BlueprintCallable, Category = "Test")
	static bool IsActorInsideCube(const AActor* Cube, const AActor* Actor);

	[[nodiscard]] static FOGOFWAR_API bool IsPrime(NumberType N);
	[[nodiscard]] static FOGOFWAR_API bool IsPrime(NumberType A, NumberType i, NumberType N);

	[[nodiscard]] static FOGOFWAR_API bool IsPrimeMillerRabinTest(NumberType N, const int8 IterationNum = 50);

	//[[nodiscard]] consteval static FOGOFWAR_API bool IsPrimeMillerRabinTestConst(NumberType N, const int8 IterationNum = 50);

	UFUNCTION(BlueprintPure, Category = "Transformation")
	[[nodiscard]] static FOGOFWAR_API FWorldLocationOnScreen2 GetActorWorldLocationsOnScreen(const AActor* Actor);

	UFUNCTION(BlueprintPure, Category = "Transformation")
	[[nodiscard]] static FOGOFWAR_API FVector2D GetCorner(const FWorldLocationOnScreen2& WorldLocation, ECorner Corner)
	{
		return WorldLocation.GetCorner(Corner);
	}

	UFUNCTION(BlueprintPure, Category = "World Location On Screen", meta = (DisplayName = "Is Valid"))
	[[nodiscard]] static FOGOFWAR_API bool IsWorldLocationOnScreen2Valid(const FWorldLocationOnScreen2& WorldLocation)
	{
		return WorldLocation.IsValid();
	}

	UFUNCTION(BlueprintPure, Category = "Camera")
	[[nodiscard]] static FOGOFWAR_API bool IsPositionOnScreen(const FVector2D& ScreenPosition);

	UFUNCTION(BlueprintPure, Category = "Camera", meta = (DefaultToSelf = WorldContextObject))
	[[nodiscard]] static FOGOFWAR_API bool GetScreenCornersWorldLocation(TArray<FVector>& OutCorners, const UObject* WorldContextObject, const double GroundHeight);

	UFUNCTION(BlueprintPure, Category = "Math")
	static FOGOFWAR_API double GetSignedAngleBetweenVectors2D(const FVector2D& A, const FVector2D& B);

	UFUNCTION(BlueprintCallable, Category = "Test")
	static FOGOFWAR_API void DrawOnScreen(APlayerController* Controller, FVector2f DrawCenter, int32 HalfSize, TArray<FColor>& ColorArray);

	static FOGOFWAR_API FGridBoundsDataType MakeGridBoundsTypeFromActor(const AActor& Actor);

	static void GetActorMinMax(FVector& Min, FVector& Max, const AActor& Actor);

	static void GetBoxPlaneMin(FVector& Min, const FVector& BoxOrigin, const FVector& BoxExtent, const FVector& VectorToPlane, const FVector& ForwardVectorOnPlane, const FVector& RightVectorOnPlane);
	static void GetBoxPlaneMax(FVector& Max, const FVector& BoxOrigin, const FVector& BoxExtent, const FVector& VectorToPlane, const FVector& ForwardVectorOnPlane, const FVector& RightVectorOnPlane);
	static void GetPlaneCorners(FVector& LeftDown, FVector& LeftTop, FVector& RightDown, FVector& RightTop, const FVector& PlaneCenter, const FVector& Extent, const FVector& ForwardVectorOnPlane, const FVector& RightVectorOnPlane);

	template<class ParameterMemberType, class ArrayElementType>
	static void UploadStructedBuffer(ParameterMemberType& ParameterMember, uint32& ParameterMemberOfArrayNum, FRDGBuilder& GraphBuilder, const TArray<ArrayElementType>& Array, const TCHAR* Name);

	template<class ArrayElementType>
	static UTexture2D* CreateTexture2D(const TArray<ArrayElementType>& Array, EPixelFormat PixelFormat);

	//UFUNCTION(meta = (DefaultToSelf = WorldContextObject))
	static bool GetGridSize(FVector2D& GridSize, const EGridType GridType, const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, meta = (DefaultToSelf = WorldContextObject))
	static FVector GetIntersectionFromCameraToGround(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, meta = (DefaultToSelf = WorldContextObject))
	static TArray<FVector> GetCameraFrustumGroundIntersections(const UObject* WorldContextObject, const double GroundHeight = 0.);

	UFUNCTION(BlueprintPure, Category = "Bounds", meta = (DefaultToSelf = Actor))
	static bool GetActorOrientBox(FOrientedBox& OrientedBox, const AActor* Actor);

	UFUNCTION(BlueprintPure, Category = "Bounds", meta = (DefaultToSelf = Component))
	static bool GetComponentOrientBox(FOrientedBox& OrientedBox, const USceneComponent* Component);

	static FBox GetOrientedBoxAABB(const FOrientedBox& OrientedBox);
	static FOrientedBox GetAABBBoxOriented(const FBox& AABB);

private:
	[[nodiscard]] static FOGOFWAR_API NumberType Witness(NumberType A, NumberType i, NumberType N);

	/*[[nodiscard]] consteval static FOGOFWAR_API int64 RandomIntegerInRange(int64& OutResult, const int64 Min, const int64 Max, const int64 LastSeed = 1);*/
};

template<class ParameterMemberType, class ArrayElementType>
inline void UFogOfWarComponentStatics::UploadStructedBuffer(ParameterMemberType& ParameterMember, uint32& ParameterMemberOfArrayNum, FRDGBuilder& GraphBuilder, const TArray<ArrayElementType>& Array, const TCHAR* Name)
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

template<class ArrayElementType>
inline UTexture2D* UFogOfWarComponentStatics::CreateTexture2D(const TArray<ArrayElementType>& Array, EPixelFormat PixelFormat)
{
	UTexture2D* TempTexture = UTexture2D::CreateTransient(FogOfWarConst::kTextureWidth, FogOfWarConst::kTextureHeight, PixelFormat);

	FTexture2DMipMap& Mip = TempTexture->GetPlatformData()->Mips[0];
	void* TextureData = Mip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureData, Array.GetData(), Array.Num() * sizeof(ArrayElementType));
	Mip.BulkData.Unlock();

	TempTexture->UpdateResource();

	return TempTexture;
}