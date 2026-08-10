#include "WorldHeightSubsystem.h"
#include "WorldHeightVolume.h"
#include "GameFramework/Actor.h"
#include "Landscape.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"
#include "DrawDebugHelpers.h"
#include "Components/ActorComponent.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "WorldHeightEffectiveActorInterface.h"

UWorldHeightSubsystem::UWorldHeightSubsystem() : UWorldSubsystem()
{
	UE_LOG(LogTemp, Warning, TEXT("UWorldHeightSubsystem::UWorldHeightSubsystem"));
}

void UWorldHeightSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	//FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UWorldHeightSubsystem::OnPostLoadMapWithWorld);
	//FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UWorldHeightSubsystem::OnPostWorldInitialization);
	//FWorldDelegates::OnWorldInitializedActors.AddUObject(this, &UWorldHeightSubsystem::OnWorldInitializedActors);


#if WITH_EDITOR
	//if (GIsEditor)
	//{
	//	GEngine->OnActorMoved().AddUObject(this, &UNavigationSystemV1::OnActorMoved);
	//}
#endif
	InitializeDelegates();

	UE_LOG(LogTemp, Warning, TEXT("UWorldHeightSubsystem::Initialize"));
}

void UWorldHeightSubsystem::Deinitialize()
{
	Super::Deinitialize();

	DeinitializeDelegates();

	UE_LOG(LogTemp, Warning, TEXT("UWorldHeightSubsystem::Deinitialize"));
}

void UWorldHeightSubsystem::RequestUpdateWorldHeightData(const AWorldHeightVolume& Volume)
{
	if (!Volume.IsRootComponentStatic() || !IsWorldHeightVolumeOverlapWithGround(Volume)) { return; }

	RequestUpdateWorldHeightData(*Cast<const AActor>(&Volume), FWorldHeightBoundsUpdateRequest::Type::Added);
}

void UWorldHeightSubsystem::RequestUpdateWorldHeightData(const AActor& OtherActor, FWorldHeightBoundsUpdateRequest::Type RequestType)
{
	if (!Land.IsValid() || (!CanAddActor(OtherActor) && !OtherActor.IsA<AWorldHeightVolume>())/*!OtherActor.IsRootComponentStatic()*/) { return; }

	FGridBoundsDataType Bounds = UFogOfWarComponentStatics::MakeGridBoundsTypeFromActor(OtherActor);

	PendingWorldHeightBoundsUpdates.Add(FWorldHeightBoundsUpdateRequest{ OtherActor.GetUniqueID() , Bounds.Box, RequestType, &OtherActor });

	if (RequestType == FWorldHeightBoundsUpdateRequest::Type::Removed)
	{
		if (TimerHandle.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		}

		UpdateWorldHeightData();
	}
	else if (!TimerHandle.IsValid())
	{
		TimerHandle = GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UWorldHeightSubsystem::UpdateWorldHeightData);
	}

	//RequestUpdateWorldHeightData(Bounds);
	//UpdateWorldHeightData(OtherActor);
}

//GridIndexType UWorldHeightSubsystem::GetGridIndex(const FVector2D& Location2D) const
//{
//	return GridIndexType();
//}

bool UWorldHeightSubsystem::GetGridSize(FVector2D& GridSize) const
{
	if (GridNumX < 0 || GridNumY < 0 || !LandBounds.IsSet()) { return false; }

	const FBox& LandBox = LandBounds.GetValue();
	GridSize = { (LandBox.Max.X - LandBox.Min.X) / GridNumX , (LandBox.Max.Y - LandBox.Min.Y) / GridNumY };
	return true;
}

//FogOfWarTypes::GridIndexType UWorldHeightSubsystem::GetGridIndex(const FVector2D& Location2D) const
//{
//	const FGridBoundsDataType& GridBoundsData = LandBounds.LandBounds;
//
//	if (!GridBoundsData.IsValid() || !GridBoundsData.Box.IsInsideOrOnXY(FVector{ Location2D.X, Location2D.Y, 0 })) { return INDEX_NONE; }
//
//	const FBox& LandBox{ GridBoundsData.Box };
//
//	//const double OffsetX{ FMath::Abs(Location2D.X - LandBox.Min.X) / (LandBox.GetExtent().X * 2) };
//	//const double OffsetY{ FMath::Abs(Location2D.Y - LandBox.Min.Y) / (LandBox.GetExtent().Y * 2) };
//
//	double X{ FMath::Abs((Location2D.X - LandBox.Min.X) / (LandBox.GetExtent().X * 2)) };
//	double Y{ FMath::Abs((Location2D.Y - LandBox.Min.Y) / (LandBox.GetExtent().Y * 2)) };
//
//	//return FMath::Min(FMath::Floor(Y * GridNumY), GridNumY - 1) * GridNumX + FMath::Min(X * GridNumX, GridNumX - 1);
//	return FMath::Floor(Y * GridNumY) * GridNumX + X * GridNumX;
//}

