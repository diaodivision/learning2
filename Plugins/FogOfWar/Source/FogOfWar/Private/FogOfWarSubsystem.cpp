// Fill out your copyright notice in the Description page of Project Settings.


#include "FogOfWarSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "FogOfWarComponent.h"
#include "WorldHeightSubsystem.h"
#include "GameFramework/Character.h"
#include "FogOfWarComponentStatics.h"
#include "RenderGraphUtils.h"
#include "GameFramework/CharacterMovementComponent.h"

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
		CachedOutputTexture.SafeRelease();

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

	if (GetWorld())
	{
		auto* Controller = GetWorld()->GetFirstPlayerController();

		if (Controller)
		{
			auto ch = Controller->GetPawn();

			if (ch)
			{
				//UE_LOG(LogTemp, Error, TEXT("Position 111: %s"), *ch->GetActorLocation().ToString());
				//UE_LOG(LogTemp, Error, TEXT("Position time 111: %f"), GetWorld()->GetTimeSeconds());
			}
		}
	}

	ENQUEUE_RENDER_COMMAND(CalculateVisionArea)(
		[this, ActorPositions, ActorVision, RadiusSqList](FRHICommandListImmediate& RHICmdList)
		{
			if (!this) { return; }

			FRDGBuilder GraphBuilder(RHICmdList);


			UpdateWorldHeightDataToTexture(RHICmdList);
			// 创建或重用纹理

			FFogOfWarComputeShader::FParameters* PassParameters = GraphBuilder.AllocParameters<FFogOfWarComputeShader::FParameters>();

			const TOptional<FIntPoint> ScreenSize{ GetScreenSize() };
			if (!ScreenSize.IsSet()) { return; }
			PassParameters->TextureSize.X = ScreenSize.GetValue().X;
			PassParameters->TextureSize.Y = ScreenSize.GetValue().Y;
			//PassParameters->TextureSize = FVector2f(FogOfWarConst::kTextureWidth, FogOfWarConst::kTextureHeight);
			//PassParameters->TextureSize = FVector2f(FogOfWarConst::kScreenWidth, FogOfWarConst::kScreenHeight);			

			UploadFogOfWarActorData(ActorPositions, ActorVision, RadiusSqList, *PassParameters, GraphBuilder);
			UploadFogOfWarWorldHeightData(*PassParameters, GraphBuilder, TEXT("WorldHeightData"));

			FRDGTextureRef OutputRDGTexture;
			SetComputeShaderOutputTextureCache(OutputRDGTexture, *PassParameters, GraphBuilder, false);
			bViewportResized = false;

			const FIntVector ThreadCount{ ScreenSize.GetValue().X, ScreenSize.GetValue().Y, 1 };
			const FIntVector GroupSize{ FogOfWarConst::kThreadsX , FogOfWarConst::kThreadsY, FogOfWarConst::kThreadsZ };
			const FIntVector DispatchCount = FComputeShaderUtils::GetGroupCount(ThreadCount, GroupSize);

			TShaderMapRef<FFogOfWarComputeShader> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

			FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("CalculateVisionArea"), ComputeShader, PassParameters, DispatchCount);

			GraphBuilder.QueueTextureExtraction(OutputRDGTexture, &CachedOutputTexture);

			GraphBuilder.Execute();

			AsyncTask(ENamedThreads::GameThread, [this]()
				{
					if (!this || !CachedOutputTexture.IsValid() || !DynamicTexture || !FogOfWarMaterial) { return; }

					FTextureResource* RenderResource = DynamicTexture->GetResource();
					if (!RenderResource || !RenderResource->TextureRHI) { return; }

					// 使用拷贝而不是替换 RHI
					ENQUEUE_RENDER_COMMAND(CopyVisibilityTexture)(
						[this, RenderResource](FRHICommandListImmediate& RHICmdList)
						{
							FRDGBuilder GraphBuilder(RHICmdList);

							FRDGTextureRef SourceTexture = GraphBuilder.RegisterExternalTexture(CachedOutputTexture);
							FRDGTextureRef DestTexture = GraphBuilder.RegisterExternalTexture(
								CreateRenderTarget(RenderResource->TextureRHI, TEXT("DestTexture"))
							);

							AddCopyTexturePass(GraphBuilder, SourceTexture, DestTexture);

							GraphBuilder.Execute();

							// 在渲染命令完成后回调
							AsyncTask(ENamedThreads::GameThread, [this]()
								{
									if (!this || !DynamicTexture || !FogOfWarMaterial) { return; }

									//OnVisibilityTextureUpdated.Broadcast(OutputTexture);
									FogOfWarMaterial->SetTextureParameterValue(TEXT("DynamicMaterial"), DynamicTexture);
									SetLandLocationAndSizeParameters();
								}
							);
						});
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
		FogOfWarMaterial->SetTextureParameterValue(TEXT("DynamicMaterial"), DynamicTexture);
		SetLandLocationAndSizeParameters();

		APostProcessVolume* PostProcessVolume = Cast<APostProcessVolume>(UGameplayStatics::GetActorOfClass(this, APostProcessVolume::StaticClass()));
		if (ensureAlwaysMsgf(PostProcessVolume, TEXT("Cannot find PostProvessVolume. %s might not be work correctly."), *GetNameSafe(this)))
		{
			PostProcessVolume->Settings.AddBlendable(FogOfWarMaterial, 1.f);
		}
	}
}

void UFogOfWarSubsystem::SetLandLocationAndSizeParameters() const
{
	if (!GetWorld()) { return; }
	UWorldHeightSubsystem* WorldHeightSubsystem = GetWorld()->GetSubsystem<UWorldHeightSubsystem>();
	if (!WorldHeightSubsystem) { return; }

	const TOptional<FBox2D> LandBoundingBox{ WorldHeightSubsystem->GetLandBoundingBox() };
	if (!LandBoundingBox.IsSet()) { return; }

	{

		TArray<FVector> Corners = UFogOfWarComponentStatics::GetCameraFrustumGroundIntersections(this);

		if (Corners.Num() < 4) { return; }
		FVector4 LeftDownPosition{ Corners[static_cast<int32>(ECorner::LeftDown)] };
		//LeftDownPosition.X = FMath::Max(LeftDownPosition.X, LandBoundingBox.GetValue().Min.X);
		//LeftDownPosition.Y = FMath::Max(LeftDownPosition.Y, LandBoundingBox.GetValue().Min.Y);
		//LeftDownPosition.Z -= 1;
		FVector4 RightTopPosition{ Corners[static_cast<int32>(ECorner::RightTop)] };
		//RightTopPosition.X = FMath::Min(RightTopPosition.X, LandBoundingBox.GetValue().Max.X);
		//RightTopPosition.Y = FMath::Min(RightTopPosition.Y, LandBoundingBox.GetValue().Max.Y);
		//RightTopPosition.Z -= 1;

		FogOfWarMaterial->SetDoubleVectorParameterValue(TEXT("LeftDownPosition"), LeftDownPosition);
		FogOfWarMaterial->SetDoubleVectorParameterValue(TEXT("RightTopPosition"), RightTopPosition);
	}
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

	TArray<FVector> Corners = UFogOfWarComponentStatics::GetCameraFrustumGroundIntersections(this);
	if (Corners.Num() < 4) { return NullOpt; }

	// 1. 正确构建 2D 包围盒（自动计算正确的 Min/Max，避免相机旋转导致 Min/Max 颠倒）
	FBox2D ScreenAABB{ ForceInit };
	ScreenAABB += FVector2D{ Corners[static_cast<int32>(ECorner::LeftDown)] };
	ScreenAABB += FVector2D{ Corners[static_cast<int32>(ECorner::RightTop)] };
	//FBox2D ScreenAABB{ FVector2D{Corners[static_cast<int32>(ECorner::LeftDown)]},  FVector2D{Corners[static_cast<int32>(ECorner::RightTop)]} };

	// 2. 与 LandBoundingBox 求交集
	ScreenAABB.Min.X = FMath::Max(ScreenAABB.Min.X, LandBoundingBox.Min.X);
	ScreenAABB.Min.Y = FMath::Max(ScreenAABB.Min.Y, LandBoundingBox.Min.Y);
	ScreenAABB.Max.X = FMath::Min(ScreenAABB.Max.X, LandBoundingBox.Max.X);
	ScreenAABB.Max.Y = FMath::Min(ScreenAABB.Max.Y, LandBoundingBox.Max.Y);

	// 3. 检查交集包围盒是否有效（防止除以零或负尺寸）
	const FVector2D BoxSize = ScreenAABB.GetSize();
	if (BoxSize.X <= 0.0f || BoxSize.Y <= 0.0f) { return NullOpt; }

	// 4. 计算网格坐标
	const FVector2D NormalizedPos = (WorldLocation - ScreenAABB.Min) / BoxSize;
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

	if (ViewportSize.X > ViewportSize.Y) { HeightScaleFactor = ViewportSize.X / ViewportSize.Y; }
	else { WidthScaleFactor = ViewportSize.Y / ViewportSize.X; }

	UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::SetupScaleFactor HeightScaleFactor %f"), HeightScaleFactor);
	UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::SetupScaleFactor WidthScaleFactor %f"), WidthScaleFactor);
	UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::SetupScaleFactor ViewportSize %s"), *ViewportSize.ToString());

	bIsInitialScale = true;
	OnViewportSizeChangedDelegate.Broadcast();
}

void UFogOfWarSubsystem::GetFogOfWarActorData(TArray<FIntPoint>& ActorPositions, TArray<FVector2f>& ActorVision, TArray<int32>& RadiusSqList) const
{
	if (!GetWorld()) { return; }
	UWorldHeightSubsystem* WorldHeightSubsystem = GetWorld()->GetSubsystem<UWorldHeightSubsystem>();
	if (!WorldHeightSubsystem) { return; }

	const TOptional<FBox2D> LandBoundingBox{ WorldHeightSubsystem->GetLandBoundingBox() };
	if (!LandBoundingBox.IsSet()) { return; }

	for (const TWeakObjectPtr<UFogOfWarComponent> WeakComponentPtr : FogOfWarComponents)
	{
		if (!WeakComponentPtr.IsValid())
		{
			bHasInvalidComponents = true;

			continue;
		}

		const UFogOfWarComponent* Component = WeakComponentPtr.Get();

		FFogOfWarData Data;
		if (Component->GetFogOfWarData(Data))
		{
			const TOptional<FIntPoint> ActorPosition{ ProjectWorldToLand(FVector2D{ Data.ActorLocation }, LandBoundingBox.GetValue()) };
			if (!ActorPosition.IsSet()) { continue; }

			//TOptional<FIntPoint> ActorPosition{ FIntPoint{} };
			//if (!ProjectWorldToLand(ActorPosition.GetValue(), FVector2D{ Data.ActorLocation }, LandBoundingBox.GetValue())) { continue; }

			FVector2D GridSize;
			if (!WorldHeightSubsystem->GetGridSize(GridSize)) { continue; }

			ActorPositions.Add(ActorPosition.GetValue());
			ActorVision.Add(FVector2f{ static_cast<float>(Data.ActorVisionLeft.X), static_cast<float>(Data.ActorVisionLeft.Y) });
			ActorVision.Add(FVector2f{ static_cast<float>(Data.ActorVisionRight.X), static_cast<float>(Data.ActorVisionRight.Y) });
			RadiusSqList.Add(FMath::CeilToInt32(FMath::Pow(Data.Radius, 2.f) / GridSize.X));
		}
	}
}

void UFogOfWarSubsystem::UpdateWorldHeightData()
{
	if (!GetWorld()) { return; }
	UWorldHeightSubsystem* WorldHeightSubsystem = GetWorld()->GetSubsystem<UWorldHeightSubsystem>();
	if (!WorldHeightSubsystem) { return; }

	const TOptional<FIntPoint> ScreenSize{ GetScreenSize() };
	if (!ScreenSize.IsSet()) { return; }
	if (const int32 Size{ ScreenSize.GetValue().X * ScreenSize.GetValue().Y }; WorldHeightData.Num() != Size) { WorldHeightData.SetNumZeroed(Size); }
	else { FMemory::Memzero(WorldHeightData.GetData(), WorldHeightData.GetAllocatedSize()); }

	{
		CachedWorldHeightDataVersion = WorldHeightSubsystem->GetWorldHeightDataVersion();

		//for (auto It = WorldHeightSubsystem->GetWorldHeightMap().CreateConstIterator(); It; ++It)
		//{
		//	WorldHeightData[It->Key] = It->Value > 0 ? 1 : 0;
		//}

		{
			const TOptional<FBox2D> LandBoundingBox{ WorldHeightSubsystem->GetLandBoundingBox() };
			if (!LandBoundingBox.IsSet()) { return; }

			TArray<FVector> Corners = UFogOfWarComponentStatics::GetCameraFrustumGroundIntersections(this);
			if (Corners.Num() < 4) { return; }

			FGridBoundsDataType ScreenAABB{ Corners[static_cast<int32>(ECorner::LeftDown)], Corners[static_cast<int32>(ECorner::RightTop)] };
			//ScreenAABB.Box.Min.X = FMath::Max(ScreenAABB.Box.Min.X, LandBoundingBox.GetValue().Min.X);
			//ScreenAABB.Box.Min.Y = FMath::Max(ScreenAABB.Box.Min.Y, LandBoundingBox.GetValue().Min.Y);
			//ScreenAABB.Box.Max.X = FMath::Min(ScreenAABB.Box.Max.X, LandBoundingBox.GetValue().Max.X);
			//ScreenAABB.Box.Max.Y = FMath::Min(ScreenAABB.Box.Max.Y, LandBoundingBox.GetValue().Max.Y);
			//ScreenAABB.Box.Max.Z = ScreenAABB.Box.Min.Z + 1;

			//// 提取 ScreenAABB 的 2D 尺寸
			//const FVector2D BoxMin{ ScreenAABB.Box.Min.X, ScreenAABB.Box.Min.Y };
			//const FVector2D BoxSize = FVector2D{ ScreenAABB.Box.GetSize() };

			//// 边界异常保护（防止除零）
			//if (BoxSize.X <= 0.0f || BoxSize.Y <= 0.0f)
			//{
			//	return;
			//}

			//const int32 ScreenWidth = ScreenSize.GetValue().X;
			//const int32 ScreenHeight = ScreenSize.GetValue().Y;

			//for (auto It = WorldHeightSubsystem->GetWorldHeightMap().CreateConstIterator(); It; ++It)
			//{
			//	// 1. 【必须】非障碍物直接跳过！不参与膨胀，防止把旁边的 1 误覆写为 0
			//	if (It->Value <= 0)
			//	{
			//		continue;
			//	}

			//	FVector Location;
			//	if (WorldHeightSubsystem->GetGridLocationByIndex(Location, It->Key))
			//	{
			//		// 2. 【核心修复】直接根据世界坐标与 ScreenAABB 计算 2D 归一化比例 (0.0 ~ 1.0)
			//		// 彻底绕过会产生 uint32 溢出或 modulo 卷绕的 GetGridIndexOnScreen 函数
			//		float NormX = (Location.X - BoxMin.X) / BoxSize.X;
			//		float NormY = (Location.Y - BoxMin.Y) / BoxSize.Y;

			//		// 3. 使用 FloorToInt32 正确处理负数坐标（屏幕下方的障碍物 NormY < 0，BaseY 算出来就是 -1, -2 等负数）
			//		int32 BaseX = FMath::FloorToInt32(NormX * ScreenWidth);
			//		int32 BaseY = FMath::FloorToInt32(NormY * ScreenHeight);

			//		constexpr int32 Step{ 10 };

			//		// 4. 【核心修复】屏外粗裁剪：如果障碍物距离屏幕边缘超过膨胀半径 Step，直接跳过！
			//		// 当障碍物在屏幕下方很远时，BaseY 是 -20，-20 < -10 成立，这里会直接 continue 丢弃，绝对不会影响屏幕顶部！
			//		if (BaseX < -Step || BaseX >= ScreenWidth + Step ||
			//			BaseY < -Step || BaseY >= ScreenHeight + Step)
			//		{
			//			continue;
			//		}

			//		// 5. 2D 局部膨胀
			//		for (int32 i = -Step; i <= Step; i++)
			//		{
			//			for (int32 j = -Step; j <= Step; j++)
			//			{
			//				int32 TargetX = BaseX + j;
			//				int32 TargetY = BaseY + i;

			//				// 6. 只有真正落在屏幕 [0, ScreenWidth-1] 和 [0, ScreenHeight-1] 内的像素才会被标记
			//				if (TargetX >= 0 && TargetX < ScreenWidth && TargetY >= 0 && TargetY < ScreenHeight)
			//				{
			//					int32 NearIndex = TargetY * ScreenWidth + TargetX;
			//					WorldHeightData[NearIndex] = 1;
			//				}
			//			}
			//		}
			//	}
			//}
			for (auto It = WorldHeightSubsystem->GetWorldHeightMap().CreateConstIterator(); It; ++It)
			{
				FVector Location;
				WorldHeightSubsystem->GetGridLocationByIndex(Location, It->Key);
				if (FVector2D::Distance(FVector2D{ Location.X, Location.Y }, FVector2D{ 520, -190 }) < 10.f)
				{
					//UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::SetLandLocationAndSizeParameters Location: %s"), *Location.ToString());
				}

				if (WorldHeightSubsystem->GetGridLocationByIndex(Location, It->Key))
				{

					FogOfWarTypes::GridIndexType Index{ ScreenAABB.GetGridIndexOnScreen(Location, ScreenSize.GetValue()) };
					if (FMath::Abs(Location.X - 2010) < 200 && FMath::Abs(Location.Y - 1680) < 200)
					{
						//UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::SetLandLocationAndSizeParameters Index: %d"), Index);
					}
					if (WorldHeightData.IsValidIndex(Index)) { WorldHeightData[Index] = It->Value > 0 ? 1 : 0; }
					else { continue; }

					if ((It->Value > 0) == false) { continue; }

					//const int32 ScreenWidth = ScreenSize.GetValue().X;
					//const int32 ScreenHeight = ScreenSize.GetValue().Y;

					//// 1. 使用 FloorToInt32 正确解构 2D 坐标（防止 negative Index 除法截断）
					//const int32 BaseY = FMath::FloorToInt32(static_cast<float>(Index) / ScreenWidth);
					//const int32 BaseX = Index - BaseY * ScreenWidth;

					//constexpr int32 Step{ 10 };
					//for (int32 i = -Step; i <= Step; i++)
					//{
					//	for (int32 j = -Step; j <= Step; j++)
					//	{
					//		// 2. 正确的曼哈顿距离裁剪（如果你想让膨胀呈现圆形/菱形而非硬方形）
					//		// 如果想要纯方形膨胀，可以直接把这几行 if 删掉
					//		if (FMath::Abs(i) + FMath::Abs(j) > Step * 1.5f)
					//		{
					//			continue;
					//		}

					//		const int32 TargetX = BaseX + j;
					//		const int32 TargetY = BaseY + i;

					//		// 3. 【关键修复】正确校验边界：包含 0！(0 <= TargetX < ScreenWidth)
					//		if (TargetX < 0 || TargetX >= ScreenWidth || TargetY < 0 || TargetY >= ScreenHeight)
					//		{
					//			continue;
					//		}

					//		const FogOfWarTypes::GridIndexType NearIndex = TargetY * ScreenWidth + TargetX;
					//		if (!WorldHeightData.IsValidIndex(NearIndex)) { continue; }
					//		WorldHeightData[NearIndex] = 1;
					//	}
					//}

					const int32 CurrentX{ Index % ScreenSize.GetValue().X };
					const int32 CurrentY{ Index / ScreenSize.GetValue().X };

#if WITH_EDITOR
					struct FDebugInfo
					{
						int32 Index;
						FIntPoint ScreenSize;
						FIntPoint CurrentIndex;
						FIntPoint	TargetIndex;
						int32 NearIndex;
						FIntPoint	FixedIndex;
						FVector Location;
						FVector FixedLocation;
						double DistXY;
						int32 Count;
					};

					bool bIsShow = false;
					static int32 ShowCount{ 0 };

					TOptional<FDebugInfo> MinX;
					TOptional<FDebugInfo> MinY;
					TOptional<FDebugInfo> MaxX;
					TOptional<FDebugInfo> MaxY;
#endif
					constexpr int32 Step{ 10 };
					for (auto i = -Step; i <= Step; i++)
					{
						const auto TargetX{ CurrentX + i };
						if (TargetX < 0 || TargetX >= ScreenSize.GetValue().X) { continue; }
						for (auto j = -Step; j <= Step; j++)
						{
							const auto TargetY{ CurrentY + j };
							if (TargetY < 0 || TargetY >= ScreenSize.GetValue().Y) { continue; }

							const FogOfWarTypes::GridIndexType NearIndex{ TargetX + TargetY * ScreenSize.GetValue().X };
							if (WorldHeightData.IsValidIndex(NearIndex))
							{
								const auto FixedX{ NearIndex % ScreenSize.GetValue().X };
								const auto FixedY{ NearIndex / ScreenSize.GetValue().X };
								const TOptional<FVector> FixedLocation{ ScreenAABB.GetGridLocationByIndex(NearIndex, ScreenSize.GetValue()) };
								//if (FixedLocation.IsSet() && FVector::DistSquared(Location, FixedLocation.GetValue()) > FMath::Pow(50., 2))
								//{
								//	if (ShowCount % 300 == 0 && !bIsShow)
								//	{
								//		bIsShow = true;
								//		UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::UpdateWorldHeightData--------START--------------"));
								//		UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::UpdateWorldHeightData, Index: %d"), Index);
								//		UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::UpdateWorldHeightData, ScreenSize.GetValue(): %s"), *ScreenSize.GetValue().ToString());
								//		UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::UpdateWorldHeightData, CurrentX: %d"), CurrentX);
								//		UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::UpdateWorldHeightData, CurrentY: %d"), CurrentY);
								//		UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::UpdateWorldHeightData, TargetX: %d"), TargetX);
								//		UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::UpdateWorldHeightData, TargetY: %d"), TargetY);
								//		UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::UpdateWorldHeightData, NearIndex: %d"), NearIndex);
								//		UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::UpdateWorldHeightData, FixedX: %d"), FixedX);
								//		UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::UpdateWorldHeightData, FixedY: %d"), FixedY);
								//		UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::UpdateWorldHeightData, Location: %s"), *Location.ToString());
								//		UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::UpdateWorldHeightData, FixedLocation: %s"), *FixedLocation.GetValue().ToString());
								//		UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::UpdateWorldHeightData, FVector::DistSquared(Location, FixedLocation): %f"), FVector::DistSquared(Location, FixedLocation.GetValue()));
								//		UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::UpdateWorldHeightData--------END--------------"));
								//	}
								//	continue;
								//}

								auto SetMax = [&](TOptional<FDebugInfo>& Info)
									{
										Info = FDebugInfo{};

										Info.GetValue().Index = Index;
										Info.GetValue().ScreenSize = ScreenSize.GetValue();
										Info.GetValue().CurrentIndex = FIntPoint{ CurrentX, CurrentY };
										Info.GetValue().TargetIndex = FIntPoint{ TargetX, TargetY };
										Info.GetValue().NearIndex = NearIndex;
										Info.GetValue().FixedIndex = FIntPoint{ FixedX, FixedY };
										Info.GetValue().Location = Location;
										Info.GetValue().FixedLocation = FixedLocation.IsSet() ? FixedLocation.GetValue() : FVector::ZeroVector;
										Info.GetValue().DistXY = FixedLocation.IsSet() ? FVector::DistXY(Location, FixedLocation.GetValue()) : 0.;
									};

								if ( /*FixedLocation.IsSet() &&*/ /*ShowCount % 300 == 0*/true)
								{
									if (!MinX.IsSet() || TargetX < MinX.GetValue().TargetIndex.X) { SetMax(MinX); }
									if (!MinY.IsSet() || TargetY < MinY.GetValue().TargetIndex.Y) { SetMax(MinY); }
									if (!MaxX.IsSet() || TargetX > MaxX.GetValue().TargetIndex.X) { SetMax(MaxX); }
									if (!MaxY.IsSet() || TargetY > MaxY.GetValue().TargetIndex.Y) { SetMax(MaxY); }
								}

								WorldHeightData[NearIndex] = 1;
							}
						}
					}

					auto PrintMax = [](const FDebugInfo& Info, const FString& Name)
						{
							UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::UpdateWorldHeightData--------START--------------"));
							UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::UpdateWorldHeightData, Name: %s"), *Name);
							UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::UpdateWorldHeightData, Index: %d"), Info.Index);
							UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::UpdateWorldHeightData, ScreenSize.GetValue(): %s"), *Info.ScreenSize.ToString());
							UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::UpdateWorldHeightData, CurrentX: %d"), Info.CurrentIndex.X);
							UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::UpdateWorldHeightData, CurrentY: %d"), Info.CurrentIndex.Y);
							UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::UpdateWorldHeightData, TargetX: %d"), Info.TargetIndex.X);
							UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::UpdateWorldHeightData, TargetY: %d"), Info.TargetIndex.Y);
							UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::UpdateWorldHeightData, NearIndex: %d"), Info.NearIndex);
							UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::UpdateWorldHeightData, FixedX: %d"), Info.FixedIndex.X);
							UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::UpdateWorldHeightData, FixedY: %d"), Info.FixedIndex.Y);
							UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::UpdateWorldHeightData, Location: %s"), *Info.Location.ToString());
							UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::UpdateWorldHeightData, FixedLocation: %s"), *Info.FixedLocation.ToString());
							UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::UpdateWorldHeightData, FVector::DistXY(Location, FixedLocation): %f"), Info.DistXY);
							UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::UpdateWorldHeightData--------END--------------"));
						};

					if (ShowCount % 300 == 0)
					{
						if (MinX.IsSet()) { PrintMax(MinX.GetValue(), TEXT("Min X")); }
						if (MinY.IsSet()) { PrintMax(MinY.GetValue(), TEXT("Min Y")); }
						if (MaxX.IsSet()) { PrintMax(MaxX.GetValue(), TEXT("Max X")); }
						if (MaxY.IsSet()) { PrintMax(MaxY.GetValue(), TEXT("Max Y")); }
					}
					ShowCount++;

					//constexpr int32 Step{ 10 };
					//for (auto i = -Step; i <= Step; i++)
					//{
					//	if (i < 0 || i >= ScreenSize.GetValue().Y) { continue; }
					//	for (auto j = -Step; j <= Step; j++)
					//	{
					//		if (j < 0 || j >= ScreenSize.GetValue().X) { continue; }
					//		FogOfWarTypes::GridIndexType NearIndex = Index + i * ScreenSize.GetValue().X + j;
					//		if (WorldHeightData.IsValidIndex(NearIndex))
					//		{
					//			WorldHeightData[NearIndex] = It->Value > 0 ? 1 : 0;
					//		}
					//	}
					//}
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
	if (bCreateNewOne || !CachedOutputTexture.IsValid())
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
	else
	{
		ShaderOutputTexture = GraphBuilder.RegisterExternalTexture(CachedOutputTexture);
	}

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