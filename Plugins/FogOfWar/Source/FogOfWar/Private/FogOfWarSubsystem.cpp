// Fill out your copyright notice in the Description page of Project Settings.


#include "FogOfWarSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "FogOfWarComponent.h"
#include "WorldHeightSubsystem.h"
#include "GameFramework/Character.h"
#include "FogOfWarComponentStatics.h"
#include "RenderGraphUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "FogOfWarShaderTypes.ush"

UFogOfWarSubsystem::UFogOfWarSubsystem() :ULocalPlayerSubsystem()
{
}

void UFogOfWarSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Warning, TEXT("UFogOfWarSubsystem::Initialize"));

	TArray<AActor*> Characters;
	UGameplayStatics::GetAllActorsOfClass(this, ACharacter::StaticClass(), Characters);

	for (AActor* Ch : Characters)
	{
		if (UFogOfWarComponent* Component = Ch->GetComponentByClass<UFogOfWarComponent>())
		{
			OnPostComponentInitialize(Component);
		}
	}
}

void UFogOfWarSubsystem::Deinitialize()
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	ACharacter* Character = PlayerController ? PlayerController->GetCharacter() : nullptr;

	if (GetWorld() && GetWorld()->GetFirstPlayerController() && GetWorld()->GetFirstPlayerController()->GetCharacter())
	{
		GetWorld()->GetFirstPlayerController()->GetCharacter()->OnCharacterMovementUpdated.RemoveAll(this);
	}

	{
		for (TWeakObjectPtr<UFogOfWarComponent>& ComponentPtr : FogOfWarComponents)
		{
			if (USceneComponent * SceneComponent{ ComponentPtr.IsValid() && ComponentPtr->GetOwner() ? ComponentPtr->GetOwner()->GetRootComponent() : nullptr })
			{
				LastComponentOwnerOrCameraTransformMap.Add(SceneComponent);
			}
		}

		for (auto It{ LastComponentOwnerOrCameraTransformMap.CreateIterator() }; It; ++It)
		{
			if (It->Key.IsValid()) { It->Key->TransformUpdated.RemoveAll(this); }
		}
	}

	Super::Deinitialize();

	GEngine->GameViewport->Viewport->ViewportResizedEvent.RemoveAll(this);

	UE_LOG(LogTemp, Warning, TEXT("UFogOfWarSubsystem::Deinitialize"));
}

void UFogOfWarSubsystem::Tick(float DeltaTime)
{
	if (UGameplayStatics::IsGamePaused(this)) { return; }

	if (bHasInvalidComponents)
	{
		bHasInvalidComponents = false;

		FogOfWarComponents.RemoveAll([](TWeakObjectPtr<UFogOfWarComponent>& ComponentPtr) {return !ComponentPtr.IsValid(); });
		for (auto It{ LastComponentOwnerOrCameraTransformMap.CreateIterator() }; It; ++It)
		{
			if (!It->Key.IsValid()) { It.RemoveCurrent(); }
		}
	}

	//if (!IsCameraFOVChanged()) { return; }
	Tick_Internal();
}

