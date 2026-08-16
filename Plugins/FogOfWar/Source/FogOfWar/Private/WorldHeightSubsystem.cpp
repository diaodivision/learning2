#include "WorldHeightSubsystem.h"
#include "FogOfWarTypes.h"
#include "WorldHeightVolume.h"
#include "GameFramework/Actor.h"
// #include "Landscape.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"
#include "DrawDebugHelpers.h"
#include "Components/ActorComponent.h"
// #include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "WorldHeightEffectiveActorInterface.h"
#include "RenderGraphUtils.h"

void UWorldHeightSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InitializeDelegates();

	CreateWorldHeightTexture();
}

void UWorldHeightSubsystem::Deinitialize()
{
	Super::Deinitialize();

	DeinitializeDelegates();
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
}

TOptional<FGridSizeType> UWorldHeightSubsystem::GetGridSize() const
{
	if (GridNumX < 0 || GridNumY < 0 || !LandBounds.IsSet()) { return NullOpt; }

	const FBox& LandBox = LandBounds.GetValue();
	FVector Result{LandBox.GetSize().X / GridNumY, LandBox.GetSize().Y / GridNumX, 0.};
	Result.Z = FMath::Max(Result.X, Result.Y);
	return FGridSizeType{ Result, FGridSizeType::EGridSizeCoordinate::Screen };
}

FogOfWarTypes::GridIndexType UWorldHeightSubsystem::GetGridIndex(const FVector2D& Location2D) const
{
	if (!LandBounds.IsSet() || !LandBounds.GetValue().IsInsideOrOnXY(FVector{ Location2D.X, Location2D.Y, 0 }))
	{
		return INDEX_NONE;
	}

	const FBox& LandBox{ LandBounds.GetValue() };

	// // 1. 获得 0.0 ~ 1.0 的比例
	// double RatioX = (Location2D.X - LandBox.Min.X) / (LandBox.GetExtent().X * 2.0);
	// double RatioY = (Location2D.Y - LandBox.Min.Y) / (LandBox.GetExtent().Y * 2.0);

	// // 2. 转换为离散的整数行列号，并严格防止越界
	// int32 GridX = FMath::Min(FMath::FloorToInt(RatioX * GridNumX), GridNumX); // 列 (Column)
	// int32 GridY = FMath::Min(FMath::FloorToInt(RatioY * GridNumY), GridNumY); // 行 (Row)

	// // 3. 标准一维化公式：Row * Width + Column
	// return GridY * GridNumX + GridX;

	const FVector2D NormalizedPosition{(Location2D.Y - LandBox.Min.Y) / LandBox.GetSize().Y, (LandBox.Max.X - Location2D.X) / LandBox.GetSize().X};
	if (NormalizedPosition.GetMin() >= 0. && NormalizedPosition.GetMax() <= 1.)
	{
		return FMath::FloorToInt32(NormalizedPosition.X * GridNumX) + FMath::FloorToInt32(NormalizedPosition.Y * GridNumY) * GridNumX;
	}
	return INDEX_NONE;
}

FogOfWarTypes::GridIndexType UWorldHeightSubsystem::GetGridIndex(const FVector& Location) const
{
	if (Location.Z < LandBounds.GetValue().Max.Z) { return INDEX_NONE; }
	
	return GetGridIndex(FVector2D{ Location.X, Location.Y });
}

TOptional<FIntPoint> UWorldHeightSubsystem::IndexToGridPosition(const FogOfWarTypes::GridIndexType Index) const
{
	if (Index < 0 || Index >= (GridNumX * GridNumY)) { return NullOpt; }

	return FIntPoint{ static_cast<int32>(Index % GridNumX), static_cast<int32>(Index / GridNumX) };
}

