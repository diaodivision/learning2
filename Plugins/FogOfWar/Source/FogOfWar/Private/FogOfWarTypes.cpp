#include "FogOfWarTypes.h"
#include "WorldHeightSubsystem.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "WorldHeightEffectiveActorInterface.h"

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

FGridIndexIterator::FGridIndexIterator(FBox Box, const UWorldHeightSubsystem* WorldHeightSubsystem, FOrientedBox OrientedBox, EGridType GridType)
	:Box(Box), OrientedBox(OrientedBox), WorldHeightSubsystem(WorldHeightSubsystem)
{
	if (WorldHeightSubsystem)
	{
		//if (GridType == EGridType::World) { WorldHeightSubsystem->GetGridSize(GridSize); }
		//else ()

		UFogOfWarComponentStatics::GetGridSize(GridSize, GridType, WorldHeightSubsystem);
		bIsValid = GridSize.GetMin() >= 0 && !GridSize.ContainsNaN();

		if (!bIsValid) { return; }
		X = Box.Min.X + GridSize.X * .5f;
		Y = Box.Min.Y + GridSize.Y * .5f;
		if (!IsInBound()) { operator++(); }
	}

#if !UE_BUILD_SHIPPING
	id = ids++;
#endif
}

void FGridIndexIterator::operator++()
{
	do
	{
#if !UE_BUILD_SHIPPING
		if (Step++ > StepMax) { return; }
#endif
		Y += GridSize.Y;
		if (Box.Max.Y <= Y)
		{
			Y = Box.Min.Y;
			X += GridSize.X;
		}

		if (IsInBound()) { break; }
		//if (Actor)
		//{
		//	FOrientedBox ActorObb;
		//	{
		//		ActorObb.Center = Actor->GetActorLocation();
		//		ActorObb.AxisX = Actor->GetActorForwardVector();
		//		ActorObb.AxisY = Actor->GetActorRightVector();
		//		ActorObb.AxisZ = Actor->GetActorUpVector();

		//		FVector ActorOrigin, ActorExtent;
		//		Actor->GetActorBounds(false, ActorOrigin, ActorExtent);
		//		ActorObb.Center = ActorOrigin;

		//		USceneComponent* RootComp = Actor->GetRootComponent();
		//		// 1. 获取组件在“未应用任何世界变换（即局部空间）”下的原始 Bounding Box
		//		// 很多底层组件（如 Mesh, Shape）都会重写这个函数来返回自己最原始的盒体大小
		//		FTransform Transform{ FTransform::Identity };
		//		Transform.SetScale3D(Actor->GetActorScale3D());
		//		FBox LocalBox = RootComp->CalcBounds(Transform).GetBox();


		//		ActorObb.ExtentX = FMath::Abs(LocalBox.GetExtent().X);
		//		ActorObb.ExtentY = FMath::Abs(LocalBox.GetExtent().Y);
		//		ActorObb.ExtentZ = FMath::Abs(LocalBox.GetExtent().Z);
		//	}

		//	const FVector Direction{ FVector{X, Y, 0} - ActorObb.Center };
		//	const bool bConditionX{ FMath::Abs(FVector::DotProduct(Direction, ActorObb.AxisX)) <= ActorObb.ExtentX };
		//	const bool bConditionY{ FMath::Abs(FVector::DotProduct(Direction, ActorObb.AxisY)) <= ActorObb.ExtentY };
		//	const bool bConditionZ{ FMath::Abs(FVector::DotProduct(Direction, ActorObb.AxisZ)) <= ActorObb.ExtentZ };

		//	//if (/*Step < 1000 && */ActorObb.AxisX != FVector::ForwardVector && FMath::Abs(X - Actor->GetActorLocation().X) < 100 && FMath::Abs(Y - Actor->GetActorLocation().Y) < 100)
		//	//{
		//	//	UE_LOG(LogTemp, Error, TEXT("UFogOfWarComponentStatics::IsActorInsideCube X %f"), X);
		//	//	UE_LOG(LogTemp, Error, TEXT("UFogOfWarComponentStatics::IsActorInsideCube Y %f"), Y);
		//	//	UE_LOG(LogTemp, Error, TEXT("UFogOfWarComponentStatics::IsActorInsideCube Actor %s"), *GetNameSafe(Actor));
		//	//	UE_LOG(LogTemp, Error, TEXT("UFogOfWarComponentStatics::IsActorInsideCube Direction %s"), *Direction.ToString());
		//	//	UE_LOG(LogTemp, Error, TEXT("UFogOfWarComponentStatics::IsActorInsideCube ActorObb.AxisX %s"), *ActorObb.AxisX.ToString());
		//	//	UE_LOG(LogTemp, Error, TEXT("UFogOfWarComponentStatics::IsActorInsideCube ActorObb.AxisY %s"), *ActorObb.AxisY.ToString());
		//	//	UE_LOG(LogTemp, Error, TEXT("UFogOfWarComponentStatics::IsActorInsideCube ActorObb.AxisZ %s"), *ActorObb.AxisZ.ToString());
		//	//	UE_LOG(LogTemp, Error, TEXT("UFogOfWarComponentStatics::IsActorInsideCube FVector::DotProduct(Direction, ActorObb.AxisX) %f"), FVector::DotProduct(Direction, ActorObb.AxisX));
		//	//	UE_LOG(LogTemp, Error, TEXT("UFogOfWarComponentStatics::IsActorInsideCube FVector::DotProduct(Direction, ActorObb.AxisY) %f"), FVector::DotProduct(Direction, ActorObb.AxisY));
		//	//	UE_LOG(LogTemp, Error, TEXT("UFogOfWarComponentStatics::IsActorInsideCube FVector::DotProduct(Direction, ActorObb.AxisZ) %f"), FVector::DotProduct(Direction, ActorObb.AxisZ));
		//	//	UE_LOG(LogTemp, Error, TEXT("UFogOfWarComponentStatics::IsActorInsideCube ActorObb.ExtentX %f"), ActorObb.ExtentX);
		//	//	UE_LOG(LogTemp, Error, TEXT("UFogOfWarComponentStatics::IsActorInsideCube ActorObb.ExtentY %f"), ActorObb.ExtentY);
		//	//	UE_LOG(LogTemp, Error, TEXT("UFogOfWarComponentStatics::IsActorInsideCube ActorObb.ExtentZ %f"), ActorObb.ExtentZ);
		//	//}

		//	if (bConditionX && bConditionY && bConditionZ) { break; }
		//}
	} while (operator bool());
}

