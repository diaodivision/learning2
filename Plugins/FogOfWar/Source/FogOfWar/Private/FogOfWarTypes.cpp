#include "FogOfWarTypes.h"
// #include "WorldHeightSubsystem.h"
#include "FogOfWarComponentStatics.h"
#include "GameFramework/Actor.h"
#include "WorldHeightEffectiveActorInterface.h"

FGridSizeType::FGridSizeType(const double X, const double Y, const FGridSizeType::EGridSizeCoordinate GridSizeCoordinate) : FGridSizeType(X, Y, 0., GridSizeCoordinate)
{
}

FGridSizeType::FGridSizeType(const double X, const double Y, const double Z, const FGridSizeType::EGridSizeCoordinate GridSizeCoordinate) : FGridSizeType(FVector{X, Y, Z}, GridSizeCoordinate)
{
}

FGridSizeType::FGridSizeType(const FVector2D& GridSizeValue, const FGridSizeType::EGridSizeCoordinate GridSizeCoordinate) 
	: FGridSizeType(FVector{GridSizeValue, 0.}, GridSizeCoordinate)
{
}

FGridSizeType::FGridSizeType(const FVector& GridSizeValue, const FGridSizeType::EGridSizeCoordinate GridSizeCoordinate)
	: GridSizeValue(GridSizeCoordinate == EGridSizeCoordinate::World ? GridSizeValue : FVector{GridSizeValue.Y, GridSizeValue.X, GridSizeValue.Z})
{
}

FVector FGridSizeType::GetGridSizeOnWorldCoordinate() const
{
	return GridSizeValue;
}

FVector FGridSizeType::GetGridSizeOnScreenCoordinate() const
{
	return FVector{ GridSizeValue.Y, GridSizeValue.X, GridSizeValue.Z };
}

FWorldLocationOnScreen2::FWorldLocationOnScreen2(FVector2D MinPosition, FVector2D MaxPosition)
	:Rectangle(MinPosition, MaxPosition)
{
}

bool FWorldLocationOnScreen2::IsValid() const
{
	if (!Rectangle.bIsValid || Rectangle.Min.ContainsNaN() || Rectangle.Max.ContainsNaN()) { return false; }

	return Rectangle.Min.X < Rectangle.Max.X && Rectangle.Min.Y < Rectangle.Max.Y;
}

FVector2D FWorldLocationOnScreen2::GetCorner(ECorner Corner) const
{
	if (!this->IsValid()) { return FogOfWarConst::kInvalidVector2D; }

	switch (Corner)
	{
	case ECorner::LeftDown:
		return Rectangle.Min;
	case ECorner::LeftTop:
		return Rectangle.GetCenter() - Rectangle.GetExtent() * FVector2D::UnitX() + Rectangle.GetExtent() * FVector2D::UnitY();
	case ECorner::RightDown:
		return Rectangle.GetCenter() + Rectangle.GetExtent() * FVector2D::UnitX() - Rectangle.GetExtent() * FVector2D::UnitY();
	case ECorner::RightTop:
		return Rectangle.Max;
	default:
		return FogOfWarConst::kInvalidVector2D;
	}
}

FWorldHeightEffectiveActorData::FWorldHeightEffectiveActorData(const FOrientedBox& OrientedBox) : OrientedBox(OrientedBox)
{
	//EffectiveArea = Actor.GetComponentsBoundingBox(true);
}

bool FWorldHeightEffectiveActorData::IsValid() const
{
	//if (EffectiveArea.Min.ContainsNaN() || EffectiveArea.Max.ContainsNaN()) { return false; }

	//return EffectiveArea.Min.X < EffectiveArea.Max.X && EffectiveArea.Min.Y < EffectiveArea.Max.Y;
	return OrientedBox.ExtentX > 0 && OrientedBox.ExtentY;
}

FWorldHeightBoundsUpdateRequest::FWorldHeightBoundsUpdateRequest(uint32 UniqueID, FBox AreaBox, FWorldHeightBoundsUpdateRequest::Type UpdateRequest, const AActor* Agent)
	:UniqueID(UniqueID), AreaBox(AreaBox), UpdateRequest(UpdateRequest), Agent(Agent)
{
}