void UFogOfWarSubsystem::Tick_Internal()
{
	if (!GEngine || !GEngine->GameViewport || !GEngine->GameViewport->Viewport) { return; }

	if (!bIsInitialScale)
	{
		SetupScaleFactor();

		GEngine->GameViewport->Viewport->ViewportResizedEvent.AddUObject(this, &UFogOfWarSubsystem::OnViewportResized);

		//CreateOutputTexture();
		CreateWorldHeightTexture();
		CreateDynamicTexture();

		bIsInitialScale = true;
	}

	if (bViewportResized)
	{
		// 释放旧的、断开旧的 RHI 链条
		// CachedOutputTexture.SafeRelease();

		// 重新生成正确尺寸的 UTexture2D 资源
		CreateWorldHeightTexture();
		CreateDynamicTexture();

		bViewportResized = false;
	}

	UpdateWorldHeightData();

	TArray<FIntPoint> ActorPositions;
	TArray<FVector2f> ActorVision;
	TArray<int32> RadiusSqList;

	GetFogOfWarActorData(ActorPositions, ActorVision, RadiusSqList);


	const TOptional<FIntPoint> ScreenSize{ GetScreenSize() };
	if (!ScreenSize.IsSet()) { return; }
	UE_LOG(LogTemp, Error, TEXT("ScreenSize: %s"), *ScreenSize.GetValue().ToString());

	FTextureResource* RenderResource = DynamicTexture ? DynamicTexture->GetResource() : nullptr;
	if (!RenderResource || !RenderResource->TextureRHI) { return; }

	const FTextureRHIRef* WorldHeightTextureRHIRefPtr{ UFogOfWarComponentStatics::GetWorldHeightSubsystem(this) ? UFogOfWarComponentStatics::GetWorldHeightSubsystem(this)->GetWorldHeightTextureRef() : nullptr };
	if (!WorldHeightTextureRHIRefPtr) { return; }

	const TOptional<FGridSizeType> ScreenGridSize{UFogOfWarComponentStatics::GetGridSize(EGridType::Screen, this)};
	const TOptional<FGridSizeType> WorldGridSize{UFogOfWarComponentStatics::GetGridSize(EGridType::World, this)};
	if (!ScreenGridSize.IsSet() || !WorldGridSize.IsSet()) { return; }
	const TOptional<FBox2D> CameraBounds{ UFogOfWarComponentStatics::GetCameraFrustumGroundIntersections(this) };
	if (!CameraBounds.IsSet()) { return; }

	const TOptional<FIntPoint> GridPositionOnWorld{UFogOfWarComponentStatics::GetGridPositionOnWorld(FVector2D{ CameraBounds.GetValue().Max.X, CameraBounds.GetValue().Min.Y}, this)};
	if (!GridPositionOnWorld.IsSet()) { return; }

	// FGridTransformContext GridTransformContext;
	// GridTransformContext.ScreenGridOriginInWorldGrid = GridPositionOnWorld.GetValue();
	// const FVector2D ScreenToWorldGridScale{ScreenGridSize.GetValue() / WorldGridSize.GetValue()};
	// GridTransformContext.ScreenToWorldGridScale = FVector2f{static_cast<float>(ScreenToWorldGridScale.X), static_cast<float>(ScreenToWorldGridScale.Y)};
	const FVector2D ScreenToWorldGridScale{ScreenGridSize.GetValue().GetGridSizeOnScreenCoordinate() / WorldGridSize.GetValue().GetGridSizeOnScreenCoordinate()};
	const FVector2f ScreenToWorldGridScaleFVector2f{static_cast<float>(ScreenToWorldGridScale.X), static_cast<float>(ScreenToWorldGridScale.Y)};

	ENQUEUE_RENDER_COMMAND(CalculateVisionArea)(
		[this, ActorPositions, ActorVision, RadiusSqList, RenderResource, WorldHeightTextureRHIRef = *WorldHeightTextureRHIRefPtr, GridPositionOnWorld, ScreenToWorldGridScaleFVector2f](FRHICommandListImmediate& RHICmdList)
		{
			if (!this) { return; }

			FRDGBuilder GraphBuilder(RHICmdList);


			// UpdateWorldHeightDataToTexture(RHICmdList);
			// 创建或重用纹理

			FFogOfWarComputeShader::FParameters* PassParameters = GraphBuilder.AllocParameters<FFogOfWarComputeShader::FParameters>();

			const TOptional<FIntPoint> ScreenSize{ GetScreenSize() };
			if (!ScreenSize.IsSet()) { return; }
			PassParameters->TextureSize.X = ScreenSize.GetValue().X;
			PassParameters->TextureSize.Y = ScreenSize.GetValue().Y;
			//PassParameters->TextureSize = FVector2f(FogOfWarConst::kTextureWidth, FogOfWarConst::kTextureHeight);
			//PassParameters->TextureSize = FVector2f(FogOfWarConst::kScreenWidth, FogOfWarConst::kScreenHeight);			

			UploadFogOfWarActorData(ActorPositions, ActorVision, RadiusSqList, *PassParameters, GraphBuilder);
			// UploadFogOfWarWorldHeightData(*PassParameters, GraphBuilder, TEXT("WorldHeightData"));
			FRDGTextureRef RDGTexture = GraphBuilder.RegisterExternalTexture(
				CreateRenderTarget(WorldHeightTextureRHIRef, TEXT("WorldHeightData"))
			);
			PassParameters->WorldHeightTexture = GraphBuilder.CreateSRV(FRDGTextureSRVDesc::Create(RDGTexture));

			// PassParameters->GridTransformContext = GridTransformContext;
			PassParameters->ScreenGridOriginInWorldGrid = GridPositionOnWorld.GetValue();
			PassParameters->ScreenToWorldGridScale = ScreenToWorldGridScaleFVector2f;

			FRDGTextureRef OutputRDGTexture{nullptr};
			SetComputeShaderOutputTextureCache(OutputRDGTexture, *PassParameters, GraphBuilder, false);
			bViewportResized = false;

			const FIntVector ThreadCount{ ScreenSize.GetValue().X, ScreenSize.GetValue().Y, 1 };
			const FIntVector GroupSize{ FogOfWarConst::kThreadsX , FogOfWarConst::kThreadsY, FogOfWarConst::kThreadsZ };
			const FIntVector DispatchCount = FComputeShaderUtils::GetGroupCount(ThreadCount, GroupSize);

			TShaderMapRef<FFogOfWarComputeShader> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

			FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("CalculateVisionArea"), ComputeShader, PassParameters, DispatchCount);

			// GraphBuilder.QueueTextureExtraction(OutputRDGTexture, &CachedOutputTexture);
			if (RenderResource && RenderResource->TextureRHI.IsValid())
			{
				FRDGTextureRef DestTexture = GraphBuilder.RegisterExternalTexture(
                	CreateRenderTarget(RenderResource->TextureRHI, TEXT("DestTexture"))
            	);

            	AddCopyTexturePass(GraphBuilder, OutputRDGTexture, DestTexture);
			}

			GraphBuilder.Execute();

										// 在渲染命令完成后回调
			AsyncTask(ENamedThreads::GameThread, [this]()
				{
					if (!this || !DynamicTexture || !FogOfWarMaterial) { return; }			
					//OnVisibilityTextureUpdated.Broadcast(OutputTexture);
					SetTextureParameter();
				}
			);
		});
}