FogOfWarTypes::GridIndexType UWorldHeightSubsystem::GetGridIndex(const FVector2D& Location2D) const
{
	if (!LandBounds.IsSet() || !LandBounds.GetValue().IsInsideOrOnXY(FVector{ Location2D.X, Location2D.Y, 0 }))
	{
		return INDEX_NONE;
	}

	const FBox& LandBox{ LandBounds.GetValue() };

	// 1. 获得 0.0 ~ 1.0 的比例
	double RatioX = (Location2D.X - LandBox.Min.X) / (LandBox.GetExtent().X * 2.0);
	double RatioY = (Location2D.Y - LandBox.Min.Y) / (LandBox.GetExtent().Y * 2.0);

	// 2. 转换为离散的整数行列号，并严格防止越界
	//int32 GridX = FMath::Min(FMath::FloorToInt(RatioX * GridNumX), GridNumX - 1); // 列 (Column)
	//int32 GridY = FMath::Min(FMath::FloorToInt(RatioY * GridNumY), GridNumY - 1); // 行 (Row)
	int32 GridX = FMath::Min(FMath::FloorToInt(RatioX * GridNumX), GridNumX); // 列 (Column)
	int32 GridY = FMath::Min(FMath::FloorToInt(RatioY * GridNumY), GridNumY); // 行 (Row)

	// 3. 标准一维化公式：Row * Width + Column
	return GridY * GridNumX + GridX;
}

FogOfWarTypes::GridIndexType UWorldHeightSubsystem::GetGridIndex(const FVector& Location) const
{
	return GetGridIndex(FVector2D{ Location.X, Location.Y });
}

bool UWorldHeightSubsystem::GetGridLocationByIndex(FVector& Location, const FogOfWarTypes::GridIndexType Index) const
{
	if (Index < 0 || Index >= (GridNumX * GridNumY) || !LandBounds.IsSet())
	{
		return false;
	}

	// 修正：Index / Width 得到的是 Row (Y)，Index % Width 得到的是 Column (X)
	const int32 GridY = static_cast<int32>(Index / GridNumX);
	const int32 GridX = static_cast<int32>(Index % GridNumX);

	const FBox& LandBox{ LandBounds.GetValue() };

	FVector2D GridSize;
	GetGridSize(GridSize);

	// 使用正确的 GridX 和 GridY 计算世界坐标中心点 (+0.5f)
	Location.X = LandBox.Min.X + (GridX + 0.5f) * GridSize.X;
	Location.Y = LandBox.Min.Y + (GridY + 0.5f) * GridSize.Y;
	Location.Z = LandBounds.GetValue().GetSize().Z;

	return true;
}

bool UWorldHeightSubsystem::IsWorldHeightVolumeOverlapWithGround(const AWorldHeightVolume& Volume) const
{
	return LandBounds.IsSet() && LandBounds.GetValue().Overlap(Volume.GetComponentsBoundingBox(true)).GetSize().IsNearlyZero() == false;
}

void UWorldHeightSubsystem::OnWorlHeightVolumeRegisteredComponents(AWorldHeightVolume& Volume)
{
	if (!WorldHeightVolumes.Contains(&Volume))
	{
		RequestUpdateWorldHeightData(Volume);
		WorldHeightVolumes.AddUnique(&Volume);
	}
}

void UWorldHeightSubsystem::OnWorlHeightVolumeUnregisteredComponents(AWorldHeightVolume& Volume)
{
	CleanInvalidData_Internal();
}

FWorldHeightData::FWorldHeightMapType& UWorldHeightSubsystem::GetWorldHeightMap(const UWorldHeightSubsystem* self)
{
	return const_cast<FWorldHeightData::FWorldHeightMapType&>(const_cast<const UWorldHeightSubsystem*>(this)->GetWorldHeightMap());
}