bool FWorldHeightBoundsUpdateRequest::IsValid() const
{
	if (UniqueID < 0 || AreaBox.Min.ContainsNaN() || AreaBox.Max.ContainsNaN()) { return false; }

	return AreaBox.Min.X < AreaBox.Max.X && AreaBox.Min.Y < AreaBox.Max.Y;
}

FGridBoundsDataType::FGridBoundsDataType(FVector MinPosition, FVector MaxPosition) :Box(MinPosition, MaxPosition)
{
}

FGridBoundsDataType::FGridBoundsDataType(const FBox& Box) : Box(Box)
{
}

bool FGridBoundsDataType::IsValid() const
{
	if (!Box.IsValid || Box.Min.ContainsNaN() || Box.Max.ContainsNaN()) { return false; }

	return Box.Min.X < Box.Max.X && Box.Min.Y < Box.Max.Y && Box.Min.Z < Box.Max.Z;
}

FVector FGridBoundsDataType::GetCorner(FVector VectorToCorner) const
{
	if (!IsValid() || VectorToCorner.ContainsNaN()) { return FogOfWarConst::kInvalidVector; }

	FVector Center{ Box.GetCenter() }, BoxExtent{ Box.GetExtent() };

	VectorToCorner.X = FMath::Sign(VectorToCorner.X);
	VectorToCorner.Y = FMath::Sign(VectorToCorner.Y);
	VectorToCorner.Z = FMath::Sign(VectorToCorner.Z);

	return Center + VectorToCorner * BoxExtent;
}

FVector FGridBoundsDataType::GetEdge(FVector VectorToEdge) const
{
	return GetCorner(VectorToEdge);
}

bool FGridBoundsDataType::IsInsideXY(const FVector& Location) const
{
	return IsValid() && Box.IsInsideXY(Location);
}

bool FGridBoundsDataType::IsInsideXY(const FVector2f& Location) const
{
	if (!IsValid()) { return false; }

	return IsInsideXY(FVector{ Location.X, Location.Y, Box.Min.Z });
}

//FogOfWarTypes::GridIndexType FGridBoundsDataType::GetGridIndexOnScreen(const FVector& Location)
//{
//	if (!IsValid() || !Box.IsInsideXY(Location)) { return INDEX_NONE; }
//
//	double X{ FMath::Abs((Location.X - Box.Min.X) / (Box.GetExtent().X * 2)) };
//	double Y{ FMath::Abs((Location.Y - Box.Min.Y) / (Box.GetExtent().Y * 2)) };
//
//	//return FMath::Min(FMath::Floor(Y * GridNumY), GridNumY - 1) * GridNumX + FMath::Min(X * GridNumX, GridNumX - 1);
//	return FMath::Floor(Y * FogOfWarConst::kScreenHeight) * FogOfWarConst::kScreenWidth + X * FogOfWarConst::kScreenWidth;
//}
FogOfWarTypes::GridIndexType FGridBoundsDataType::GetGridIndexOnScreen(const FVector& Location, const FIntPoint& ScreenSize)
{
	// 1. 安全拦截：不在包围盒内直接返回 INDEX_NONE
	if (!IsValid() || !Box.IsInsideXY(Location))
	{
		return INDEX_NONE;
	}

	// 2. 获得当前位置在 Box 内的 0.0 ~ 1.0 的归一化比例
	// 既然已经通过 IsInsideXY 校验，差值必然为正，无需 Abs
	const double RatioX = (Location.X - Box.Min.X) / (Box.GetExtent().X * 2.0);
	const double RatioY = (Location.Y - Box.Min.Y) / (Box.GetExtent().Y * 2.0);

	// 3. 计算离散的网格行列号 (Column 和 Row)
	// 使用 FloorToInt 转换为整数，并用 Min 严格防止边缘处的越界
	//const int32 GridX = FMath::Min(FMath::FloorToInt(RatioX * ScreenSize.X), ScreenSize.X - 1);
	//const int32 GridY = FMath::Min(FMath::FloorToInt(RatioY * ScreenSize.Y), ScreenSize.Y - 1);
	const int32 GridX = FMath::Min(FMath::FloorToInt(RatioX * ScreenSize.X), ScreenSize.X);
	const int32 GridY = FMath::Min(FMath::FloorToInt(RatioY * ScreenSize.Y), ScreenSize.Y);

	// 4. 标准一维化公式：Row * Width + Column
	return GridY * ScreenSize.X + GridX;
}