void UFogOfWarSubsystem::OnPostComponentInitialize(UFogOfWarComponent* Component)
{
	if (Component)
	{
		FogOfWarComponents.AddUnique(Component);

		AActor* Owner{ Component->GetOwner() };
		USceneComponent* SceneComponent{ ensure(Owner) ? Owner->GetRootComponent() : nullptr };
		if (ensure(SceneComponent))
		{
			SceneComponent->TransformUpdated.AddUObject(this, &UFogOfWarSubsystem::OnFogOfWarComponentOwnerOrCameraTransformUpdated);
			LastComponentOwnerOrCameraTransformMap.Add(SceneComponent);
		}
	}
}

TOptional<FGridSizeType> UFogOfWarSubsystem::GetGridSize() const
{
	const TOptional<FBox2D> CameraBounds{ UFogOfWarComponentStatics::GetCameraFrustumGroundIntersections(this) };
	if (!CameraBounds.IsSet()) { return NullOpt; }

	const FVector2D Result{ CameraBounds.GetValue().GetSize().Y / GetScreenSize().GetValue().X, CameraBounds.GetValue().GetSize().X / GetScreenSize().GetValue().Y };
	return FGridSizeType{Result, FGridSizeType::EGridSizeCoordinate::Screen};
}

bool UFogOfWarSubsystem::IsCameraFOVChanged()
{
	if (!PlayerCameraManager.IsValid()) { SetUpPlayerCameraManager(); }

	if (!PlayerCameraManager.IsValid()) { return false; }

	const float NewFOVAngle{ PlayerCameraManager->GetFOVAngle() };
	bool bNeedToUpdate{ !FMath::IsNearlyEqual(LastFOVAngle, NewFOVAngle) };

	LastFOVAngle = NewFOVAngle;

	return bNeedToUpdate;
}

