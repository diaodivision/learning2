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

	// GEngine->GetPreRenderDelegateEx().AddUObject(this, &UFogOfWarSubsystem::OnPreRender);
	// GEngine->GetPostRenderDelegateEx().AddUObject(this, &UFogOfWarSubsystem::OnPostRender);
	// FWorldDelegates::OnWorldPostActorTick.AddUObject(this, &UFogOfWarSubsystem::OnPostActorTick);
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

	const TOptional<FIntPoint> ScreenSize{ GetScreenSize() };
	if (!ScreenSize.IsSet()) { return; }
	UE_LOG(LogTemp, Error, TEXT("ScreenSize: %s"), *ScreenSize.GetValue().ToString());

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

TOptional<FVector2D> UFogOfWarSubsystem::GetGridSize() const
{
	const TOptional<FBox2D> CameraBounds{ UFogOfWarComponentStatics::GetCameraFrustumGroundIntersections(this) };
	if (!CameraBounds.IsSet()) { return NullOpt; }

	return FVector2D{ CameraBounds.GetValue().GetSize() / GetScreenSize().GetValue() };
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

		// TArray<FVector> Corners = UFogOfWarComponentStatics::GetCameraFrustumGroundIntersections(this);

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

	// TArray<FVector> Corners = UFogOfWarComponentStatics::GetCameraFrustumGroundIntersections(this);
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

		FFogOfWarData Data;
		if (Component->GetFogOfWarData(Data))
		{
			// const TOptional<FIntPoint> ActorPosition{ ProjectWorldToLand(FVector2D{ Data.ActorLocation }, LandBoundingBox.GetValue()) };
			// if (!ActorPosition.IsSet()) { continue; }

			//TOptional<FIntPoint> ActorPosition{ FIntPoint{} };
			//if (!ProjectWorldToLand(ActorPosition.GetValue(), FVector2D{ Data.ActorLocation }, LandBoundingBox.GetValue())) { continue; }

			const TOptional<FVector> GridSize{WorldHeightSubsystem->GetGridSize()};
			if (!GridSize.IsSet()) { continue; }

			const TOptional<FIntPoint> ScreenSize{ GetScreenSize() };
			if (!ScreenSize.IsSet()) { return; }

			const TOptional<FIntPoint> PositionOnScreen{UFogOfWarComponentStatics::GetGridPositionOnScreen(Data.ActorLocation, ScreenSize.GetValue(), ScreenBox.GetValue()) };
			if (!PositionOnScreen.IsSet()) { continue; }

			ActorPositions.Add(PositionOnScreen.GetValue());
			// ActorVision.Add(FVector2f{ static_cast<float>(Data.ActorVisionLeft.X), static_cast<float>(Data.ActorVisionLeft.Y) });
			// ActorVision.Add(FVector2f{ static_cast<float>(Data.ActorVisionRight.X), static_cast<float>(Data.ActorVisionRight.Y) });
			ActorVision.Add(FVector2f{UFogOfWarComponentStatics::ProjectWorldDirectionToScreen(Data.ActorVisionLeft)});
			ActorVision.Add(FVector2f{UFogOfWarComponentStatics::ProjectWorldDirectionToScreen(Data.ActorVisionRight)});
			RadiusSqList.Add(FMath::CeilToInt32(FMath::Pow(Data.Radius, 2.f) / GridSize.GetValue().X));	//todo 这里应该是先做除法再平方
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

			// TArray<FVector> Corners = UFogOfWarComponentStatics::GetCameraFrustumGroundIntersections(this);
			// if (Corners.Num() < 4) { return; }

			const TOptional<FBox2D> ScreenAABB{ UFogOfWarComponentStatics::GetCameraFrustumGroundIntersections(this) };
			if (!ScreenAABB.IsSet()) {return;}
			// FGridBoundsDataType ScreenAABB{ Corners[static_cast<int32>(ECorner::LeftDown)], Corners[static_cast<int32>(ECorner::RightTop)] };



			// const FBox2D ScreenBox = CalculateSnappedScreenAABB(Corners, ScreenSize.GetValue());
// if (!ScreenBox.bIsValid) { return; }
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

			for (auto It = WorldHeightSubsystem->GetWorldHeightMap().CreateConstIterator(); It; ++It)
			{
				// 1. 【必须】非障碍物直接跳过！不参与膨胀，防止把旁边的 1 误覆写为 0
				if (It->Value <= 0)
				{
					continue;
				}

				FVector Location;
				if (WorldHeightSubsystem->GetGridLocationByIndex(Location, It->Key))
				{
					const FBox2D ScreenBox{FVector2D{ScreenAABB.GetValue().Min}, FVector2D{ScreenAABB.GetValue().Max}};
					const FogOfWarTypes::GridIndexType Index { UFogOfWarComponentStatics::GetGridIndexOnScreen(Location, ScreenSize.GetValue(), ScreenBox)};
					if (WorldHeightData.IsValidIndex(Index))
					{
						WorldHeightData[Index] = 1;

					const TOptional<FIntPoint> CurrentIndex{UFogOfWarComponentStatics::GetGridPositionOnScreen(Location, ScreenSize.GetValue(), ScreenBox)};
					if (!CurrentIndex.IsSet()) { continue; }
					constexpr int32 Step{ 2 };
					for (auto i = -(Step * 3); i <= (Step * 3); i++)
					{
						const auto TargetX{ CurrentIndex.GetValue().X + i };
						// if (TargetX < 0 || TargetX >= ScreenSize.GetValue().X) { continue; }
						for (auto j = -Step; j <= Step; j++)
						{
							const auto TargetY{ CurrentIndex.GetValue().Y + j };
							// if (TargetY < 0 || TargetY >= ScreenSize.GetValue().Y) { continue; }

							const FogOfWarTypes::GridIndexType NearIndex{ TargetX + TargetY * ScreenSize.GetValue().X };
							if (WorldHeightData.IsValidIndex(NearIndex))
							{
								WorldHeightData[NearIndex] = 1;
							}
						}
					}
					}
					
					// // 2. 【核心修复】直接根据世界坐标与 ScreenAABB 计算 2D 归一化比例 (0.0 ~ 1.0)
					// // 彻底绕过会产生 uint32 溢出或 modulo 卷绕的 GetGridIndexOnScreen 函数
					// float NormX = (Location.X - BoxMin.X) / BoxSize.X;
					// float NormY = (Location.Y - BoxMin.Y) / BoxSize.Y;

					// // 3. 使用 FloorToInt32 正确处理负数坐标（屏幕下方的障碍物 NormY < 0，BaseY 算出来就是 -1, -2 等负数）
					// int32 BaseX = FMath::FloorToInt32(NormX * ScreenWidth);
					// int32 BaseY = FMath::FloorToInt32(NormY * ScreenHeight);

					// constexpr int32 Step{ 10 };

					// // 4. 【核心修复】屏外粗裁剪：如果障碍物距离屏幕边缘超过膨胀半径 Step，直接跳过！
					// // 当障碍物在屏幕下方很远时，BaseY 是 -20，-20 < -10 成立，这里会直接 continue 丢弃，绝对不会影响屏幕顶部！
					// if (BaseX < -Step || BaseX >= ScreenWidth + Step ||
					// 	BaseY < -Step || BaseY >= ScreenHeight + Step)
					// {
					// 	continue;
					// }

					// // 5. 2D 局部膨胀
					// for (int32 i = -Step; i <= Step; i++)
					// {
					// 	for (int32 j = -Step; j <= Step; j++)
					// 	{
					// 		int32 TargetX = BaseX + j;
					// 		int32 TargetY = BaseY + i;

					// 		// 6. 只有真正落在屏幕 [0, ScreenWidth-1] 和 [0, ScreenHeight-1] 内的像素才会被标记
					// 		if (TargetX >= 0 && TargetX < ScreenWidth && TargetY >= 0 && TargetY < ScreenHeight)
					// 		{
					// 			int32 NearIndex = TargetY * ScreenWidth + TargetX;
					// 			WorldHeightData[NearIndex] = 1;
					// 		}
					// 	}
					// }
				}
			}
#if !UE_BUILD_SHIPPING
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
						int32 OriginIndex;
						FIntPoint OriginCurrentIndex;
						FVector OriginLocation;
					};

					bool bIsShow = false;
					static int32 ShowCount{ 0 };

					TOptional<FDebugInfo> MinX;
					TOptional<FDebugInfo> MinY;
					TOptional<FDebugInfo> MaxX;
					TOptional<FDebugInfo> MaxY;
#endif

// auto C{ GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr };
// auto P{ C ? C->GetPawn() : nullptr };

// if (P) 
// {
//     FVector L = P->GetActorLocation();

//     // 1. 获取包围盒的真实 Min/Max 与 尺寸
//     FVector BoxMin = ScreenAABB.Box.Min;
//     FVector BoxMax = ScreenAABB.Box.Max;
//     FVector BoxSize = BoxMax - BoxMin; // 真实世界尺寸

//     // 2. 获取纹理的实际分辨率
//     int32 TexWidth = ScreenSize.GetValue().X;
//     int32 TexHeight = ScreenSize.GetValue().Y;

//     if (BoxSize.X > 0.0f && BoxSize.Y > 0.0f && TexWidth > 0 && TexHeight > 0)
//     {
//         // 3. 将世界坐标归一化到 [0.0, 1.0] 的 UV 空间
//         // 注意：UE 中世界 Y 对应水平 U 轴，世界 X 对应垂直 V 轴
//         // 假设纹理左上角为 V=0 (对应世界坐标 Max.X)，右下角为 V=1 (对应世界坐标 Min.X)
//         double NormalizedU = (L.Y - BoxMin.Y) / BoxSize.Y;
//         double NormalizedV = (BoxMax.X - L.X) / BoxSize.X; // X 越大越靠上，所以用 Max.X - L.X

//         // 4. 将 UV 映射到像素网格坐标，并强制 Clamp 防止玩家走出包围盒导致越界崩溃
//         int32 GridX = FMath::Clamp(FMath::FloorToInt(NormalizedU * TexWidth), 0, TexWidth - 1);
//         int32 GridY = FMath::Clamp(FMath::FloorToInt(NormalizedV * TexHeight), 0, TexHeight - 1);

//         // 5. 计算一维数组索引
//         const int32 I = GridY * TexWidth + GridX;

//         // 6. 安全读取数据
//         if (WorldHeightData.IsValidIndex(I))
//         {
//             WorldHeightData[I] = 1;

// 			constexpr int32 Step{ 20 };
// 					for (auto i = -Step; i <= Step; i++)
// 					{
// 						const auto TargetX{ GridX + i };
// 						// if (TargetX < 0 || TargetX >= ScreenSize.GetValue().X) { continue; }
// 						for (auto j = -Step; j <= Step; j++)
// 						{
// 							const auto TargetY{ GridY + j };
// 							// if (TargetY < 0 || TargetY >= ScreenSize.GetValue().Y) { continue; }

// 							const FogOfWarTypes::GridIndexType NearIndex{ TargetX + TargetY * ScreenSize.GetValue().X };
// 							if (WorldHeightData.IsValidIndex(NearIndex))
// 							{
// 								WorldHeightData[NearIndex] = 1;
// 							}
// 						}
// 					}
//         }
//     }
// }

			if (true) {return;}
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

					// FogOfWarTypes::GridIndexType Index{ ScreenAABB.GetGridIndexOnScreen(Location, ScreenSize.GetValue()) };
					int32 Index = 0;
					if (FMath::Abs(Location.X - 2010) < 200 && FMath::Abs(Location.Y - 1680) < 200)
					{
						//UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::SetLandLocationAndSizeParameters Index: %d"), Index);
					}



					const int32 CurrentX{ Index % ScreenSize.GetValue().X };
					const int32 CurrentY{ Index / ScreenSize.GetValue().X };

					// Index = (ScreenSize.GetValue().Y - CurrentY) * ScreenSize.GetValue().X + CurrentX;

					auto SetMax0 = [&](TOptional<FDebugInfo>& Info)
							{
								if (!Info.IsSet()) { Info = FDebugInfo{}; }	
								Info.GetValue().OriginIndex = Index;
								Info.GetValue().OriginCurrentIndex = FIntPoint{ CurrentX, CurrentY };
								Info.GetValue().OriginLocation = Location;
							};


					// if ( /*FixedLocation.IsSet() &&*/ /*ShowCount % 300 == 0*/true)
					// {
					// 	if (!MinX.IsSet() || CurrentX < MinX.GetValue().OriginCurrentIndex.X) { SetMax0(MinX); }
					// 	if (!MinY.IsSet() || CurrentY < MinY.GetValue().OriginCurrentIndex.Y) { SetMax0(MinY); }
					// 	if (!MaxX.IsSet() || CurrentX > MaxX.GetValue().OriginCurrentIndex.X) { SetMax0(MaxX); }
					// 	if (!MaxY.IsSet() || CurrentY > MaxY.GetValue().OriginCurrentIndex.Y) { SetMax0(MaxY); }
					// }

					if (WorldHeightData.IsValidIndex(Index)) { WorldHeightData[Index] = It->Value > 0 ? 1 : 0; }
					else { continue; }

					// if ((It->Value > 0) == false) { continue; }


					constexpr int32 Step{ 2 };
					for (auto i = -Step; i <= Step; i++)
					{
						const auto TargetX{ CurrentX + i };
						// if (TargetX < 0 || TargetX >= ScreenSize.GetValue().X) { continue; }
						for (auto j = -Step; j <= Step; j++)
						{
							const auto TargetY{ CurrentY + j };
							// if (TargetY < 0 || TargetY >= ScreenSize.GetValue().Y) { continue; }

							const FogOfWarTypes::GridIndexType NearIndex{ TargetX + TargetY * ScreenSize.GetValue().X };
							if (WorldHeightData.IsValidIndex(NearIndex))
							{
								// const auto FixedX{ NearIndex % ScreenSize.GetValue().X };
								// const auto FixedY{ NearIndex / ScreenSize.GetValue().X };
								// const TOptional<FVector> FixedLocation{ ScreenAABB.GetGridLocationByIndex(NearIndex, ScreenSize.GetValue()) };

								// auto SetMax = [&](TOptional<FDebugInfo>& Info, const bool bIsSetCurrent = false)
								// 	{
								// 		// Info = FDebugInfo{};
								// 		if (!Info.IsSet()) { Info = FDebugInfo{}; }

								// 		Info.GetValue().Index = Index;
								// 		Info.GetValue().ScreenSize = ScreenSize.GetValue();
								// 		Info.GetValue().CurrentIndex = FIntPoint{ CurrentX, CurrentY };
								// 		Info.GetValue().TargetIndex = bIsSetCurrent ? FIntPoint{ CurrentX, CurrentY } : FIntPoint{ TargetX, TargetY };
								// 		Info.GetValue().NearIndex = NearIndex;
								// 		Info.GetValue().FixedIndex = FIntPoint{ FixedX, FixedY };
								// 		Info.GetValue().Location = Location;
								// 		Info.GetValue().FixedLocation = FixedLocation.IsSet() ? FixedLocation.GetValue() : FVector::ZeroVector;
								// 		Info.GetValue().DistXY = FixedLocation.IsSet() ? FVector::DistXY(Location, FixedLocation.GetValue()) : 0.;
								// 	};

								// if ( /*FixedLocation.IsSet() &&*/ /*ShowCount % 300 == 0*/true)
								// {
								// 	if (!MinX.IsSet() || CurrentX < MinX.GetValue().TargetIndex.X) { SetMax(MinX, true); }
								// 	if (!MinY.IsSet() || CurrentY < MinY.GetValue().TargetIndex.Y) { SetMax(MinY, true); }
								// 	if (!MaxX.IsSet() || CurrentX > MaxX.GetValue().TargetIndex.X) { SetMax(MaxX, true); }
								// 	if (!MaxY.IsSet() || CurrentY > MaxY.GetValue().TargetIndex.Y) { SetMax(MaxY, true); }

								// 	if (!MinX.IsSet() || TargetX < MinX.GetValue().TargetIndex.X) { SetMax(MinX); }
								// 	if (!MinY.IsSet() || TargetY < MinY.GetValue().TargetIndex.Y) { SetMax(MinY); }
								// 	if (!MaxX.IsSet() || TargetX > MaxX.GetValue().TargetIndex.X) { SetMax(MaxX); }
								// 	if (!MaxY.IsSet() || TargetY > MaxY.GetValue().TargetIndex.Y) { SetMax(MaxY); }
								// }

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
							UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::UpdateWorldHeightData, OriginIndex: %d"), Info.OriginIndex);
							UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::UpdateWorldHeightData, OriginCurrentIndex: %s"), *Info.OriginCurrentIndex.ToString());
							UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::UpdateWorldHeightData, OriginLocation: %s"), *Info.OriginLocation.ToString());
							UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::UpdateWorldHeightData--------END--------------"));
						};

					// if (ShowCount % 300 == 0)
					// {
					// 	if (MinX.IsSet()) { PrintMax(MinX.GetValue(), TEXT("Min X")); }
					// 	if (MinY.IsSet()) { PrintMax(MinY.GetValue(), TEXT("Min Y")); }
					// 	if (MaxX.IsSet()) { PrintMax(MaxX.GetValue(), TEXT("Max X")); }
					// 	if (MaxY.IsSet()) { PrintMax(MaxY.GetValue(), TEXT("Max Y")); }
					// }
					ShowCount++;
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

void UFogOfWarSubsystem::OnPreRender(FRDGBuilder& GraphBuilder)
{
    // 1. 安全检查（确保 GT 侧已经初始化好纹理资源）
    if (!bIsInitialScale || !WorldHeightTexture || !WorldHeightTexture->GetResource() || !WorldHeightTexture->GetResource()->TextureRHI)
    {
        return;
    }

    const TOptional<FIntPoint> ScreenSize{ GetScreenSize() };
    if (!ScreenSize.IsSet() || ScreenSize.GetValue().X <= 0 || ScreenSize.GetValue().Y <= 0) 
    { 
        return; 
    }

	if (WorldHeightData.Num() != ScreenSize.GetValue().X * ScreenSize.GetValue().Y) {return;}

    // 2. 更新 CPU WorldHeight 数组到 RHI 纹理
    UpdateWorldHeightDataToTexture(GraphBuilder.RHICmdList);

    // 3. 构建 Compute Shader 参数
    FFogOfWarComputeShader::FParameters* PassParameters = GraphBuilder.AllocParameters<FFogOfWarComputeShader::FParameters>();
    PassParameters->TextureSize.X = ScreenSize.GetValue().X;
    PassParameters->TextureSize.Y = ScreenSize.GetValue().Y;

    // 上传数据 Buffer (使用在 GT 提前准备好的数据)
    UploadFogOfWarActorData(CachedActorPositions, CachedActorVision, CachedRadiusSqList, *PassParameters, GraphBuilder);
    UploadFogOfWarWorldHeightData(*PassParameters, GraphBuilder, TEXT("WorldHeightData"));

    // 获取/创建输出 RDG 纹理
    FRDGTextureRef OutputRDGTexture;
    SetComputeShaderOutputTextureCache(OutputRDGTexture, *PassParameters, GraphBuilder, false);

    // 4. 计算 Dispatch 组数并添加 CS Pass 到当前管线
    const FIntVector ThreadCount{ ScreenSize.GetValue().X, ScreenSize.GetValue().Y, 1 };
    const FIntVector GroupSize{ FogOfWarConst::kThreadsX, FogOfWarConst::kThreadsY, FogOfWarConst::kThreadsZ };
    const FIntVector DispatchCount = FComputeShaderUtils::GetGroupCount(ThreadCount, GroupSize);

    TShaderMapRef<FFogOfWarComputeShader> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

    // 向引擎当前的 RDG 中插入计算 Pass
    FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("CalculateVisionArea"), ComputeShader, PassParameters, DispatchCount);

    // 提取计算结果存入 CachedOutputTexture 供后续材质使用
    GraphBuilder.QueueTextureExtraction(OutputRDGTexture, &CachedOutputTexture);

    // 5. 如果需要把结果拷贝到动态纹理 DynamicTexture 的 RHI 资源中
    if (DynamicTexture && DynamicTexture->GetResource() && DynamicTexture->GetResource()->TextureRHI)
    {
        FRDGTextureRef DestRDGTexture = GraphBuilder.RegisterExternalTexture(
            CreateRenderTarget(DynamicTexture->GetResource()->TextureRHI, TEXT("FogOfWarDynamicDest"))
        );

        // 直接通过 RDG 添加拷贝 Pass，不需要手写 ENQUEUE_RENDER_COMMAND
        AddCopyTexturePass(GraphBuilder, OutputRDGTexture, DestRDGTexture);
    }

    // ⚠️ 注意：结尾绝对不要调用 GraphBuilder.Execute()！引擎会在帧末尾统一执行！
}

void UFogOfWarSubsystem::OnPostActorTick(UWorld* World, ELevelTick TickType, float DeltaSeconds)
{
    // 确保只处理当前 Subsystem 所在的 World
    if (World == GetWorld() && TickType == LEVELTICK_All)
    {
        // 此时所有 Actor（包含 CameraManager）都已经 Tick 完成
		UpdateWorldHeightData();

	GetFogOfWarActorData(CachedActorPositions, CachedActorVision, CachedRadiusSqList);
    }
}

// void UFogOfWarSubsystem::Tick(float DeltaTime)
// {
//     if (UGameplayStatics::IsGamePaused(this)) { return; }

//     if (bHasInvalidComponents)
//     {
//         bHasInvalidComponents = false;
//         FogOfWarComponents.RemoveAll([](TWeakObjectPtr<UFogOfWarComponent>& ComponentPtr) { return !ComponentPtr.IsValid(); });
//         for (auto It{ LastComponentOwnerOrCameraTransformMap.CreateIterator() }; It; ++It)
//         {
//             if (!It->Key.IsValid()) { It.RemoveCurrent(); }
//         }
//     }

//     // 【Game Thread】初始化与视口尺寸更新
//     if (!bIsInitialScale)
//     {
//         SetupScaleFactor();
//         GEngine->GameViewport->Viewport->ViewportResizedEvent.AddUObject(this, &UFogOfWarSubsystem::OnViewportResized);
//         CreateWorldHeightTexture();
//         CreateDynamicTexture();
//         bIsInitialScale = true;
//     }

//     if (bViewportResized)
//     {
//         CachedOutputTexture.SafeRelease();
//         CreateWorldHeightTexture();
//         CreateDynamicTexture();
//         bViewportResized = false;
//     }

//     // 【Game Thread】更新 CPU 数据与收集计算参数
//     // UpdateWorldHeightData();
    
//     // 收集 Actor 坐标与视野数据存入缓存变量 (如 RenderInputsCache)
//     // PrepareRenderInputs_GameThread();
// }

void UFogOfWarSubsystem::OnPostRender(FRDGBuilder& GraphBuilder)
{
	FogOfWarMaterial->SetTextureParameterValue(TEXT("DynamicMaterial"), DynamicTexture);
}

FBox2D UFogOfWarSubsystem::CalculateSnappedScreenAABB(const TArray<FVector>& GroundCorners, FIntPoint ScreenSize) const
{
    if (GroundCorners.Num() < 4 || ScreenSize.X <= 0 || ScreenSize.Y <= 0)
    {
        return FBox2D(ForceInit);
    }

    // 1. 正确收集 4 个角点，计算真正的 AABB Bounds
    FBox2D RealAABB(ForceInit);
    for (int32 i = 0; i < 4; ++i)
    {
        RealAABB += FVector2D(GroundCorners[i]);
    }

    const FVector2D BoxSize = RealAABB.GetSize();
    if (BoxSize.X <= 0.0f || BoxSize.Y <= 0.0f)
    {
        return RealAABB;
    }

    // 2. 计算单个像素（Texel）在世界空间中的物理尺寸
    const FVector2D TexelSize = FVector2D(BoxSize.X / ScreenSize.X, BoxSize.Y / ScreenSize.Y);

    // 3. 【核心 Fix】：Texel Snapping（像素世界对齐）
    // 将 Min 点向下对齐到 TexelSize 的整数倍，消除亚像素移动导致的抖动
    FVector2D SnappedMin;
    SnappedMin.X = FMath::FloorToDouble(RealAABB.Min.X / TexelSize.X) * TexelSize.X;
    SnappedMin.Y = FMath::FloorToDouble(RealAABB.Min.Y / TexelSize.Y) * TexelSize.Y;

    // 重新根据对齐后的 Min 和固定尺寸算出 Max
    FVector2D SnappedMax = SnappedMin + FVector2D(TexelSize.X * ScreenSize.X, TexelSize.Y * ScreenSize.Y);

    return FBox2D(SnappedMin, SnappedMax);
}