#pragma once

#include "CoreMinimal.h"
#include <concepts>
#include "Engine/TextureDefines.h"
#include "HAL/Platform.h"
#include "UObject/Interface.h"
#include "FogOfWarShaderTypes.ush"
#include "RenderGraphUtils.h"
#include "FogOfWarTypes.generated.h"

class UWorldHeightSubsystem;
class UFogOfWarComponentStatics;

namespace FogOfWarConst
{
	constexpr static const TCHAR* MaterialPath{ TEXT("/Game/Experimental/NewMaterial") };
	constexpr static EPixelFormat PixelFormat{ EPixelFormat::PF_G8 };
	constexpr static TEnumAsByte<enum TextureFilter> TextureFilter{ TF_MAX };

	constexpr static uint8 kThreadsX{ FComputeShaderUtils::kGolden2DGroupSize };
	constexpr static uint8 kThreadsY{ FComputeShaderUtils::kGolden2DGroupSize };
	constexpr static uint8 kThreadsZ{ 1 };

	constexpr static int16 kTextureWidth{ 1024 };
	constexpr static int16 kTextureHeight{ 1024 };

	constexpr static double kInfinity{ std::numeric_limits<double>::infinity() };
	inline const static FVector2D kInvalidVector2D{ kInfinity, kInfinity };
	inline const static FVector kInvalidVector{ kInfinity, kInfinity , kInfinity };
	inline const static FBox2D kInvalidBox2D{ kInvalidVector2D , kInvalidVector2D };
	inline const static FBox kInvalidBox{ kInvalidVector , kInvalidVector };
};

namespace FogOfWarTypes
{
	using GridNumType = int16;
	using GridIndexType = int32;
	using HeightEffectiveNumType = int32;
};

UENUM(BlueprintType)
enum class ECorner : uint8
{
	LeftDown,
	LeftTop,
	RightDown,
	RightTop
};

enum class EGridType : uint8
{
	World,
	Screen
};

enum class EAllowMinusPosition : uint8
{
	Yes,
	No
};

struct FOrientedBoxAABBAndQuat
{
	FBox Box;
	FQuat Quat;
};

struct FOrientedBoxAABBAndTransform
{
	FBox Box;
	FTransform Transform;
};

struct FGridSizeType
{
	enum class EGridSizeCoordinate : uint8
	{
		World,
		Screen
	};

	FGridSizeType(const double X, const double Y, const EGridSizeCoordinate GridSizeCoordinate);
	FGridSizeType(const double X, const double Y, const double Z, const EGridSizeCoordinate GridSizeCoordinate);
	FGridSizeType(const FVector2D& GridSizeValue, const EGridSizeCoordinate GridSizeCoordinate);
	FGridSizeType(const FVector& GridSizeValue, const EGridSizeCoordinate GridSizeCoordinate);

	FVector GetGridSizeOnWorldCoordinate() const;

	FVector GetGridSizeOnScreenCoordinate() const;

private:
	const FVector GridSizeValue;
};

USTRUCT(BlueprintType)
struct FWorldLocationOnScreen2
{
	GENERATED_BODY()

	FWorldLocationOnScreen2() = default;
	explicit FWorldLocationOnScreen2(FVector2D MinPosition, FVector2D MaxPosition);

	bool IsValid() const;

	FVector2D GetCorner(ECorner Corner) const;

	FBox2D Rectangle{ FogOfWarConst::kInvalidBox2D };
};

USTRUCT()
struct FFogOfWarData
{
	GENERATED_BODY()

	bool bIsValid{ false };

	FVector ActorLocation;
	FVector ActorVisionLeft;
	FVector ActorVisionRight;
	float Radius{ 0.f };
};

USTRUCT()
struct FWorldHeightEffectiveActorData
{
	GENERATED_BODY()

	FWorldHeightEffectiveActorData() = default;
	explicit FWorldHeightEffectiveActorData(const FOrientedBox& OrientedBox);

	bool IsValid() const;

	FOrientedBox OrientedBox/*{ FogOfWarConst::kInvalidBox }*/;
};

USTRUCT()
struct FWorldHeightBoundsUpdateRequest
{
	GENERATED_BODY()