bool UWorldHeightSubsystem::CanAddActor(const AActor& Actor)
{
	return Actor.Implements<UWorldHeightEffectiveActorInterface>();
}

#if WITH_EDITOR
void UWorldHeightSubsystem::DrawVisualization() const
{
	//if (true) { return; }

	UWorld* World = GetWorld();
	FlushPersistentDebugLines(World);
	if (!LandBounds.IsSet()) { return; }

	FVector2D GridSize;
	GetGridSize(GridSize);

	int32 ValidData{ 0 };
	TArray<TWeakObjectPtr<const AActor>> EffectiveActor;
	WorldHeightData.WorldHeightEffectiveActor.GetKeys(EffectiveActor);
	//{
	//	FString Name;
	//	for (auto A : EffectiveActor)
	//	{
	//		if (Name.Len() > 0)
	//		{
	//			Name += ", ";
	//		}
	//		Name += GetNameSafe(A);
	//	}

	//	UE_LOG(LogTemp, Warning, TEXT("WorldHeightEffectiveActor: %s"), *Name);
	//}

	for (const TWeakObjectPtr<const AActor>& ActorWeakPtr : EffectiveActor)
	{
		const AActor* Actor = ActorWeakPtr.Get();
		if (!Actor)
		{
			using NonConstThisTypePtr = std::remove_const_t<std::remove_pointer_t<decltype(this)>>*;
			const_cast<NonConstThisTypePtr>(this)->NotifyCleanInvalidData_Internal();

			continue;
		}

		ValidData++;

		//FVector Origin, BoxExtent;
		//Actor->GetActorBounds(false, Origin, BoxExtent);
		FOrientedBox Box{ IWorldHeightEffectiveActorInterface::Execute_GetBounds(Actor) };

		const double DistanceToLand{ Box.Center.Z - LandBounds.GetValue().Max.Z };
		Box.Center.Z -= DistanceToLand > 0 ? DistanceToLand : DistanceToLand - (LandBounds.GetValue().GetExtent().Z * 2);

		// 1. 提取归一化后的三个局部轴向
		FVector AxisX = Box.AxisX.GetSafeNormal();
		FVector AxisY = Box.AxisY.GetSafeNormal();
		FVector AxisZ = Box.AxisZ.GetSafeNormal();

		// 2. 构造旋转矩阵与四元数
		FMatrix RotationMatrix;
		RotationMatrix.SetAxes(&AxisX, &AxisY, &AxisZ);
		FQuat BoxQuat = RotationMatrix.ToQuat();

		// 3. 直接使用 FOrientedBox 自带的局部空间 Extent（半长）
		FVector LocalExtent{ (float)Box.ExtentX, (float)Box.ExtentY, (float)Box.ExtentZ };

		// 4. 绘制 Debug Box
		DrawDebugSolidBox(World, Box.Center, LocalExtent, BoxQuat, FColor::Green, true);
	}

	UE_LOG(LogTemp, Warning, TEXT("Valid Data: %d"), ValidData);
}

void UWorldHeightSubsystem::DrawVisualization(const FogOfWarTypes::GridIndexType Index) const
{
	if (true) { return; }

	if (!LandBounds.IsSet()) { return; }
	const FBox& LandBox = LandBounds.GetValue();
	const FVector2D GridSize{ (LandBox.Max.X - LandBox.Min.X) / GridNumX , (LandBox.Max.Y - LandBox.Min.Y) / GridNumY };
	FVector BoxLocation;

	if (!GetGridLocationByIndex(BoxLocation, Index)) { return; }

	const FVector BoxCenter{ BoxLocation + BoxDefaultHeight / 2.f * FVector::UpVector };


	UE_LOG(LogTemp, Warning, TEXT("DrawDebugSolidBox, Center: %s\tExtent: %s"), *BoxCenter.ToString(), *FVector{ GridSize.X * BoxDefaultSize, GridSize.Y * BoxDefaultSize, BoxDefaultHeight * .5f }.ToString());
	DrawDebugSolidBox(GetWorld(), BoxCenter, FVector{ GridSize.X * BoxDefaultSize, GridSize.Y * BoxDefaultSize, BoxDefaultHeight * .5f }, FColor::Green, true);
	//DrawDebugSolidPlane
}
#endif