TOptional<FVector> FGridBoundsDataType::GetGridLocationByIndex(const FogOfWarTypes::GridIndexType Index, const FIntPoint& ScreenSize)
{
	if (!IsValid()) { return NullOpt; }

	const int32 GridX{ Index % ScreenSize.X };
	const int32 GridY{ Index / ScreenSize.X };

	const double NormalizedX = static_cast<double>(GridX) / ScreenSize.X;
	const double NormalizedY = static_cast<double>(GridY) / ScreenSize.Y;

	return FVector{ Box.Min.X + Box.GetSize().X * NormalizedX,Box.Min.Y + Box.GetSize().Y * NormalizedY,Box.GetCenter().Z };
}

FGridIndexIterator::FGridIndexIterator(const FBox& Box, const FGridSizeType& InGridSize, const TFunctionRef<GridIndexType(const FVector&)> GetGridIndexFunction, const FQuat& BoxQuat)
	: BoxExtent(Box.GetExtent()), 
	BoxTransform(FTransform{ BoxQuat, Box.GetCenter() }), 
	GridSize(InGridSize.GetGridSizeOnWorldCoordinate()), 
	GetGridIndexFunction(GetGridIndexFunction)
{
	bIsValid = !FMath::IsNearlyZero(BoxExtent.Size2D()) && GridSize.X > 0.;
	if (!bIsValid) { return; }

	CurrentPosition = FVector{ -BoxExtent.X + GridSize.X * .5, -BoxExtent.Y + GridSize.Y * .5, -BoxExtent.Z + GridSize.Z * .5 };
	if (operator bool() == false) { operator++(); }

#if !UE_BUILD_SHIPPING
	id = ids++;
#endif
}

void FGridIndexIterator::operator++()
{
	do
	{
		CurrentPosition.X += GridSize.X;
		if (CurrentPosition.X < BoxExtent.X || FMath::IsNearlyEqual(GridSize.X, BoxExtent.X)) { continue; }

		if (FMath::IsNearlyZero(GridSize.Y)) { return; }
		CurrentPosition.Y += GridSize.Y;
		CurrentPosition.X = -BoxExtent.X + GridSize.X * .5;

		if (CurrentPosition.Y < BoxExtent.Y || FMath::IsNearlyEqual(GridSize.Y, BoxExtent.Y)) { continue; }

		if (FMath::IsNearlyZero(GridSize.Z)) { return; }
		CurrentPosition.Z += GridSize.Z;
		CurrentPosition.Y = -BoxExtent.Y + GridSize.Y * .5;
	} while (bIsValid && IsInsideOrOn() && operator*() == INDEX_NONE);
}

FogOfWarTypes::GridIndexType FGridIndexIterator::operator*() const
{
	return GetGridIndexFunction(BoxTransform.TransformPosition(CurrentPosition));
}

FGridIndexIterator::operator bool() const
{
	//const FBox& BoundsBox{ Bounds.Box };
	if (!bIsValid) {return false;}
	if (!IsInsideOrOn()) { return false; }
	if (operator*() == INDEX_NONE) { return false; }

#if !UE_BUILD_SHIPPING
	if (/*doonce && */Step > StepMax)
	{
		UE_LOG(LogTemp, Error, TEXT("id: %d\tIterator num larger than %d"), id, StepMax);

		return false;
	}
#endif

	return true;
}

bool FGridIndexIterator::IsInsideOrOn() const
{
	return FMath::Abs(CurrentPosition.X) <= BoxExtent.X && FMath::Abs(CurrentPosition.Y) <= BoxExtent.Y && FMath::Abs(CurrentPosition.Z) <= BoxExtent.Z;
}