TOptional<FVector> UWorldHeightSubsystem::GetGridLocationByIndex(const FogOfWarTypes::GridIndexType Index) const
{
	if (const TOptional<FGridSizeType> GridSize{ GetGridSize() }; Index >= 0 && Index < (GridNumX * GridNumY) && Land.IsValid() && GridSize.IsSet())
	{
		const TOptional<FIntPoint> GridPosition{IndexToGridPosition(Index)};
		if (!GridPosition.IsSet()) {return NullOpt;}
		// const int32 GridX = static_cast<int32>(Index % GridNumX);
		// const int32 GridY = static_cast<int32>(Index / GridNumX);

		const FVector GridSizeOnWorld{ GridSize->GetGridSizeOnWorldCoordinate() };
		const FBox& LandBox{ LandBounds.GetValue() };
		return FVector{ LandBox.Max.X - (GridPosition.GetValue().Y + 0.5f) * GridSizeOnWorld.X, LandBox.Min.Y + (GridPosition.GetValue().X + 0.5f) * GridSizeOnWorld.Y, LandBox.Max.Z };
	}

	return NullOpt;
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

bool UWorldHeightSubsystem::CanAddActor(const AActor& Actor)
{
	return Actor.Implements<UWorldHeightEffectiveActorInterface>();
}

const FTextureRHIRef* UWorldHeightSubsystem::GetWorldHeightTextureRef()
{
	return GetWorldHeightTexture() ? &const_cast<UTexture2D*>(GetWorldHeightTexture())->GetResource()->GetTextureRHI() : nullptr;
}

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
	if (&Actor == Land.Get() || !CanAddActor(Actor)) { return; }

	if (RequestType == FWorldHeightBoundsUpdateRequest::Type::Added)
	{
		WorldHeightData.AddHeightEffectiveActor(Actor, *this);
	}
	else if (RequestType == FWorldHeightBoundsUpdateRequest::Type::Removed)
	{
		WorldHeightData.RemoveHeightEffectiveActor(Actor, *this);
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

void UWorldHeightSubsystem::OnActorDestroyed(AActor* Actor)
{
	if (!Actor) { return; }

	if (Actor->Implements<UWorldHeightEffectiveActorInterface>()) { RequestUpdateWorldHeightData(*Actor, FWorldHeightBoundsUpdateRequest::Type::Removed); }

	if (Actor == Land.Get()) { LandBounds.Reset(); }
}

void UWorldHeightSubsystem::UpdateWorldHeightTexture()
{
	if (WorldHeightDataVersion == WorldHeightTextureVersion) { return; }

	// 1. 安全校验
    if (FMath::Min(GridNumX, GridNumY) <= 0 || !WorldHeightTexture || !WorldHeightTexture->GetResource()) { return; }

	#if WITH_EDITOR
	struct FDebugInfo
	{
		FVector Location;
		int32 Index;
		FIntPoint GridPosition;
		bool bIsSet = false;
	};
	#endif

	FDebugInfo MinIndex;
	FDebugInfo MaxIndex;
	FDebugInfo MinX;
	FDebugInfo MinY;
	FDebugInfo MaxX;
	FDebugInfo MaxY;

    // 2. 将数据深拷贝一份交由渲染线程持有，防止主线程销毁/重分配该数组
    TArray<uint8> WorldHeightDataArray;
	WorldHeightDataArray.SetNumZeroed(GridNumX * GridNumY);
	for (auto It = GetWorldHeightMap().CreateConstIterator(); It; ++It)
	{
		const FogOfWarTypes::GridIndexType Index{It->Key};
		if (!ensure(WorldHeightDataArray.IsValidIndex(It->Key))) { continue; }
		
		WorldHeightDataArray[Index] = 1;
		
		const TOptional<FIntPoint> GridPosition{ IndexToGridPosition(Index) };
		if (!GridPosition.IsSet()) {continue;}		
		auto SetDebugInfo = [this, Index, GridPosition](FDebugInfo& Info)
		{
			const TOptional<FVector> Location{GetGridLocationByIndex(Index)};
 			if (!Location.IsSet() || !GridPosition.IsSet()) {return;}		
			
			Info.Location = Location.GetValue();
			Info.GridPosition = GridPosition.GetValue();
			Info.Index = Index;
			Info.bIsSet = true;
		};
		
		if (!MinIndex.bIsSet || MinIndex.Index > Index) {SetDebugInfo(MinIndex);}
		if (!MaxIndex.bIsSet || MaxIndex.Index < Index) {SetDebugInfo(MaxIndex);}
		if (!MinX.bIsSet || MinX.GridPosition.X > GridPosition.GetValue().X) {SetDebugInfo(MinX);}
		if (!MinY.bIsSet || MinY.GridPosition.Y > GridPosition.GetValue().Y) {SetDebugInfo(MinY);}
		if (!MaxX.bIsSet || MaxX.GridPosition.X < GridPosition.GetValue().X) {SetDebugInfo(MaxX);}
		if (!MaxY.bIsSet || MaxY.GridPosition.Y < GridPosition.GetValue().Y) {SetDebugInfo(MaxY);}
		
		// constexpr int32 Step{2};
		// for (auto i{-Step}; i <= Step; i++)
		// {
			// for (auto j{-Step}; j <= Step; j++)
			// {
				// const FogOfWarTypes::GridIndexType NearIndex{ GridPosition.GetValue().X + i + (GridPosition.GetValue().Y + j) * GridNumX };
				// if (WorldHeightDataArray.IsValidIndex(NearIndex)) { WorldHeightDataArray[NearIndex] = 1; }
			// }
		// }
	}

	static int32 Count{1};
	auto PrintDebugInfo = [](const FDebugInfo& Info, const FString& Name)
	{
		UE_LOG(LogTemp, Error, TEXT("Printing Debug Info, Name: %s"), *Name);
		UE_LOG(LogTemp, Error, TEXT("Printing Debug Info, Location: %s"), *Info.Location.ToString());
		UE_LOG(LogTemp, Error, TEXT("Printing Debug Info, Position: %s"), *Info.GridPosition.ToString());
		UE_LOG(LogTemp, Error, TEXT("Printing Debug Info, Index: %d"), Info.Index);
	};
	UE_LOG(LogTemp, Error, TEXT("Printing Debug Info, Count: %d start------------------------"), Count);
	if (MinIndex.bIsSet) {PrintDebugInfo(MinIndex, TEXT("MinIndex"));}
	if (MaxIndex.bIsSet) {PrintDebugInfo(MaxIndex, TEXT("MaxIndex"));}
	if (MinX.bIsSet) {PrintDebugInfo(MinX, TEXT("MinX"));}
	if (MinY.bIsSet) {PrintDebugInfo(MinY, TEXT("MinY"));}
	if (MaxX.bIsSet) {PrintDebugInfo(MaxX, TEXT("MaxX"));}
	if (MaxY.bIsSet) {PrintDebugInfo(MaxY, TEXT("MaxY"));}
	UE_LOG(LogTemp, Error, TEXT("Printing Debug Info, Count: %d end--------------------------"), Count++);

    // 3. 投递渲染命令
    ENQUEUE_RENDER_COMMAND(UpdateTextureCmd)(
        [WeakThis = MakeWeakObjectPtr(this), WorldHeightDataArray = MoveTemp(WorldHeightDataArray), TargetVersion = WorldHeightDataVersion](FRHICommandListImmediate& RHICmdList)
        {
            FTextureResource* Resource = WeakThis.IsValid() ? WeakThis->WorldHeightTexture->GetResource() : nullptr;
            if (!Resource || !Resource->TextureRHI.IsValid()){ return; }

            FUpdateTextureRegion2D Region(0, 0, 0, 0, WeakThis->GridNumX, WeakThis->GridNumY);
            const uint32 Pitch = WeakThis->GridNumX * sizeof(uint8); // Pitch = 单行字节数

            RHICmdList.UpdateTexture2D(
                Resource->TextureRHI,
                0,                  // MipIndex
                Region,             // Region
                Pitch,              // Pitch (行距)
                WorldHeightDataArray.GetData()  // Buffer
            );

			AsyncTask(ENamedThreads::GameThread, [WeakThis, TargetVersion]()
				{
					if (!WeakThis.IsValid() || WeakThis->WorldHeightTextureVersion > TargetVersion) { return; }			
					//OnVisibilityTextureUpdated.Broadcast(OutputTexture);
					WeakThis->WorldHeightTextureVersion = TargetVersion;
				}
			);
        }
    );
}

void UWorldHeightSubsystem::CreateWorldHeightTexture()
{
	WorldHeightTexture = UTexture2D::CreateTransient(GridNumX, GridNumY, FogOfWarConst::PixelFormat);
	WorldHeightTexture->UpdateResource();
}

#if WITH_EDITOR
void UWorldHeightSubsystem::DrawVisualization() const
{
	//if (true) { return; }

	const UWorld* World{ GetWorld() };
	FlushPersistentDebugLines(World);
	if (!LandBounds.IsSet()) { return; }

	int32 ValidData{ 0 };
	TArray<TWeakObjectPtr<const AActor>> EffectiveActor;
	WorldHeightData.WorldHeightEffectiveActor.GetKeys(EffectiveActor);

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
		
		FOrientedBoxAABBAndTransform BoxAABBAndTransform{UFogOfWarComponentStatics::GetOrientedBoxAABBAndTransform(IWorldHeightEffectiveActorInterface::Execute_GetBounds(Actor))};
		const double DistanceToLand{ BoxAABBAndTransform.Box.GetCenter().Z - LandBounds.GetValue().Max.Z };
		const double FixedBoxZ {DistanceToLand > 0 ? DistanceToLand : DistanceToLand - (LandBounds.GetValue().GetExtent().Z * 2)};
		BoxAABBAndTransform.Box.Min.Z = FixedBoxZ - BoxAABBAndTransform.Box.GetExtent().Z;
		BoxAABBAndTransform.Box.Max.Z = FixedBoxZ + BoxAABBAndTransform.Box.GetExtent().Z;

		// 4. 绘制 Debug Box
		DrawDebugSolidBox(World, BoxAABBAndTransform.Box, FColor::Green, BoxAABBAndTransform.Transform, true);
	}

	UE_LOG(LogTemp, Warning, TEXT("Valid Data: %d"), ValidData);
}

void UWorldHeightSubsystem::OnActorMoved(AActor* Actor)
{
	if (!Actor) { return; }

	RequestUpdateWorldHeightData(*Actor, FWorldHeightBoundsUpdateRequest::Type::Added);
}

void UWorldHeightSubsystem::OnActorRegistered(AActor* Actor)
{
	if (!Actor) { return; }

	RequestUpdateWorldHeightData(*Actor, FWorldHeightBoundsUpdateRequest::Type::Added);
}

void UWorldHeightSubsystem::OnActorUnregistered(AActor* Actor)
{
	if (!Actor) { return; }

	RequestUpdateWorldHeightData(*Actor, FWorldHeightBoundsUpdateRequest::Type::Removed);
}
#endif