void UFogOfWarSubsystem::CreateDynamicTexture()
{
	UMaterialInterface* Material = LoadObject<UMaterialInterface>(this, FogOfWarConst::MaterialPath);
	if (!Material)
	{
		UE_LOG(LogTemp, Warning, TEXT("Fail to load UMaterialInterface on %s"), FogOfWarConst::MaterialPath);

		return;
	}

	const TOptional<FIntPoint> ScreenSize{ GetScreenSize() };
	if (!ScreenSize.IsSet()) { return; }

	DynamicTexture = UTexture2D::CreateTransient(ScreenSize.GetValue().X, ScreenSize.GetValue().Y, FogOfWarConst::PixelFormat);
	if (DynamicTexture)
	{
		DynamicTexture->CompressionSettings = TC_EditorIcon;
		DynamicTexture->SRGB = true;
		DynamicTexture->Filter = FogOfWarConst::TextureFilter;

		DynamicTexture->UpdateResource();
	}

	if (FogOfWarMaterial)
	{
		APostProcessVolume* PostProcessVolume = Cast<APostProcessVolume>(UGameplayStatics::GetActorOfClass(this, APostProcessVolume::StaticClass()));
		if (ensureAlwaysMsgf(PostProcessVolume, TEXT("Cannot find PostProvessVolume. %s might not be work correctly."), *GetNameSafe(this)))
		{
			PostProcessVolume->Settings.RemoveBlendable(FogOfWarMaterial);
		}
	}

	FogOfWarMaterial = UMaterialInstanceDynamic::Create(Material, this);
	if (FogOfWarMaterial)
	{
		SetTextureParameter();

		APostProcessVolume* PostProcessVolume = Cast<APostProcessVolume>(UGameplayStatics::GetActorOfClass(this, APostProcessVolume::StaticClass()));
		if (ensureAlwaysMsgf(PostProcessVolume, TEXT("Cannot find PostProvessVolume. %s might not be work correctly."), *GetNameSafe(this)))
		{
			PostProcessVolume->Settings.AddBlendable(FogOfWarMaterial, 1.f);
		}
	}
}

void UFogOfWarSubsystem::SetTextureParameter() const
{
	if (!DynamicTexture) {return;}
	FogOfWarMaterial->SetTextureParameterValue(TEXT("DynamicMaterial"), DynamicTexture);
}

void UFogOfWarSubsystem::CreateWorldHeightTexture()
{
	const TOptional<FIntPoint> ScreenSize{ GetScreenSize() };
	if (!ScreenSize.IsSet()) { return; }

	WorldHeightTexture = UTexture2D::CreateTransient(ScreenSize.GetValue().X, ScreenSize.GetValue().Y, FogOfWarConst::PixelFormat);
	WorldHeightTexture->UpdateResource();
}

//TOptional<FIntPoint> UFogOfWarSubsystem::ProjectWorldToLand(const FVector2D& WorldLocation, const FBox2D& LandBoundingBox) const
//{
//	const TOptional<FIntPoint> ScreenSize{ GetScreenSize() };
//	if (!ScreenSize.IsSet()) { return NullOpt; }
//
//	TArray<FVector> Corners = UFogOfWarComponentStatics::GetCameraFrustumGroundIntersections(this);
//	if (Corners.Num() < 4) { return NullOpt; }
//
//	FBox2D ScreenAABB{ FVector2D{Corners[static_cast<int32>(ECorner::LeftDown)]}, FVector2D{Corners[static_cast<int32>(ECorner::RightTop)] } };
//	ScreenAABB.Min.X = FMath::Max(ScreenAABB.Min.X, LandBoundingBox.Min.X);
//	ScreenAABB.Min.Y = FMath::Max(ScreenAABB.Min.Y, LandBoundingBox.Min.Y);
//	ScreenAABB.Max.X = FMath::Min(ScreenAABB.Max.X, LandBoundingBox.Max.X);
//	ScreenAABB.Max.Y = FMath::Min(ScreenAABB.Max.Y, LandBoundingBox.Max.Y);
//
//	const FVector2D Result{ (WorldLocation - ScreenAABB.Min) * ScreenSize.GetValue() / ScreenAABB.GetSize() };
//	return FIntPoint{ FMath::FloorToInt32(Result.X), FMath::FloorToInt32(Result.Y) };
//}