FWorldHeightEffectiveActorData* FWorldHeightData::FindWorldHeightEffectiveActor(const AActor& Actor)
{
	return const_cast<FWorldHeightEffectiveActorData*>(const_cast<const FWorldHeightData*>(this)->FindWorldHeightEffectiveActor(Actor));
}

const FWorldHeightEffectiveActorData* FWorldHeightData::FindWorldHeightEffectiveActor(const AActor& Actor) const
{
	return WorldHeightEffectiveActor.Find(&Actor);
}

void FWorldHeightData::AddHeightEffectiveActor(const AActor& Actor, const UWorldHeightSubsystem& WorldHeightSubsystem)
{
	bool bMapElementRemoved = RemoveHeightEffectiveActor_Internal(Actor, WorldHeightSubsystem, false);
	bool bMapElementAdded = AddHeightEffectiveActor_Internal(Actor, WorldHeightSubsystem, false);

	const FWorldHeightEffectiveActorData* Data = WorldHeightEffectiveActor.Find(&Actor);
	//const FBox NewEffectiveArea = Actor.GetComponentsBoundingBox(true);
	FOrientedBox OrientedBox;
	ensure(UFogOfWarComponentStatics::GetActorOrientBox(OrientedBox, &Actor));
	const FOrientedBoxAABBAndQuat OrientedBoxAABBAndQuat{ UFogOfWarComponentStatics::GetOrientedBoxAABBAndQuat(OrientedBox) };

	const FOrientedBoxAABBAndQuat OldOrientedBoxAABBAndQuat{ UFogOfWarComponentStatics::GetOrientedBoxAABBAndQuat(Data->OrientedBox) };

	if (bMapElementAdded && bMapElementRemoved && Data != nullptr && OldOrientedBoxAABBAndQuat.Box.Intersect(OrientedBoxAABBAndQuat.Box))
	{
		const TOptional<FGridSizeType> GridSize {UFogOfWarComponentStatics::GetGridSize(EGridType::World, &WorldHeightSubsystem)};
		if (!GridSize.IsSet()) { return; }

		auto GetGridIndexFunction = [StrongPtr = TStrongObjectPtr(&WorldHeightSubsystem)](const FVector& Location) -> FogOfWarTypes::GridIndexType
		{
			// return StrongPtr ? StrongPtr->GetGridIndex(Location) : INDEX_NONE;
			return StrongPtr ? UFogOfWarComponentStatics::GetGridIndexOnWorld(Location, *StrongPtr) : INDEX_NONE;
		};

		TSet<FogOfWarTypes::GridIndexType> OverlapArea;
		for (FGridIndexIterator It{ OrientedBoxAABBAndQuat.Box, GridSize.GetValue(), GetGridIndexFunction, OrientedBoxAABBAndQuat.Quat }; It; ++It)
		{
			OverlapArea.Add(*It);
		}

		for (FGridIndexIterator It{ OldOrientedBoxAABBAndQuat.Box, GridSize.GetValue(), GetGridIndexFunction, OldOrientedBoxAABBAndQuat.Quat }; It; ++It)
		{
			if (OverlapArea.Find(*It) == nullptr && WorldHeightMap.Find(*It) == nullptr)
			{
				OnWorldHeightEffectiveNumMapElementNumChangeDelegate.Broadcast(EChangeType::Removed);
				break;
			}
		}

		bool bNotifyAdded{ false };
		for (FGridIndexIterator It{ OrientedBoxAABBAndQuat.Box, GridSize.GetValue(), GetGridIndexFunction, OrientedBoxAABBAndQuat.Quat }; It; ++It)
		{
			if (OverlapArea.Find(*It) == nullptr)
			{
				if (FogOfWarTypes::HeightEffectiveNumType* Height = WorldHeightMap.Find(*It); Height && *Height == 1)
				{
					OnWorldHeightEffectiveNumMapElementNumChangeDelegate.Broadcast(EChangeType::Added);
					break;
				}
			}
		}
	}
	else
	{
		if (bMapElementAdded)
		{
			OnWorldHeightEffectiveNumMapElementNumChangeDelegate.Broadcast(EChangeType::Added);
		}

		if (bMapElementRemoved)
		{
			OnWorldHeightEffectiveNumMapElementNumChangeDelegate.Broadcast(EChangeType::Removed);
		}
	}
}