void UWorldHeightSubsystem::HandleWorldHeightVolumeInUpdateRequest()
{
	TArray<const AWorldHeightVolume*> WorldHeightVolumeRequest;

	for (const FWorldHeightBoundsUpdateRequest& Request : PendingWorldHeightBoundsUpdates)
	{
		if (const AWorldHeightVolume* Volume = Cast<AWorldHeightVolume>(Request.Agent.Get()))
		{
			WorldHeightVolumeRequest.Add(Volume);
		}
	}

	for (const AWorldHeightVolume* Volume : WorldHeightVolumeRequest)
	{
		UpdateWorldHeightData_Internal(*Volume);
	}
}

void UWorldHeightSubsystem::UpdateWorldHeightData()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);

	HandleWorldHeightVolumeInUpdateRequest();
	PendingWorldHeightBoundsUpdates.RemoveAllSwap([](const FWorldHeightBoundsUpdateRequest& Request) {return Cast<AWorldHeightVolume>(Request.Agent.Get()); });

	bool bWorldHeightUpdated{ false };
	for (const FWorldHeightBoundsUpdateRequest& Request : PendingWorldHeightBoundsUpdates)
	{
		if (Request.Agent.Get())
		{
			bWorldHeightUpdated = true;
			UpdateWorldHeightData_Internal(*Request.Agent.Get(), Request.UpdateRequest);
		}
	}
	if (bWorldHeightUpdated) { WorldHeightDataVersion++; }

	PendingWorldHeightBoundsUpdates.Empty();

#if WITH_EDITOR
	DrawVisualization();
#endif
}

void UWorldHeightSubsystem::UpdateWorldHeightData_Internal(const AWorldHeightVolume& Volume)
{
	WorldHeightVolumes.AddUnique(&Volume);

	FCollisionQueryParams QueryParams;
	QueryParams.TraceTag = FName("BoxTraceMultiForObjectsForUpdateWorldHeight");
	QueryParams.bTraceComplex = false;
	QueryParams.bReturnPhysicalMaterial = false;
	QueryParams.AddIgnoredActors(TArray<TWeakObjectPtr<const AActor>>{WorldHeightVolumes});

	TArray<FOverlapResult> Result;
	FBox VolumeBox = Volume.GetComponentsBoundingBox(true);

	//if (GetWorld()->OverlapMultiByObjectType(Result, VolumeBox.GetCenter(), FQuat{ FRotator::ZeroRotator }, FCollisionObjectQueryParams::AllStaticObjects, FCollisionShape::MakeBox(VolumeBox.GetExtent()), QueryParams))
	//{
	//	for (const FOverlapResult& R : Result)
	//	{
	//		AActor* OverlapActor = R.GetActor();

	//		if (OverlapActor->IsRootComponentStatic() && WorldHeightData.FindWorldHeightEffectiveActor(*OverlapActor))
	//		{
	//			uint32 UniqueID{ OverlapActor->GetUniqueID() };
	//			FBox Box{ OverlapActor->GetComponentsBoundingBox(true) };

	//			PendingWorldHeightBoundsUpdates.Add(FWorldHeightBoundsUpdateRequest{ UniqueID, Box, FWorldHeightBoundsUpdateRequest::Type::Added, OverlapActor });
	//		}
	//	}
	//}

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
	UClass* ActorClassFilter = AStaticMeshActor::StaticClass();
	TArray<AActor*> ActorToIgnore;
	//ActorToIgnore.Append(WorldHeightVolumes);
	for (const TWeakObjectPtr<const AWorldHeightVolume>& VolumeWeakPtr : WorldHeightVolumes)
	{
		if (const AWorldHeightVolume* V = VolumeWeakPtr.Get())
		{
			ActorToIgnore.Add(const_cast<AWorldHeightVolume*>(V));
		}
		else
		{
			NotifyCleanInvalidData_Internal();
		}
	}

	TArray<AActor*> OverlappingActors;

	if (UKismetSystemLibrary::BoxOverlapActors(GetWorld(), VolumeBox.GetCenter(), VolumeBox.GetExtent(), ObjectTypes, /*ActorClassFilter*/nullptr, ActorToIgnore, OverlappingActors))
	{
		for (const AActor* OverlapActor : OverlappingActors)
		{
			if (CanAddActor(*OverlapActor) /*&& OverlapActor->IsRootComponentStatic()*/ /*&& WorldHeightData.FindWorldHeightEffectiveActor(*OverlapActor)*/)
			{
				uint32 UniqueID{ OverlapActor->GetUniqueID() };
				//FBox Box{ OverlapActor->GetComponentsBoundingBox(true) };
				//FVector ActorOrigin, ActorExtent;
				//OverlapActor->GetActorBounds(false, ActorOrigin, ActorExtent);

				FBox Box;
				{
					FOrientedBox Obb = IWorldHeightEffectiveActorInterface::Execute_GetBounds(OverlapActor);
					Box.Min = { Obb.Center.X - Obb.ExtentX, Obb.Center.Y - Obb.ExtentY ,Obb.Center.Z - Obb.ExtentZ };
					Box.Max = { Obb.Center.X + Obb.ExtentX, Obb.Center.Y + Obb.ExtentY ,Obb.Center.Z + Obb.ExtentZ };
				}

				PendingWorldHeightBoundsUpdates.Add(FWorldHeightBoundsUpdateRequest{ UniqueID, Box, FWorldHeightBoundsUpdateRequest::Type::Added, OverlapActor });
			}
		}
	}
}