TOptional<FIntPoint> UFogOfWarSubsystem::ProjectWorldToLand(const FVector2D& WorldLocation, const FBox2D& LandBoundingBox) const
{
	const TOptional<FIntPoint> ScreenSize{ GetScreenSize() };
	if (!ScreenSize.IsSet()) { return NullOpt; }

	const TOptional<FBox2D> ScreenAABB {UFogOfWarComponentStatics::GetCameraFrustumGroundIntersections(this)};
	if (!ScreenAABB.IsSet()) { return NullOpt; }

	// // 1. 正确构建 2D 包围盒（自动计算正确的 Min/Max，避免相机旋转导致 Min/Max 颠倒）
	// FBox2D ScreenAABB{ ForceInit };
	// ScreenAABB += FVector2D{ Corners[static_cast<int32>(ECorner::LeftDown)] };
	// ScreenAABB += FVector2D{ Corners[static_cast<int32>(ECorner::RightTop)] };
	// //FBox2D ScreenAABB{ FVector2D{Corners[static_cast<int32>(ECorner::LeftDown)]},  FVector2D{Corners[static_cast<int32>(ECorner::RightTop)]} };

	// // 2. 与 LandBoundingBox 求交集
	// ScreenAABB.Min.X = FMath::Max(ScreenAABB.Min.X, LandBoundingBox.Min.X);
	// ScreenAABB.Min.Y = FMath::Max(ScreenAABB.Min.Y, LandBoundingBox.Min.Y);
	// ScreenAABB.Max.X = FMath::Min(ScreenAABB.Max.X, LandBoundingBox.Max.X);
	// ScreenAABB.Max.Y = FMath::Min(ScreenAABB.Max.Y, LandBoundingBox.Max.Y);

	// 3. 检查交集包围盒是否有效（防止除以零或负尺寸）
	const FVector2D BoxSize = ScreenAABB.GetValue().GetSize();
	if (BoxSize.X <= 0.0f || BoxSize.Y <= 0.0f) { return NullOpt; }

	// 4. 计算网格坐标
	const FVector2D NormalizedPos = (WorldLocation - ScreenAABB.GetValue().Min) / BoxSize;
	const FIntPoint GridPos{
		FMath::FloorToInt32(NormalizedPos.X * ScreenSize.GetValue().X),
		FMath::FloorToInt32(NormalizedPos.Y * ScreenSize.GetValue().Y)
	};

	//// 5. 校验最终坐标范围，如果不合规则返回 NullOpt（这步最关键！）
	//const FIntPoint MaxSize = ScreenSize.GetValue();
	//if (GridPos.X < 0 || GridPos.Y < 0 || GridPos.X >= MaxSize.X || GridPos.Y >= MaxSize.Y)
	//{
	//	return NullOpt;
	//}

	return GridPos;
}

//bool UFogOfWarSubsystem::ProjectWorldToLand(FIntPoint& Position, const FVector2D& WorldLocation, const FBox2D& LandBoundingBox) const
//{
//	//FVector2f PositionOnLand = WorldLocation - LandLeftDownLocation;
//	//Position.X = FMath::Floor(PositionOnLand.X * FogOfWarConst::kTextureWidth / LandSize.X);
//	//Position.Y = FMath::Floor(PositionOnLand.Y * FogOfWarConst::kTextureHeight / LandSize.Y);
//
//	//return Position.X >= 0 && Position.Y >= 0;
//
//
//	TArray<FVector> Corners = UFogOfWarComponentStatics::GetCameraFrustumGroundIntersections(this);
//	if (Corners.Num() < 4) { return false; }
//
//	FGridBoundsDataType ScreenAABB{ Corners[static_cast<int32>(ECorner::LeftDown)], Corners[static_cast<int32>(ECorner::RightTop)] };
//	ScreenAABB.Box.Min.X = FMath::Max(ScreenAABB.Box.Min.X, LandBoundingBox.Min.X);
//	ScreenAABB.Box.Min.Y = FMath::Max(ScreenAABB.Box.Min.Y, LandBoundingBox.Min.Y);
//	ScreenAABB.Box.Max.X = FMath::Min(ScreenAABB.Box.Max.X, LandBoundingBox.Max.X);
//	ScreenAABB.Box.Max.Y = FMath::Min(ScreenAABB.Box.Max.Y, LandBoundingBox.Max.Y);
//	ScreenAABB.Box.Max.Z = ScreenAABB.Box.Min.Z + 1;
//
//	//UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::SetLandLocationAndSizeParameters LandLeftDownLocation %s"), *LandLeftDownLocation.ToString());
//	//UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::SetLandLocationAndSizeParameters WorldLocation %s"), *WorldLocation.ToString());
//	//UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::SetLandLocationAndSizeParameters ScreenAABB %s"), *ScreenAABB.Box.ToString());
//	//UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::SetLandLocationAndSizeParameters WorldLocation %d"), ScreenAABB.IsInsideXY(WorldLocation));
//	//if (!ScreenAABB.IsInsideXY(WorldLocation)) { return false; }
//
//	const TOptional<FIntPoint> ScreenSize{ GetScreenSize() };
//	if (!ScreenSize.IsSet()) { return false; }
//
//	FVector2D PositionOnLand = WorldLocation - FVector2D{ ScreenAABB.Box.Min };
//	Position.X = FMath::Floor(PositionOnLand.X * ScreenSize.GetValue().X / (ScreenAABB.Box.Max.X - ScreenAABB.Box.Min.X));
//	Position.Y = FMath::Floor(PositionOnLand.Y * ScreenSize.GetValue().Y / (ScreenAABB.Box.Max.Y - ScreenAABB.Box.Min.Y));
//
//	return Position.X >= 0 && Position.Y >= 0;
//}