bool FWorldHeightData::AddHeightEffectiveActor_Internal(const AActor& Actor, const UWorldHeightSubsystem& WorldHeightSubsystem, bool bNotifyMapChange)
{
	bool bMapChange{ false };
	if (!Actor.Implements<UWorldHeightEffectiveActorInterface>()) { return bMapChange; }

	const FOrientedBox OrientedBox{ IWorldHeightEffectiveActorInterface::Execute_GetBounds(&Actor) };
	const FOrientedBoxAABBAndQuat OrientedBoxAABBAndQuat{ UFogOfWarComponentStatics::GetOrientedBoxAABBAndQuat(OrientedBox) };
	const TOptional<FGridSizeType> GridSize {UFogOfWarComponentStatics::GetGridSize(EGridType::World, &WorldHeightSubsystem)};
	if (!GridSize.IsSet()) { return bMapChange; }
	auto GetGridIndexFunction = [StrongPtr = TStrongObjectPtr(&WorldHeightSubsystem)](const FVector& Location) -> FogOfWarTypes::GridIndexType
	{
		// return StrongPtr ? StrongPtr->GetGridIndex(Location) : INDEX_NONE;
		return StrongPtr ? UFogOfWarComponentStatics::GetGridIndexOnWorld(Location, *StrongPtr) : INDEX_NONE;
	};

	for (FGridIndexIterator It{ OrientedBoxAABBAndQuat.Box, GridSize.GetValue(), GetGridIndexFunction, OrientedBoxAABBAndQuat.Quat }; It; ++It)
	{
		if (FogOfWarTypes::HeightEffectiveNumType* EffectiveNumPtr = WorldHeightMap.Find(*It))
		{
			(*EffectiveNumPtr)++;
		}
		else
		{
			bMapChange = true;

			if (*It >= 0) { WorldHeightMap.Add(*It, 1); }
			else { UE_LOG(LogTemp, Error, TEXT("*It: %d"), *It); }
		}
	}

	if (bNotifyMapChange && bMapChange)
	{
		OnWorldHeightEffectiveNumMapElementNumChangeDelegate.Broadcast(EChangeType::Added);
	}

	if (FWorldHeightEffectiveActorData* ActorData = FindWorldHeightEffectiveActor(Actor))
	{
		//ActorData->EffectiveArea;
		ActorData->OrientedBox = OrientedBox;
	}
	else
	{
		WorldHeightEffectiveActor.Add(&Actor, FWorldHeightEffectiveActorData{ OrientedBox });
	}

	return bMapChange;
}

void FWorldHeightData::RemoveHeightEffectiveActor(const AActor& Actor, const UWorldHeightSubsystem& WorldHeightSubsystem)
{
	RemoveHeightEffectiveActor_Internal(Actor, WorldHeightSubsystem, true);

	if (WorldHeightEffectiveActor.Find(&Actor))
	{
		WorldHeightEffectiveActor.Remove(&Actor);
	}
}

