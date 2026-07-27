#pragma once

#include "CoreMinimal.h"
#include <concepts>
#include "UObject/Interface.h"
#include "FogOfWarShaderTypes.ush"
#include "RenderGraphUtils.h"
#include "Engine/CurveTable.h"
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
public:
	explicit FGridIndexIterator(FBox Box, const UWorldHeightSubsystem* WorldHeightSubsystem, FOrientedBox OrientedBox, EGridType GridType = EGridType::World);

	void operator++();
	FogOfWarTypes::GridIndexType operator*() const;
	explicit operator bool() const;

	//bool IsValid() const;

private:
	bool IsInBound() const;

private:
	//const FGridBoundsDataType OrientedBox{ FogOfWarConst::kInvalidBox };
	const FBox Box;
	const FOrientedBox OrientedBox;

	TStrongObjectPtr<const UWorldHeightSubsystem> WorldHeightSubsystem{ nullptr };

	double X{ FogOfWarConst::kInfinity };
	double Y{ FogOfWarConst::kInfinity };
	FVector2D GridSize{ FogOfWarConst::kInvalidVector2D };

	bool bIsValid{ false };
	EGridType GridType{ EGridType::World };

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
			//		if (mid > Squared / mid) { right = mid - 1; }// ·ÀÖ¹Òç³ö
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

//template <class KeyType, class ValueType> requires std::is_integral_v<KeyType>
//struct TArrayMap
//{
//	template <class KeyType, class ValueType> requires std::is_integral_v<KeyType>
//	struct TArrayMapElement
//	{
//		KeyType Key{ INDEX_NONE };
//		ValueType Data;
//
//		inline bool IsValid() const { return Key != INDEX_NONE; }
//		inline void Invalidate() { Key = INDEX_NONE; }
//	};
//
//	using ArrayMapElementType = TArrayMapElement<KeyType, ValueType>;
//
//	void Add(const KeyType Key, const ValueType& Data);
//	ValueType* Find(const KeyType Key);
//	const ValueType* Find(const KeyType Key) const;
//	void Remove(const KeyType Key);
//	//ArrayMapElementType* GetData();
//	ArrayMapElementType* GetData();
//	const ArrayMapElementType* GetData() const;
//	void SetNum(const KeyType NewNum);
//	KeyType Num() const;
//
//	void GrowCapacity();
//
//	ArrayMapElementType* Find_Internal(const KeyType Key);
//	const ArrayMapElementType* Find_Internal(const KeyType Key) const;
//	KeyType FindIndexToInsertByKey_Internal(const KeyType Key) const;
//	void ReorderData_Internal(KeyType OldDataNum);
//
//	TArray<ArrayMapElementType> MapData;
//	KeyType Size{ 0 };
//
//	inline constexpr static float CapacityGrowThreshold{ .5f };
//	inline constexpr static float CapacityGrowFactor{ 1.5f };
//};

USTRUCT()
struct FWorldHeightData
{
	GENERATED_BODY()

	enum class EChangeType :uint8
	{
		Added,
		Removed
	};

	//using WorldHeightDataType = TArrayMap<FogOfWarTypes::GridIndexType, FogOfWarTypes::HeightEffectiveNumType>::ArrayMapElementType;
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
struct FGridBounds
{
	GENERATED_BODY()

	FGridBounds() = default;
	FGridBounds(FVector LandMinPosition, FVector LandMaxPosition);

	bool IsValid() const;

	inline bool HasDirtyBounds() const { return DirtyBounds.IsValid(); }

	bool IsPointInBox(const FVector& Point) const;

	//void ExpandRectangleToBounds(const FGridBoundsDataType& OtherGridBounds);
	void AddDirtyBounds(const FGridBoundsDataType& OtherGridBoundsData);

	void FixGridSize();

	//bool GetBoundsDataXYMin(double& X, double& Y) const;
	//bool GetBoundsDataXYMax(double& X, double& Y) const;
	//bool GetBoundsMaxXYMin(double& X, double& Y) const;
	//bool GetBoundsMaxXYMax(double& X, double& Y) const;
	bool GetDirtyBounds(FVector& Center, FVector& BoxExtent) const;
	bool GetLandBounds(FVector& Center, FVector& BoxExtent) const;