void UWorldHeightSubsystem::UpdateWorldHeightData_Internal(const AActor& Actor, FWorldHeightBoundsUpdateRequest::Type RequestType)
{
	if (&Actor == Land.Get()) { return; }

	//UE_LOG(LogTemp, Warning, TEXT("UpdateWorldHeightData_Internal: %s\t%d"), *GetNameSafe(&Actor), RequestType);

	if (CanAddActor(Actor)/*Actor.IsRootComponentStatic()*/)
	{
		UE_LOG(LogTemp, Warning, TEXT("UpdateWorldHeightData_Internal: %s\t%d"), *GetNameSafe(&Actor), RequestType);

		if (RequestType == FWorldHeightBoundsUpdateRequest::Type::Added)
		{
			WorldHeightData.AddHeightEffectiveActor(Actor, *this);
		}
		else if (RequestType == FWorldHeightBoundsUpdateRequest::Type::Removed)
		{
			WorldHeightData.RemoveHeightEffectiveActor(Actor, *this);
		}
	}
}

void UWorldHeightSubsystem::NotifyCleanInvalidData_Internal()
{
	if (!CleanInvalidDataTimerHandle.IsValid())
	{
		CleanInvalidDataTimerHandle = GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UWorldHeightSubsystem::CleanInvalidData_Internal);
	}
}

void UWorldHeightSubsystem::CleanInvalidData_Internal()
{
	CleanInvalidDataTimerHandle.Invalidate();

	WorldHeightVolumes.RemoveAll([](TWeakObjectPtr<const AWorldHeightVolume>& Ptr) {
		return !Ptr.IsValid();
		});

	WorldHeightData.NotifyCleanInvalidWorldHeightEffectiveActorData(*this);
}

void UWorldHeightSubsystem::InitializeDelegates()
{
#if WITH_EDITOR
	if (GIsEditor)
	{
		GEngine->OnActorMoved().AddUObject(this, &UWorldHeightSubsystem::OnActorMoved);
	}

	//GEngine->OnLevelActorAdded().
	GEngine->OnLevelActorAdded().AddUObject(this, &UWorldHeightSubsystem::OnActorRegistered);
	GEngine->OnLevelActorDeleted().AddUObject(this, &UWorldHeightSubsystem::OnActorUnregistered);
#endif
	if (!OnActorDestroyedDelegateHandle.IsValid())
	{
		TDelegate<void(AActor*)> OnActorDestroyedDelegate;
		OnActorDestroyedDelegate.BindUObject(this, &UWorldHeightSubsystem::OnActorDestroyed);
		OnActorDestroyedDelegateHandle = GetWorld()->AddOnActorDestroyedHandler(OnActorDestroyedDelegate);
	}
	PostLandActorRegisteredComponentsDelegateHandle = GetWorld()->AddOnPostRegisterAllActorComponentsHandler(
		FOnPostRegisterAllActorComponents::FDelegate::CreateUObject(this, &UWorldHeightSubsystem::OnActorRegisteredComponents
		));
}