void UFogOfWarSubsystem::SetupScaleFactor()
{
	const UWorld* World = GetWorld();
	if (!World) { return; }

	const UGameViewportClient* Client = World->GetGameViewport();
	if (!Client) { return; }

	FVector2D ViewportSize;
	Client->GetViewportSize(ViewportSize);

	WidthScaleFactor = 1.f;
	HeightScaleFactor = 1.f;
	if (ViewportSize.X > ViewportSize.Y) { WidthScaleFactor = ViewportSize.X / ViewportSize.Y; }
	else { HeightScaleFactor = ViewportSize.Y / ViewportSize.X; }

	UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::SetupScaleFactor HeightScaleFactor %f"), HeightScaleFactor);
	UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::SetupScaleFactor WidthScaleFactor %f"), WidthScaleFactor);
	UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::SetupScaleFactor ViewportSize %s"), *ViewportSize.ToString());

	bIsInitialScale = true;
	OnViewportSizeChangedDelegate.Broadcast();
}

void UFogOfWarSubsystem::GetFogOfWarActorData(TArray<FIntPoint>& ActorPositions, TArray<FVector2f>& ActorVision, TArray<int32>& RadiusSqList) const
{
	const UWorldHeightSubsystem* WorldHeightSubsystem{GetWorld() ? GetWorld()->GetSubsystem<UWorldHeightSubsystem>() : nullptr};
	if (!WorldHeightSubsystem) { return; }

	const TOptional<FBox2D> ScreenBox{UFogOfWarComponentStatics::GetCameraFrustumGroundIntersections(this)};
	if (!ScreenBox.IsSet()) { return; }

	for (const TWeakObjectPtr<UFogOfWarComponent> WeakComponentPtr : FogOfWarComponents)
	{
		if (!WeakComponentPtr.IsValid())
		{
			bHasInvalidComponents = true;

			continue;
		}

		const UFogOfWarComponent* Component = WeakComponentPtr.Get();

		const TOptional<FFogOfWarData> Data{Component->GetFogOfWarData()};
		if (!Data.IsSet()) { continue; }
		const TOptional<FGridSizeType> GridSize{WorldHeightSubsystem->GetGridSize()};
		const TOptional<FIntPoint> ScreenSize{ GetScreenSize() };
		if (!GridSize.IsSet() || !ScreenSize.IsSet()) { return; }
		// const TOptional<FIntPoint> PositionOnScreen{UFogOfWarComponentStatics::GetGridPositionOnScreen(Data.GetValue().ActorLocation, ScreenSize.GetValue(), ScreenBox.GetValue()) };
		// if (!PositionOnScreen.IsSet()) { continue; }
		// ActorPositions.Add(PositionOnScreen.GetValue());
		
		// const TOptional<FIntPoint> GridPositionOnLand{UFogOfWarComponentStatics::GetGridPosition(Data.GetValue().ActorLocation, this)};
		// if (!GridPositionOnLand.IsSet()) {continue;}
		// ActorPositions.Add(GridPositionOnLand.GetValue());

		const TOptional<FIntPoint> ActorPositionOnScreen{UFogOfWarComponentStatics::GetGridPositionOnScreen(Data.GetValue().ActorLocation, ScreenSize.GetValue(), ScreenBox.GetValue(), EAllowMinusPosition::Yes)};
		if (!ActorPositionOnScreen.IsSet()) {continue;}
		ActorPositions.Add(ActorPositionOnScreen.GetValue());
		ActorVision.Add(FVector2f{UFogOfWarComponentStatics::ProjectWorldDirectionToScreen(Data.GetValue().ActorVisionLeft)});
		ActorVision.Add(FVector2f{UFogOfWarComponentStatics::ProjectWorldDirectionToScreen(Data.GetValue().ActorVisionRight)});
		RadiusSqList.Add(FMath::CeilToInt32(FMath::Pow(Data.GetValue().Radius / GridSize.GetValue().GetGridSizeOnScreenCoordinate().X, 2.f)));
	}
}