bool FWorldHeightData::RemoveHeightEffectiveActor_Internal(const AActor& Actor, const UWorldHeightSubsystem& WorldHeightSubsystem, bool bNotifyMapChange)
{
	//if (const FWorldHeightEffectiveActorData* Data = WorldHeightEffectiveActor.Find(&Actor))

	if (const FWorldHeightEffectiveActorData* Data = FindWorldHeightEffectiveActor(Actor))
	{
		bool bMapChange{ false };
		if (!Actor.Implements<UWorldHeightEffectiveActorInterface>()) { return bMapChange; }
		//const FOrientedBox OrientedBox{ IWorldHeightEffectiveActorInterface::Execute_GetBounds(&Actor) };
		const FOrientedBoxAABBAndQuat OrientedBoxAABBAndQuat{UFogOfWarComponentStatics::GetOrientedBoxAABBAndQuat(Data->OrientedBox)};
		const TOptional<FGridSizeType> GridSize {UFogOfWarComponentStatics::GetGridSize(EGridType::World, &WorldHeightSubsystem)};
		if (!GridSize.IsSet()) { return bMapChange; }

		auto GetGridIndexFunction = [StrongPtr = TStrongObjectPtr(&WorldHeightSubsystem)](const FVector& Location) -> FogOfWarTypes::GridIndexType
		{
			// return StrongPtr ? StrongPtr->GetGridIndex(Location) : INDEX_NONE;
			return StrongPtr ? UFogOfWarComponentStatics::GetGridIndexOnWorld(Location, *StrongPtr) : INDEX_NONE;
		};

		for (FGridIndexIterator It{ OrientedBoxAABBAndQuat.Box, GridSize.GetValue(), GetGridIndexFunction, OrientedBoxAABBAndQuat.Quat }; It; ++It)
		{
			if (FogOfWarTypes::HeightEffectiveNumType* EffectiveNumPtr = WorldHeightMap.Find(*It))
			{
				(*EffectiveNumPtr)--;

				if (*EffectiveNumPtr <= 0)
				{
					bMapChange = true;
					WorldHeightMap.Remove(*It);
				}
			}
		}

		if (bNotifyMapChange && bMapChange)
		{
			OnWorldHeightEffectiveNumMapElementNumChangeDelegate.Broadcast(EChangeType::Removed);
		}

		return bMapChange;
	}

	return false;
}

bool FWorldHeightData::RemoveHeightEffectiveActor_Internal(const FWorldHeightEffectiveActorData& Data, const UWorldHeightSubsystem& WorldHeightSubsystem, const AActor* Actor, bool bNotifyMapChange)
{
	bool bMapChange{ false };

	const FOrientedBoxAABBAndQuat OrientedBoxAABBAndQuat{UFogOfWarComponentStatics::GetOrientedBoxAABBAndQuat(Data.OrientedBox)};
	const TOptional<FGridSizeType> GridSize {UFogOfWarComponentStatics::GetGridSize(EGridType::World, &WorldHeightSubsystem)};
	if (!GridSize.IsSet()) { return bMapChange; }

	auto GetGridIndexFunction = [StrongPtr = TStrongObjectPtr(&WorldHeightSubsystem)](const FVector& Location) -> FogOfWarTypes::GridIndexType
	{
		// return StrongPtr ? StrongPtr->GetGridIndex(Location) : INDEX_NONE;
		return StrongPtr ? UFogOfWarComponentStatics::GetGridIndexOnWorld(Location, *StrongPtr) : INDEX_NONE;
	};

	for (FGridIndexIterator It{ OrientedBoxAABBAndQuat.Box, GridSize.GetValue(), GetGridIndexFunction, OrientedBoxAABBAndQuat.Quat }; It; ++It)
	{
		if (FogOfWarTypes::HeightEffectiveNumType* EffectiveNumPtr = WorldHeightMap.Find(*It))
		{
			(*EffectiveNumPtr)--;

			if (*EffectiveNumPtr <= 0)
			{
				bMapChange = true;
				WorldHeightMap.Remove(*It);
			}
		}
	}

	if (bNotifyMapChange && bMapChange)
	{
		OnWorldHeightEffectiveNumMapElementNumChangeDelegate.Broadcast(EChangeType::Removed);
	}

	return bMapChange;
}

void FWorldHeightData::NotifyCleanInvalidWorldHeightEffectiveActorData(const UWorldHeightSubsystem& WorldHeightSubsystem)
{
	CleanInvalidWorldHeightEffectiveActorData_Internal(WorldHeightSubsystem);
}

void FWorldHeightData::CleanInvalidWorldHeightEffectiveActorData_Internal(const UWorldHeightSubsystem& WorldHeightSubsystem)
{
	for (auto It = WorldHeightEffectiveActor.CreateIterator(); It; ++It)
	{
		if (!It->Key.IsValid())
		{
			RemoveHeightEffectiveActor_Internal(It->Value, WorldHeightSubsystem, nullptr, true);

			It.RemoveCurrent();
		}
	}
}