void UWorldHeightSubsystem::DeinitializeDelegates()
{
#if WITH_EDITOR
	if (GIsEditor)
	{
		GEngine->OnActorMoved().RemoveAll(this);
	}

	GEngine->OnLevelActorAdded().RemoveAll(this);
	GEngine->OnLevelActorDeleted().RemoveAll(this);
#endif
	PostLandActorRegisteredComponentsDelegateHandle.Reset();

	if (UWorld* World = GetWorld())
	{
		if (OnActorDestroyedDelegateHandle.IsValid())
		{
			World->RemoveOnActorDestroyedHandler(OnActorDestroyedDelegateHandle);
			OnActorDestroyedDelegateHandle.Reset();
		}

		if (PostLandActorRegisteredComponentsDelegateHandle.IsValid())
		{
			World->RemoveOnPostRegisterAllActorComponentsHandler(PostLandActorRegisteredComponentsDelegateHandle);
			PostLandActorRegisteredComponentsDelegateHandle.Reset();
		}
	}
}

#if WITH_EDITOR
void UWorldHeightSubsystem::OnActorMoved(AActor* Actor)
{
	if (!Actor) { return; }

	UE_LOG(LogTemp, Error, TEXT("UWorldHeightSubsystem::OnActorMoved: Land.IsValid(): %d"), Land.IsValid());

	RequestUpdateWorldHeightData(*Actor, FWorldHeightBoundsUpdateRequest::Type::Added);
	//UE_LOG(LogTemp, Warning, TEXT("UWorldHeightSubsystem::OnActorMoved: %s"), *GetNameSafe(Actor));
}

void UWorldHeightSubsystem::OnActorRegistered(AActor* Actor)
{
	if (!Actor) { return; }

	RequestUpdateWorldHeightData(*Actor, FWorldHeightBoundsUpdateRequest::Type::Added);

	UE_LOG(LogTemp, Warning, TEXT("UWorldHeightSubsystem::OnActorRegistered: %s"), *GetNameSafe(Actor));
}

void UWorldHeightSubsystem::OnActorUnregistered(AActor* Actor)
{
	if (!Actor) { return; }

	RequestUpdateWorldHeightData(*Actor, FWorldHeightBoundsUpdateRequest::Type::Removed);
	UE_LOG(LogTemp, Warning, TEXT("UWorldHeightSubsystem::OnActorUnregistered: %s"), *GetNameSafe(Actor));
}
#endif

void UWorldHeightSubsystem::OnActorDestroyed(AActor* Actor)
{
	if (!Actor) { return; }

	if (Actor->Implements<UWorldHeightEffectiveActorInterface>()) { RequestUpdateWorldHeightData(*Actor, FWorldHeightBoundsUpdateRequest::Type::Removed); }

	if (Actor == Land.Get()) { LandBounds.Reset(); }
}

void UWorldHeightSubsystem::OnActorRegisteredComponents(AActor* Actor)
{
	if (Actor->Implements<UWorldLandInterface>())
	{
		OnLandRegisteredComponents(*Actor);
	}
	else if (AWorldHeightVolume* Volume = Cast<AWorldHeightVolume>(Actor))
	{
		OnWorlHeightVolumeRegisteredComponents(*Volume);
	}
}

void UWorldHeightSubsystem::OnLandRegisteredComponents(AActor& InLand)
{
	Land = &InLand;
	Land->OnDestroyed.AddUniqueDynamic(this, &UWorldHeightSubsystem::OnActorDestroyed);
	LandBounds = FGridBounds{ Land->GetComponentsBoundingBox(true) };

	TArray<AActor*> Volumes;
	UGameplayStatics::GetAllActorsOfClass(this, AWorldHeightVolume::StaticClass(), Volumes);

	for (AActor* Volume : Volumes)
	{
		if (Volume->IsActorInitialized() && !WorldHeightVolumes.Contains(Volume))
		{
			if (AWorldHeightVolume* WorldHeightVolume = Cast<AWorldHeightVolume>(Volume))
			{
				WorldHeightVolumes.AddUnique(WorldHeightVolume);
			}
		}
	}

	for (const TWeakObjectPtr<const AWorldHeightVolume>& VolumeWeakPtr : WorldHeightVolumes)
	{
		if (const AWorldHeightVolume* Volume = VolumeWeakPtr.Get())
		{
			RequestUpdateWorldHeightData(*Volume);
		}
		else
		{
			NotifyCleanInvalidData_Internal();
		}
	}
}

void UWorldHeightSubsystem::OnGridSizeUpdated()
{
}

//FGridBounds::FGridBounds(GridSizeType InWidth, GridSizeType InHeight) :Width(InWidth), Height(InHeight)
//{
//}