	FGridBoundsDataType DirtyBounds;
	FGridBoundsDataType LandBounds;
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

template <class KeyType, class ValueType> requires std::is_integral_v<KeyType>
inline void TArrayMap<KeyType, ValueType>::Add(const KeyType Key, const ValueType& Data)
{
	if (MapData.Num() == 0 || Size > MapData.Num() * CapacityGrowThreshold)
	{
		KeyType OldNum{ MapData.Num() };
		GrowCapacity();
	}

	if (ValueType* Value = Find(Key))
	{
		*Value = Data;
	}
	else
	{
		MapData[FindIndexToInsertByKey_Internal(Key)] = TArrayMapElement{ Key, Data };
		Size++;
	}
}

template <class KeyType, class ValueType> requires std::is_integral_v<KeyType>
inline ValueType* TArrayMap<KeyType, ValueType>::Find(const KeyType Key)
{
	return const_cast<ValueType*>(const_cast<const TArrayMap<KeyType, ValueType>*>(this)->Find(Key));
}

template <class KeyType, class ValueType> requires std::is_integral_v<KeyType>
inline const ValueType* TArrayMap<KeyType, ValueType>::Find(const KeyType Key) const
{
	if (const ArrayMapElementType* Data = Find_Internal(Key))
	{
		return &(Data->Data);
	}

	return nullptr;
}

template <class KeyType, class ValueType> requires std::is_integral_v<KeyType>
inline void TArrayMap<KeyType, ValueType>::Remove(const KeyType Key)
{
	if (ArrayMapElementType* Data = Find_Internal(Key))
	{
		//Data->Key = INDEX_NONE;
		Data->Invalidate();
		Size--;
	}
}

template <class KeyType, class ValueType> requires std::is_integral_v<KeyType>
inline  TArrayMap<KeyType, ValueType>::ArrayMapElementType* TArrayMap<KeyType, ValueType>::GetData()
{
	return const_cast<TArrayMap<KeyType, ValueType>::ArrayMapElementType*>(const_cast<const TArrayMap<KeyType, ValueType>*>(this)->MapData.GetData());
}

template <class KeyType, class ValueType> requires std::is_integral_v<KeyType>
inline const TArrayMap<KeyType, ValueType>::ArrayMapElementType* TArrayMap<KeyType, ValueType>::GetData() const
{
	return MapData.GetData();
}

template <class KeyType, class ValueType> requires std::is_integral_v<KeyType>
inline void TArrayMap<KeyType, ValueType>::SetNum(const KeyType NewNum)
{
	KeyType OldDataNum{ MapData.Num() };

	KeyType NewNumPrime{ NewNum };
	while (UFogOfWarComponentStatics::IsPrimeMillerRabinTest(NewNumPrime) == false)
	{
		NewNumPrime++;
	}

	MapData.SetNum(NewNumPrime);
	ReorderData_Internal(OldDataNum);
}

template <class KeyType, class ValueType> requires std::is_integral_v<KeyType>
inline KeyType TArrayMap<KeyType, ValueType>::Num() const
{
	return MapData.Num();
}

template <class KeyType, class ValueType> requires std::is_integral_v<KeyType>
inline void TArrayMap<KeyType, ValueType>::GrowCapacity()
{
	if (MapData.Num() >= 2)
	{
		SetNum(MapData.Num() * CapacityGrowFactor);
	}
	else
	{
		SetNum(2);
	}
}

template<class KeyType, class ValueType> requires std::is_integral_v<KeyType>
inline TArrayMap<KeyType, ValueType>::ArrayMapElementType* TArrayMap<KeyType, ValueType>::Find_Internal(const KeyType Key)
{
	return const_cast<ArrayMapElementType*>(const_cast<const TArrayMap<KeyType, ValueType>*>(this)->Find_Internal(Key));
}

template<class KeyType, class ValueType> requires std::is_integral_v<KeyType>
inline const TArrayMap<KeyType, ValueType>::ArrayMapElementType* TArrayMap<KeyType, ValueType>::Find_Internal(const KeyType Key) const
{
	if (MapData.Num() == 0) { return nullptr; }

	KeyType FindStep{ 0 };

	KeyType Index = Key % MapData.Num();

	while (MapData[Index].Key != Key)
	{
		Index++;
		FindStep++;

		if (Index >= MapData.Num()) { Index = 0; }

		if (FindStep >= Size) { return nullptr; }
	}

	return &(MapData[Index]);
}

template<class KeyType, class ValueType> requires std::is_integral_v<KeyType>
inline KeyType TArrayMap<KeyType, ValueType>::FindIndexToInsertByKey_Internal(const KeyType Key) const
{
	KeyType IndexToInsert{ Key % MapData.Num() };
	while (MapData[IndexToInsert].IsValid())
	{
		IndexToInsert++;
		if (IndexToInsert >= MapData.Num()) { IndexToInsert = 0; }
	}

	return IndexToInsert;
}

template <class KeyType, class ValueType> requires std::is_integral_v<KeyType>
inline void TArrayMap<KeyType, ValueType>::ReorderData_Internal(const KeyType OldDataNum)
{
	TArray<ArrayMapElementType> TempArray;

	for (ArrayMapElementType& Data : MapData)
	{
		if (Data.IsValid())
		{
			TempArray.Add(Data);
			Data.Invalidate();
		}
	}

	Size = 0;
	for (const ArrayMapElementType& Data : TempArray)
	{
		Add(Data.Key, Data.Data);
	}
}