void UFogOfWarSubsystem::UpdateWorldHeightData()
{
	const UWorldHeightSubsystem* WorldHeightSubsystem{UFogOfWarComponentStatics::GetWorldHeightSubsystem(this)};
	const TOptional<FIntPoint> ScreenSize{ GetScreenSize() };
	if (!WorldHeightSubsystem || !ScreenSize.IsSet()) { return; }

	if (const int32 Size{ ScreenSize.GetValue().X * ScreenSize.GetValue().Y }; WorldHeightData.Num() != Size) { WorldHeightData.SetNumZeroed(Size); }
	else { FMemory::Memzero(WorldHeightData.GetData(), WorldHeightData.GetAllocatedSize()); }

	CachedWorldHeightDataVersion = WorldHeightSubsystem->GetWorldHeightDataVersion();
	
	
	const TOptional<FBox2D> LandBoundingBox{ WorldHeightSubsystem->GetLandBoundingBox() };
	if (!LandBoundingBox.IsSet()) { return; }
	const TOptional<FBox2D> ScreenAABB{ UFogOfWarComponentStatics::GetCameraFrustumGroundIntersections(this) };
	if (!ScreenAABB.IsSet()) {return;}
	for (auto It = WorldHeightSubsystem->GetWorldHeightMap().CreateConstIterator(); It; ++It)
	{
		if (It->Value <= 0){continue;}
		const TOptional<FVector> Location{WorldHeightSubsystem->GetGridLocationByIndex(It->Key)};
		if (!Location.IsSet()) { continue; }
		
		const FBox2D ScreenBox{FVector2D{ScreenAABB.GetValue().Min}, FVector2D{ScreenAABB.GetValue().Max}};
		const FogOfWarTypes::GridIndexType Index { UFogOfWarComponentStatics::GetGridIndexOnScreen(Location.GetValue(), ScreenSize.GetValue(), ScreenBox)};
		if (!WorldHeightData.IsValidIndex(Index)) {continue;}
		WorldHeightData[Index] = 1;
		
		const TOptional<FIntPoint> CurrentIndex{UFogOfWarComponentStatics::GetGridPositionOnScreen(Location.GetValue(), ScreenSize.GetValue(), ScreenBox)};
		if (!CurrentIndex.IsSet()) { continue; }
		constexpr int32 Step{ 2 };
		for (auto i = -(Step * 3); i <= (Step * 3); i++)
		{
			const auto TargetX{ CurrentIndex.GetValue().X + i };
			for (auto j = -Step; j <= Step; j++)
			{
				const auto TargetY{ CurrentIndex.GetValue().Y + j };
				const FogOfWarTypes::GridIndexType NearIndex{ TargetX + TargetY * ScreenSize.GetValue().X };
				if (WorldHeightData.IsValidIndex(NearIndex))
				{
					WorldHeightData[NearIndex] = 1;
				}
			}
		}
	}
}

void UFogOfWarSubsystem::UpdateWorldHeightDataToTexture(FRHICommandListImmediate& RHICmdList)
{
	const TOptional<FIntPoint> ScreenSize{ GetScreenSize() };
	if (!ScreenSize.IsSet()) { return; }
	FUpdateTextureRegion2D Region(0, 0, 0, 0, ScreenSize.GetValue().X, ScreenSize.GetValue().Y);

	// 2. 使用RHI命令直接更新
	// 这是最底层、最高效的调用方法之一，直接从系统内存更新纹理
	RHICmdList.UpdateTexture2D(
		WorldHeightTexture->GetResource()->GetTextureRHI(),          // RHI纹理资源
		0,                   // Mip索引
		Region,              // 更新区域
		WorldHeightData.GetTypeSize() * ScreenSize.GetValue().X,    // 数据行距（Pitch）
		WorldHeightData.GetData()  // 源数据指针
	);

	//UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::SetLandLocationAndSizeParameters WorldHeightTexture->GetSizeX() %d"), WorldHeightTexture->GetSizeX());
	//UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::SetLandLocationAndSizeParameters WorldHeightTexture->GetSizeY() %d"), WorldHeightTexture->GetSizeY());
	//UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::SetLandLocationAndSizeParameters WorldHeightData.Num() %d"), WorldHeightData.Num());
}
//TArray<FIntPoint>ActorPositions;
//TArray<FVector2f>ActorVision;
void UFogOfWarSubsystem::UploadFogOfWarActorData(const TArray<FIntPoint>& ActorPositions, const TArray<FVector2f>& ActorVision, const TArray<int32>& RadiusSqList, FFogOfWarComputeShader::FParameters& Parameter, FRDGBuilder& GraphBuilder) const
{
	//TArray<FIntPoint>ActorPositions;
	//TArray<FVector2f>ActorVision;

	//GetFogOfWarActorData(ActorPositions, ActorVision);

	UFogOfWarComponentStatics::UploadStructedBuffer(
		Parameter.VisionStartPosBuffer,
		Parameter.NumVisionStart,
		GraphBuilder,
		ActorPositions,
		TEXT("VisionStartPosBuffer")
	);

	UFogOfWarComponentStatics::UploadStructedBuffer(
		Parameter.VisionBuffer,
		Parameter.NumVision,
		GraphBuilder,
		ActorVision,
		TEXT("VisionBuffer")
	);

	UFogOfWarComponentStatics::UploadStructedBuffer(
		Parameter.RadiusSqBuffer,
		Parameter.NumRadiusSq,
		GraphBuilder,
		RadiusSqList,
		TEXT("RadiusSqBuffer")
	);
}

