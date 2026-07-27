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
	if (bHasInvalidComponents)
	{
		bHasInvalidComponents = false;

		FogOfWarComponents.RemoveAll([](TWeakObjectPtr<UFogOfWarComponent>& ComponentPtr) {return !ComponentPtr.IsValid(); });
		for (auto It{ LastComponentOwnerOrCameraTransformMap.CreateIterator() }; It; ++It)
		{
			if (!It->Key.IsValid()) { It.RemoveCurrent(); }
		}
	}

	if (!IsCameraFOVChanged()) { return; }

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

			int32 ScreenWidth, ScreenHeight;
			GetScreenSize(ScreenWidth, ScreenHeight);
			PassParameters->TextureSize.X = ScreenWidth;
			PassParameters->TextureSize.Y = ScreenHeight;
			//PassParameters->TextureSize = FVector2f(FogOfWarConst::kTextureWidth, FogOfWarConst::kTextureHeight);
			//PassParameters->TextureSize = FVector2f(FogOfWarConst::kScreenWidth, FogOfWarConst::kScreenHeight);			

			UploadFogOfWarActorData(ActorPositions, ActorVision, RadiusSqList, *PassParameters, GraphBuilder);
			UploadFogOfWarWorldHeightData(*PassParameters, GraphBuilder, TEXT("WorldHeightData"));

			FRDGTextureRef OutputRDGTexture;
			SetComputeShaderOutputTextureCache(OutputRDGTexture, *PassParameters, GraphBuilder, false);
			bViewportResized = false;

			const FIntVector ThreadCount{ ScreenWidth, ScreenHeight, 1 };
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

bool UFogOfWarSubsystem::GetScreenSize(int32& ScreenWidth, int32& ScreenHeight) const
{
	if (!bIsInitialScale) { return false; }

	ScreenWidth = kScreenBaseWidth * WidthScaleFactor;
	ScreenHeight = kScreenBaseHeight * HeightScaleFactor;
	return true;
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

	int32 ScreenWidth, ScreenHeight;
	GetScreenSize(ScreenWidth, ScreenHeight);
	DynamicTexture = UTexture2D::CreateTransient(ScreenWidth, ScreenHeight, FogOfWarConst::PixelFormat);
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

	FVector Origin, BoxExtent;
	if (!WorldHeightSubsystem->GetLandBounds(Origin, BoxExtent)) { return; }

	{
		//FVector4 LeftDownPosition{ Origin - BoxExtent };
		//LeftDownPosition.Z = Origin.Z + BoxExtent.Z;
		//LeftDownPosition.Z -= 1;
		//FogOfWarMaterial->SetDoubleVectorParameterValue(TEXT("LeftDownPosition"), LeftDownPosition);

		//FVector4 RightTopPosition{ Origin + BoxExtent };
		//RightTopPosition.Z -= 1;
		//FogOfWarMaterial->SetDoubleVectorParameterValue(TEXT("RightTopPosition"), RightTopPosition);
		TArray<FVector> Corners = UFogOfWarComponentStatics::GetCameraFrustumGroundIntersections(this);
		//if (!ensure(Corners.Num() >= 4)) { return; }
		if (Corners.Num() < 4) { return; }
		FVector4 LeftDownPosition{ Corners[static_cast<int32>(ECorner::LeftDown)] };
		LeftDownPosition.X = FMath::Max(LeftDownPosition.X, (Origin - BoxExtent).X);
		LeftDownPosition.Y = FMath::Max(LeftDownPosition.Y, (Origin - BoxExtent).Y);
		LeftDownPosition.Z -= 1;
		FVector4 RightTopPosition{ Corners[static_cast<int32>(ECorner::RightTop)] };
		RightTopPosition.X = FMath::Min(RightTopPosition.X, (Origin + BoxExtent).X);
		RightTopPosition.Y = FMath::Min(RightTopPosition.Y, (Origin + BoxExtent).Y);
		RightTopPosition.Z -= 1;

		//UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::SetLandLocationAndSizeParameters LeftDownPosition %s"), *LeftDownPosition.ToString());
		//UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::SetLandLocationAndSizeParameters RightTopPosition %s"), *RightTopPosition.ToString());

		FogOfWarMaterial->SetDoubleVectorParameterValue(TEXT("LeftDownPosition"), LeftDownPosition);
		FogOfWarMaterial->SetDoubleVectorParameterValue(TEXT("RightTopPosition"), RightTopPosition);
	}
}