	enum class Type : uint8
	{
		Added,
		Removed,
	};

	FWorldHeightBoundsUpdateRequest() = default;
	explicit FWorldHeightBoundsUpdateRequest(uint32 UniqueID, FBox AreaBox, Type UpdateRequest, const AActor* Agent);

	bool IsValid() const;

	bool operator==(const FWorldHeightBoundsUpdateRequest& Other) const
	{
		return UniqueID == Other.UniqueID && UpdateRequest == Other.UpdateRequest;
	}

	friend uint32 GetTypeHash(const FWorldHeightBoundsUpdateRequest& Request)
	{
		return GetTypeHash(Request.UniqueID);
	}

	uint32 UniqueID{ 0 };
	FBox AreaBox{ FogOfWarConst::kInvalidBox };
	Type UpdateRequest;

	UPROPERTY()
	TWeakObjectPtr<const AActor> Agent{ nullptr };
};

USTRUCT(BlueprintType)
struct FGridBoundsDataType
{
	GENERATED_BODY()

	FGridBoundsDataType() = default;
	explicit FGridBoundsDataType(FVector MinPosition, FVector MaxPosition);
	explicit FGridBoundsDataType(const FBox& Box);

	bool IsValid() const;

	FVector GetCorner(FVector VectorToCorner) const;
	FVector GetEdge(FVector VectorToEdge) const;

	bool IsInsideXY(const FVector& Location) const;
	bool IsInsideXY(const FVector2f& Location) const;

	FogOfWarTypes::GridIndexType GetGridIndexOnScreen(const FVector& Location, const FIntPoint& ScreenSize);
	TOptional<FVector> GetGridLocationByIndex(const FogOfWarTypes::GridIndexType Index, const FIntPoint& ScreenSize);

	FBox Box{ FogOfWarConst::kInvalidBox };

	inline const static FVector BoxForward{ 1, 0, 0 };
	inline const static FVector BoxBack{ -BoxForward };
	inline const static FVector BoxRight{ 0, 1, 0 };
	inline const static FVector BoxLeft{ -BoxRight };
	inline const static FVector BoxUp{ 0, 0, 1 };
	inline const static FVector BoxDown{ -BoxUp };

};

class FGridIndexIterator
{
	using GridIndexType = FogOfWarTypes::GridIndexType;

public:
	explicit FGridIndexIterator(const FBox& Box, const FGridSizeType& InGridSize, const TFunctionRef<GridIndexType(const FVector&)> GetGridIndexFunction, const FQuat& BoxQuat = FQuat::Identity);

	FORCENOINLINE void operator++();
	FORCENOINLINE GridIndexType operator*() const;
	FORCENOINLINE explicit operator bool() const;

	//bool IsValid() const;

private:
 	FORCENOINLINE bool IsInsideOrOn() const;

private:
	const FVector BoxExtent;
	const FTransform BoxTransform;
	const FVector GridSize{ FVector::ZeroVector };
	const TFunctionRef<GridIndexType(const FVector&)> GetGridIndexFunction;
	
	bool bIsValid{ false };
	FVector CurrentPosition;

#if !UE_BUILD_SHIPPING
	inline static int32 ids{ 1 };
	int32 id;
	mutable int32 Step{ 0 };
	inline constexpr static int32 StepMax = []() constexpr
		{
			constexpr int32 Width = FogOfWarConst::kTextureWidth;
			constexpr int32 Height = FogOfWarConst::kTextureHeight;
			return Width * Height;
			//constexpr int32 Squared = Width * Width + Height * Height;

			//int32 Result;

			//if (Squared <= 1) { Result = Squared + 10; }
			//else
			//{
			//	int32 left = 1, right = Squared;
			//	while (left <= right)
			//	{
			//		int32 mid = left + (right - left) / 2;
			//		if (mid > Squared / mid) { right = mid - 1; }// 防止溢出
			//		else if (mid + 1 > Squared / (mid + 1))
			//		{
			//			Result = mid + 10;
			//			break;
			//		}
			//		else { left = mid + 1; }
			//	}
			//	Result = right + 10;
			//}

			//return Result;
		}();;
#endif
};

USTRUCT()
struct FWorldHeightData
{
	GENERATED_BODY()