FogOfWarTypes::GridIndexType FGridIndexIterator::operator*() const
{
	return WorldHeightSubsystem->GetGridIndex(FVector2D{ X, Y });
}

FGridIndexIterator::operator bool() const
{
	//const FBox& BoundsBox{ Bounds.Box };
	if (!bIsValid) { return false; }
	if (X == FogOfWarConst::kInfinity || Y == FogOfWarConst::kInfinity) { return false; }
	if (X < Box.Min.X || Box.Max.X < X || Y < Box.Min.Y || Box.Max.Y < Y) { return false; }
	//if (OrientedBox.ExtentX > 0 && OrientedBox.ExtentY > 0) { return false; }
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

bool FGridIndexIterator::IsInBound() const
{




	//{
	//	ActorObb.Center = Actor->GetActorLocation();
	//	ActorObb.AxisX = Actor->GetActorForwardVector();
	//	ActorObb.AxisY = Actor->GetActorRightVector();
	//	ActorObb.AxisZ = Actor->GetActorUpVector();

	//	FVector ActorOrigin, ActorExtent;
	//	Actor->GetActorBounds(false, ActorOrigin, ActorExtent);
	//	ActorObb.Center = ActorOrigin;

	//	USceneComponent* RootComp = Actor->GetRootComponent();
	//	// 1. 获取组件在“未应用任何世界变换（即局部空间）”下的原始 Bounding Box
	//	// 很多底层组件（如 Mesh, Shape）都会重写这个函数来返回自己最原始的盒体大小
	//	FTransform Transform{ FTransform::Identity };
	//	Transform.SetScale3D(Actor->GetActorScale3D());
	//	FBox LocalBox = RootComp->CalcBounds(Transform).GetBox();


	//	ActorObb.ExtentX = FMath::Abs(LocalBox.GetExtent().X);
	//	ActorObb.ExtentY = FMath::Abs(LocalBox.GetExtent().Y);
	//	ActorObb.ExtentZ = FMath::Abs(LocalBox.GetExtent().Z);
	//}
	//const FOrientedBox ActorObb = IWorldHeightEffectiveActorInterface::Execute_GetBounds(Actor);

	//Actor->GetRootComponent()->GetLocalBounds();
	//FTransform Transform{ FTransform::Identity };
	//Transform.SetScale3D(Actor->GetActorScale3D());

	const FVector Direction{ FVector{X, Y, OrientedBox.Center.Z} - OrientedBox.Center };
	const bool bConditionX{ FMath::Abs(FVector::DotProduct(Direction, OrientedBox.AxisX)) <= OrientedBox.ExtentX };
	const bool bConditionY{ FMath::Abs(FVector::DotProduct(Direction, OrientedBox.AxisY)) <= OrientedBox.ExtentY };
	const bool bConditionZ{ FMath::Abs(FVector::DotProduct(Direction, OrientedBox.AxisZ)) <= OrientedBox.ExtentZ };

	//if (GetNameSafe(Actor).StartsWith("BP_DestructibleDoor"))
	//{
	//	UE_LOG(LogTemp, Error, TEXT("BP_DestructibleDoor bConditionX %d"), bConditionX);
	//	UE_LOG(LogTemp, Error, TEXT("BP_DestructibleDoor bConditionY %d"), bConditionY);
	//	UE_LOG(LogTemp, Error, TEXT("BP_DestructibleDoor bConditionZ %d"), bConditionZ);
	//	UE_LOG(LogTemp, Error, TEXT("BP_DestructibleDoor X %f"), X);
	//	UE_LOG(LogTemp, Error, TEXT("BP_DestructibleDoor Y %f"), Y);
	//	UE_LOG(LogTemp, Error, TEXT("BP_DestructibleDoor ActorObb.Center %s"), *ActorObb.Center.ToString());
	//	UE_LOG(LogTemp, Error, TEXT("BP_DestructibleDoor ActorObb.AxisX %s"), *ActorObb.AxisX.ToString());
	//	UE_LOG(LogTemp, Error, TEXT("BP_DestructibleDoor ActorObb.AxisY %s"), *ActorObb.AxisY.ToString());
	//	UE_LOG(LogTemp, Error, TEXT("BP_DestructibleDoor ActorObb.AxisZ %s"), *ActorObb.AxisZ.ToString());
	//	UE_LOG(LogTemp, Error, TEXT("BP_DestructibleDoor ActorObb.ExtentX %f"), ActorObb.ExtentX);
	//	UE_LOG(LogTemp, Error, TEXT("BP_DestructibleDoor ActorObb.ExtentY %f"), ActorObb.ExtentY);
	//	UE_LOG(LogTemp, Error, TEXT("BP_DestructibleDoor ActorObb.ExtentZ %f"), ActorObb.ExtentZ);
	//}

	return bConditionX && bConditionY && bConditionZ;
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
	const FBox AABBBox{ UFogOfWarComponentStatics::GetOrientedBoxAABB(OrientedBox) };

	const FBox OldAABBBox{ UFogOfWarComponentStatics::GetOrientedBoxAABB(Data->OrientedBox) };

	if (bMapElementAdded && bMapElementRemoved && Data != nullptr && OldAABBBox.Intersect(AABBBox))
	{
		TSet<FogOfWarTypes::GridIndexType> OverlapArea;
		for (FGridIndexIterator It{ AABBBox , &WorldHeightSubsystem ,OrientedBox }; It; ++It)
		{
			OverlapArea.Add(*It);
		}

		for (FGridIndexIterator It{ OldAABBBox , &WorldHeightSubsystem, Data->OrientedBox }; It; ++It)
		{
			if (OverlapArea.Find(*It) == nullptr && WorldHeightMap.Find(*It) == nullptr)
			{
				OnWorldHeightEffectiveNumMapElementNumChangeDelegate.Broadcast(EChangeType::Removed);
				break;
			}
		}

		bool bNotifyAdded{ false };
		for (FGridIndexIterator It{ AABBBox , &WorldHeightSubsystem, OrientedBox }; It; ++It)
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
	UFogOfWarComponentStatics::GetOrientedBoxAABB(OrientedBox);

	for (FGridIndexIterator It{ UFogOfWarComponentStatics::GetOrientedBoxAABB(OrientedBox), &WorldHeightSubsystem, OrientedBox }; It; ++It)
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

		for (FGridIndexIterator It{ UFogOfWarComponentStatics::GetOrientedBoxAABB(Data->OrientedBox) , &WorldHeightSubsystem, Data->OrientedBox }; It; ++It)
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

	for (FGridIndexIterator It{ UFogOfWarComponentStatics::GetOrientedBoxAABB(Data.OrientedBox) , &WorldHeightSubsystem, Data.OrientedBox }; It; ++It)
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