void UFogOfWarSubsystem::CreateWorldHeightTexture()
{
	int32 ScreenWidth, ScreenHeight;
	GetScreenSize(ScreenWidth, ScreenHeight);

	WorldHeightTexture = UTexture2D::CreateTransient(ScreenWidth, ScreenHeight, FogOfWarConst::PixelFormat);
	WorldHeightTexture->UpdateResource();
}

bool UFogOfWarSubsystem::ProjectWorldToLand(FIntPoint& Position, const FVector2f& WorldLocation, const FVector2f& LandLeftDownLocation, const FVector2f& LandSize) const
{
	//FVector2f PositionOnLand = WorldLocation - LandLeftDownLocation;
	//Position.X = FMath::Floor(PositionOnLand.X * FogOfWarConst::kTextureWidth / LandSize.X);
	//Position.Y = FMath::Floor(PositionOnLand.Y * FogOfWarConst::kTextureHeight / LandSize.Y);

	//return Position.X >= 0 && Position.Y >= 0;


	TArray<FVector> Corners = UFogOfWarComponentStatics::GetCameraFrustumGroundIntersections(this);
	if (Corners.Num() < 4) { return false; }

	FGridBoundsDataType ScreenAABB{ Corners[static_cast<int32>(ECorner::LeftDown)], Corners[static_cast<int32>(ECorner::RightTop)] };
	ScreenAABB.Box.Min.X = FMath::Max(ScreenAABB.Box.Min.X, (LandLeftDownLocation).X);
	ScreenAABB.Box.Min.Y = FMath::Max(ScreenAABB.Box.Min.Y, (LandLeftDownLocation).Y);
	ScreenAABB.Box.Max.X = FMath::Min(ScreenAABB.Box.Max.X, (LandLeftDownLocation + LandSize).X);
	ScreenAABB.Box.Max.Y = FMath::Min(ScreenAABB.Box.Max.Y, (LandLeftDownLocation + LandSize).Y);
	ScreenAABB.Box.Max.Z = ScreenAABB.Box.Min.Z + 1;

	//UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::SetLandLocationAndSizeParameters LandLeftDownLocation %s"), *LandLeftDownLocation.ToString());
	//UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::SetLandLocationAndSizeParameters WorldLocation %s"), *WorldLocation.ToString());
	//UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::SetLandLocationAndSizeParameters ScreenAABB %s"), *ScreenAABB.Box.ToString());
	//UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::SetLandLocationAndSizeParameters WorldLocation %d"), ScreenAABB.IsInsideXY(WorldLocation));
	//if (!ScreenAABB.IsInsideXY(WorldLocation)) { return false; }

	int32 ScreenWidth, ScreenHeight;
	GetScreenSize(ScreenWidth, ScreenHeight);
	FVector2f PositionOnLand = WorldLocation - FVector2f{ static_cast<float>(ScreenAABB.Box.Min.X), static_cast<float>(ScreenAABB.Box.Min.Y) };
	Position.X = FMath::Floor(PositionOnLand.X * ScreenWidth / (ScreenAABB.Box.Max.X - ScreenAABB.Box.Min.X));
	Position.Y = FMath::Floor(PositionOnLand.Y * ScreenHeight / (ScreenAABB.Box.Max.Y - ScreenAABB.Box.Min.Y));

	return Position.X >= 0 && Position.Y >= 0;
}

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
	FVector2f LandLeftDownLocation;
	FVector2f LandSize;

	if (!GetWorld()) { return; }
	UWorldHeightSubsystem* WorldHeightSubsystem = GetWorld()->GetSubsystem<UWorldHeightSubsystem>();
	if (!WorldHeightSubsystem) { return; }

	FVector Origin, BoxExtent;
	if (!WorldHeightSubsystem->GetLandBounds(Origin, BoxExtent)) { return; }

	LandLeftDownLocation.X = Origin.X - BoxExtent.X;
	LandLeftDownLocation.Y = Origin.Y - BoxExtent.Y;

	LandSize.X = BoxExtent.X * 2;
	LandSize.Y = BoxExtent.Y * 2;

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
			FVector2f ActorLocation{ static_cast<float>(Data.ActorLocation.X), static_cast<float>(Data.ActorLocation.Y) };

			FIntPoint ActorPosition;
			if (!ProjectWorldToLand(ActorPosition, ActorLocation, LandLeftDownLocation, LandSize)) { continue; }

			FVector2D GridSize;
			if (!WorldHeightSubsystem->GetGridSize(GridSize)) { continue; }

			ActorPositions.Add(ActorPosition);
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

	int32 ScreenWidth, ScreenHeight;
	GetScreenSize(ScreenWidth, ScreenHeight);
	if (WorldHeightData.Num() != (ScreenWidth * ScreenHeight))
	{
		WorldHeightData.SetNumZeroed(ScreenWidth * ScreenHeight);
	}

	{
		CachedWorldHeightDataVersion = WorldHeightSubsystem->GetWorldHeightDataVersion();

		FMemory::Memzero(WorldHeightData.GetData(), WorldHeightData.GetAllocatedSize());
		//for (auto It = WorldHeightSubsystem->GetWorldHeightMap().CreateConstIterator(); It; ++It)
		//{
		//	WorldHeightData[It->Key] = It->Value > 0 ? 1 : 0;
		//}

		{
			FVector LandOrigin, LandExtent;
			WorldHeightSubsystem->GetLandBounds(LandOrigin, LandExtent);

			TArray<FVector> Corners = UFogOfWarComponentStatics::GetCameraFrustumGroundIntersections(this);
			if (Corners.Num() < 4) { return; }

			FGridBoundsDataType ScreenAABB{ Corners[static_cast<int32>(ECorner::LeftDown)], Corners[static_cast<int32>(ECorner::RightTop)] };
			ScreenAABB.Box.Min.X = FMath::Max(ScreenAABB.Box.Min.X, (LandOrigin - LandExtent).X);
			ScreenAABB.Box.Min.Y = FMath::Max(ScreenAABB.Box.Min.Y, (LandOrigin - LandExtent).Y);
			ScreenAABB.Box.Max.X = FMath::Min(ScreenAABB.Box.Max.X, (LandOrigin + LandExtent).X);
			ScreenAABB.Box.Max.Y = FMath::Min(ScreenAABB.Box.Max.Y, (LandOrigin + LandExtent).Y);
			ScreenAABB.Box.Max.Z = ScreenAABB.Box.Min.Z + 1;
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
					FogOfWarTypes::GridIndexType Index{ ScreenAABB.GetGridIndexOnScreen(Location, FIntPoint{ScreenWidth, ScreenHeight}) };
					if (FMath::Abs(Location.X - 2010) < 200 && FMath::Abs(Location.Y - 1680) < 200)
					{
						//UE_LOG(LogTemp, Error, TEXT("UFogOfWarSubsystem::SetLandLocationAndSizeParameters Index: %d"), Index);
					}
					if (WorldHeightData.IsValidIndex(Index))
					{
						WorldHeightData[Index] = It->Value > 0 ? 1 : 0;
					}
					for (int i = -2; i <= 2; i++)
					{
						for (int j = -2; j <= 2; j++)
						{
							FogOfWarTypes::GridIndexType NearIndex = Index + i * ScreenWidth + j;
							if (WorldHeightData.IsValidIndex(NearIndex))
							{
								WorldHeightData[NearIndex] = It->Value > 0 ? 1 : 0;
							}
						}
					}
				}
			}
		}
	}
}

void UFogOfWarSubsystem::UpdateWorldHeightDataToTexture(FRHICommandListImmediate& RHICmdList)
{
	int32 ScreenWidth, ScreenHeight;
	GetScreenSize(ScreenWidth, ScreenHeight);
	FUpdateTextureRegion2D Region(0, 0, 0, 0, ScreenWidth, ScreenHeight);

	// 2. 使用RHI命令直接更新
	// 这是最底层、最高效的调用方法之一，直接从系统内存更新纹理
	RHICmdList.UpdateTexture2D(
		WorldHeightTexture->GetResource()->GetTextureRHI(),          // RHI纹理资源
		0,                   // Mip索引
		Region,              // 更新区域
		WorldHeightData.GetTypeSize() * ScreenWidth,    // 数据行距（Pitch）
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
		int32 ScreenWidth, ScreenHeight;
		GetScreenSize(ScreenWidth, ScreenHeight);

		FRDGTextureDesc TextureDesc = FRDGTextureDesc::Create2D(
			FIntPoint(ScreenWidth, ScreenHeight),
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