	enum class EChangeType :uint8
	{
		Added,
		Removed
	};

	using FWorldHeightMapType = TMap<FogOfWarTypes::GridIndexType, FogOfWarTypes::HeightEffectiveNumType>;
	using FWorldHeightMapConstIterator = FWorldHeightMapType::TConstIterator;

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnWorldHeightEffectiveNumMapElementNumChangeDelegate, FWorldHeightData::EChangeType);

	FWorldHeightData() = default;

	FWorldHeightEffectiveActorData* FindWorldHeightEffectiveActor(const AActor& Actor);
	const FWorldHeightEffectiveActorData* FindWorldHeightEffectiveActor(const AActor& Actor) const;

	void AddHeightEffectiveActor(const AActor& Actor, const UWorldHeightSubsystem& WorldHeightSubsystem);
	bool AddHeightEffectiveActor_Internal(const AActor& Actor, const UWorldHeightSubsystem& WorldHeightSubsystem, bool bNotifyMapChange);
	void RemoveHeightEffectiveActor(const AActor& Actor, const UWorldHeightSubsystem& WorldHeightSubsystem);
	bool RemoveHeightEffectiveActor_Internal(const AActor& Actor, const UWorldHeightSubsystem& WorldHeightSubsystem, bool bNotifyMapChange);
	bool RemoveHeightEffectiveActor_Internal(const FWorldHeightEffectiveActorData& Data, const UWorldHeightSubsystem& WorldHeightSubsystem, const AActor* Actor, bool bNotifyMapChange);

	void NotifyCleanInvalidWorldHeightEffectiveActorData(const UWorldHeightSubsystem& WorldHeightSubsystem);
	void CleanInvalidWorldHeightEffectiveActorData_Internal(const UWorldHeightSubsystem& WorldHeightSubsystem);

	//TArrayMap<FogOfWarTypes::GridIndexType, FogOfWarTypes::HeightEffectiveNumType> WorldHeightEffectiveNumMap;
	FWorldHeightMapType WorldHeightMap;

	UPROPERTY()
	TMap<TWeakObjectPtr<const AActor>, FWorldHeightEffectiveActorData> WorldHeightEffectiveActor;

	FOnWorldHeightEffectiveNumMapElementNumChangeDelegate OnWorldHeightEffectiveNumMapElementNumChangeDelegate;
};

USTRUCT()
struct FGridBounds : public FBox
{
	GENERATED_BODY()

	FGridBounds() = default;
	explicit FGridBounds(const FBox& InBox) : FBox(InBox) {}

	using FBox::FBox;

	//FORCEINLINE TOptional<FVector> GetCorner(const FVector& VectorToCorner) const
	//{
	//	if (GetSize().IsNearlyZero()) { return NullOpt; }
	//	if (VectorToCorner.GetMin() == VectorToCorner.GetMax() == 1) { return GetCenter() + VectorToCorner * GetExtent(); }

	//	return NullOpt;
	//}

	//FORCEINLINE TOptional<FVector> GetEdge(const FVector& VectorToEdge) const
	//{
	//	if (GetSize().IsNearlyZero()) { return NullOpt; }
	//	if (VectorToEdge.GetMin() != 0/* || VectorToEdge.GetMax() != 1*/ || VectorToEdge.Length() != 1) { return NullOpt; }

	//	return GetCenter() + VectorToEdge * GetExtent();
	//}

	//FogOfWarTypes::GridIndexType GetGridIndexOnScreen(const FVector& Location, const FIntPoint& ScreenSize);

	//inline const static FVector BoxForward{ 1, 0, 0 };
	//inline const static FVector BoxBack{ -BoxForward };
	//inline const static FVector BoxRight{ 0, 1, 0 };
	//inline const static FVector BoxLeft{ -BoxRight };
	//inline const static FVector BoxUp{ 0, 0, 1 };
	//inline const static FVector BoxDown{ -BoxUp };
};

UINTERFACE(Blueprintable, MinimalAPI)
class UWorldLandInterface : public UInterface
{
	GENERATED_BODY()
};

class FOGOFWAR_API IWorldLandInterface
{
	GENERATED_BODY()
};