void UFogOfWarSubsystem::UploadFogOfWarWorldHeightData(FFogOfWarComputeShader::FParameters& Parameter, FRDGBuilder& GraphBuilder, const TCHAR* DebugName) const
{
	FRDGTextureRef RDGTexture = GraphBuilder.RegisterExternalTexture(
		CreateRenderTarget(WorldHeightTexture->GetResource()->GetTextureRHI(), DebugName)
	);

	Parameter.WorldHeightTexture = GraphBuilder.CreateSRV(FRDGTextureSRVDesc::Create(RDGTexture));
}

void UFogOfWarSubsystem::SetComputeShaderOutputTextureCache(FRDGTextureRef& ShaderOutputTexture, FFogOfWarComputeShader::FParameters& Parameter, FRDGBuilder& GraphBuilder, const bool bCreateNewOne)
{
	if (!ShaderOutputTexture /*|| !CachedOutputTexture.IsValid() */)
	{
		const TOptional<FIntPoint> ScreenSize{ GetScreenSize() };
		if (!ScreenSize.IsSet()) { return; }

		FRDGTextureDesc TextureDesc = FRDGTextureDesc::Create2D(
			ScreenSize.GetValue(),
			FogOfWarConst::PixelFormat,
			FClearValueBinding::Black,
			TexCreate_ShaderResource | TexCreate_UAV
		);
		ShaderOutputTexture = GraphBuilder.CreateTexture(TextureDesc, TEXT("OutputTexture"));
	}
	// else
	// {
	// 	ShaderOutputTexture = GraphBuilder.RegisterExternalTexture(CachedOutputTexture);
	// }

	Parameter.OutputTexture = GraphBuilder.CreateUAV(ShaderOutputTexture);
}

void UFogOfWarSubsystem::OnViewportResized(FViewport* Viewport, uint32 Unused)
{
	bViewportResized = true;
	SetupScaleFactor();
	//CreateWorldHeightTexture();
	//CreateDynamicTexture();

	UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::OnViewportResized"));
}

void UFogOfWarSubsystem::SetUpPlayerCameraManager()
{
	PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);

	USceneComponent* SceneComponent{ PlayerCameraManager.IsValid() ? PlayerCameraManager->GetRootComponent() : nullptr };
	if (!SceneComponent) { return; }

	LastComponentOwnerOrCameraTransformMap.Add(SceneComponent);
	SceneComponent->TransformUpdated.AddUObject(this, &UFogOfWarSubsystem::OnFogOfWarComponentOwnerOrCameraTransformUpdated);
}

void UFogOfWarSubsystem::OnFogOfWarComponentOwnerOrCameraTransformUpdated(USceneComponent* SceneComponent, EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	const AActor* Owner{ SceneComponent ? SceneComponent->GetOwner() : nullptr };
	if (!Owner) { return; }

	if (FTransform* LastTransform{ LastComponentOwnerOrCameraTransformMap.Find(SceneComponent) }; !ensure(LastTransform))
	{
		Tick_Internal();
		LastComponentOwnerOrCameraTransformMap.Add(SceneComponent);
	}
	else if (const FTransform NewTransform{ Owner->GetActorTransform() }; !LastTransform->Equals(NewTransform))
	{
		Tick_Internal();
		*LastTransform = NewTransform;
	}
}