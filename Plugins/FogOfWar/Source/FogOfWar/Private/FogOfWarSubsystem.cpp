// Fill out your copyright notice in the Description page of Project Settings.


#include "FogOfWarSubsystem.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/WorldSettings.h"
#include "Kismet/GameplayStatics.h"
#include "FogOfWarComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "WorldHeightSubsystem.h"
#include "GameFramework/Character.h"
#include "FogOfWarComponentStatics.h"
#include "RenderGraphUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "FogOfWarShaderTypes.ush"
#include "FogOfWarSubsystemProviderInterface.h"
#include "GameFramework/GameModeBase.h"
#include "Configs/FogOfWarSettings.h"

bool UFogOfWarSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer)) { return false; }

	const UWorld* World{ Cast<UWorld>(Outer) };
	const AWorldSettings* WorldSettings{ World && World->IsGameWorld() ? World->GetWorldSettings() : nullptr };
	const TSubclassOf<AGameModeBase> DefaultGameMode{ WorldSettings ? WorldSettings->DefaultGameMode : nullptr };
	if (const UObject* GameMode{ DefaultGameMode ? DefaultGameMode->GetDefaultObject() : nullptr }; GameMode && GameMode->Implements<UFogOfWarSubsystemProviderInterface>())
	{
		return IFogOfWarSubsystemProviderInterface::Execute_ShouldCreateFogOfWarSubsystem(GameMode);
	}
	return false;
}

void UFogOfWarSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency<UWorldHeightSubsystem>();

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
		if (!CreateDynamicTexture()) { return; }
	}

	if (bViewportResized)
	{
		// 释放旧的、断开旧的 RHI 链条
		// CachedOutputTexture.SafeRelease();

		// 重新生成正确尺寸的 UTexture2D 资源
		if (!CreateDynamicTexture()) { return; }
	}

	if (!DynamicTexture || !FogOfWarMaterial) 
	{ 
		if (!CreateDynamicTexture()) { return; }
		return;
	}

	if (!bIsInitialScale || bViewportResized) { return; }

	TArray<FIntPoint> ActorPositions;
	TArray<FVector2f> ActorVision;
	TArray<int32> RadiusSqList;

	GetFogOfWarActorData(ActorPositions, ActorVision, RadiusSqList);


	const TOptional<FIntPoint> ScreenSize{ GetScreenSize() };
	if (!ScreenSize.IsSet()) { return; }

	FTextureResource* RenderResource = DynamicTexture ? DynamicTexture->GetResource() : nullptr;
	if (!RenderResource || !RenderResource->TextureRHI) { return; }

	const FTextureRHIRef* WorldHeightTextureRHIRefPtr{ UFogOfWarComponentStatics::GetWorldHeightSubsystem(this) ? UFogOfWarComponentStatics::GetWorldHeightSubsystem(this)->GetWorldHeightTextureRef() : nullptr };
	if (!WorldHeightTextureRHIRefPtr) { return; }

	const TOptional<FGridSizeType> ScreenGridSize{UFogOfWarComponentStatics::GetGridSize(EGridType::Screen, this)};
	const TOptional<FGridSizeType> WorldGridSize{UFogOfWarComponentStatics::GetGridSize(EGridType::World, this)};
	if (!ScreenGridSize.IsSet() || !WorldGridSize.IsSet()) { return; }
	const TOptional<FBox2D> CameraBounds{ UFogOfWarComponentStatics::GetCameraFrustumGroundIntersections(this) };
	if (!CameraBounds.IsSet()) { return; }

	const TOptional<FIntPoint> GridPositionOnWorld{UFogOfWarComponentStatics::GetGridPositionOnWorld(FVector2D{ CameraBounds.GetValue().Max.X, CameraBounds.GetValue().Min.Y}, this, EAllowMinusPosition::Yes)};
	if (!GridPositionOnWorld.IsSet()) { return; }

	const FVector2D ScreenToWorldGridScale{ScreenGridSize.GetValue().GetGridSizeOnScreenCoordinate() / WorldGridSize.GetValue().GetGridSizeOnScreenCoordinate()};
	const FVector2f ScreenToWorldGridScaleFVector2f{static_cast<float>(ScreenToWorldGridScale.X), static_cast<float>(ScreenToWorldGridScale.Y)};

	ENQUEUE_RENDER_COMMAND(CalculateVisionArea)(
		[WeakThis = MakeWeakObjectPtr(this), ActorPositions, ActorVision, RadiusSqList, RenderResource, WorldHeightTextureRHIRef = *WorldHeightTextureRHIRefPtr, GridPositionOnWorld, ScreenToWorldGridScaleFVector2f](FRHICommandListImmediate& RHICmdList)
		{
			if (!WeakThis.IsValid()) { return; }

			FRDGBuilder GraphBuilder(RHICmdList);


			// UpdateWorldHeightDataToTexture(RHICmdList);
			// 创建或重用纹理

			FFogOfWarComputeShader::FParameters* PassParameters = GraphBuilder.AllocParameters<FFogOfWarComputeShader::FParameters>();

			const TOptional<FIntPoint> ScreenSize{ WeakThis->GetScreenSize() };
			if (!ScreenSize.IsSet()) { return; }
			PassParameters->TextureSize.X = ScreenSize.GetValue().X;
			PassParameters->TextureSize.Y = ScreenSize.GetValue().Y;
			//PassParameters->TextureSize = FVector2f(FogOfWarConst::kTextureWidth, FogOfWarConst::kTextureHeight);
			//PassParameters->TextureSize = FVector2f(FogOfWarConst::kScreenWidth, FogOfWarConst::kScreenHeight);			

			WeakThis->UploadFogOfWarActorData(ActorPositions, ActorVision, RadiusSqList, *PassParameters, GraphBuilder);
			// UploadFogOfWarWorldHeightData(*PassParameters, GraphBuilder, TEXT("WorldHeightData"));
			FRDGTextureRef RDGTexture = GraphBuilder.RegisterExternalTexture(
				CreateRenderTarget(WorldHeightTextureRHIRef, TEXT("WorldHeightData"))
			);
			PassParameters->WorldHeightTexture = GraphBuilder.CreateSRV(FRDGTextureSRVDesc::Create(RDGTexture));

			// PassParameters->GridTransformContext = GridTransformContext;
			PassParameters->ScreenGridOriginInWorldGrid = GridPositionOnWorld.GetValue();
			PassParameters->ScreenToWorldGridScale = ScreenToWorldGridScaleFVector2f;

			FRDGTextureRef OutputRDGTexture{nullptr};
			WeakThis->SetComputeShaderOutputTextureCache(OutputRDGTexture, *PassParameters, GraphBuilder, false);

			const FIntVector ThreadCount{ ScreenSize.GetValue().X, ScreenSize.GetValue().Y, 1 };
			const FIntVector DispatchCount = FComputeShaderUtils::GetGroupCount(ThreadCount, UFogOfWarComponentStatics::GetFogOfWarThreadGroupSize());

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
			AsyncTask(ENamedThreads::GameThread, [WeakThis]()
				{
					if (!WeakThis.IsValid() || !WeakThis->DynamicTexture || !WeakThis->FogOfWarMaterial) { return; }			
					WeakThis->OnFogOfWarTextureUpdatedDelegate.Broadcast(WeakThis->DynamicTexture);
					WeakThis->SetTextureParameter();
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

TOptional<FIntPoint> UFogOfWarSubsystem::GetScreenSize() const
{
	const UFogOfWarSettings* FogOfWarSettings{ GetDefault<UFogOfWarSettings>() };
	if (!FogOfWarSettings || !bIsInitialScale) { return NullOpt; }

    return FIntPoint{FMath::FloorToInt32(FogOfWarSettings->kScreenBaseWidth * WidthScaleFactor), FMath::FloorToInt32(FogOfWarSettings->kScreenBaseHeight * HeightScaleFactor)}; 
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

bool UFogOfWarSubsystem::CreateDynamicTexture()
{
	UMaterialInterface* Material{ UFogOfWarComponentStatics::GetFogOfWarMaterial() };
	if (!Material) { return false; }

	const TOptional<FIntPoint> ScreenSize{ GetScreenSize() };
	if (!ScreenSize.IsSet()) { return false; }

	DynamicTexture = UTexture2D::CreateTransient(ScreenSize.GetValue().X, ScreenSize.GetValue().Y, UFogOfWarComponentStatics::GetFogOfWarTexturePixelFormat());
	if (DynamicTexture)
	{
		DynamicTexture->CompressionSettings = TC_Default;
		DynamicTexture->SRGB = false;
		DynamicTexture->Filter = UFogOfWarComponentStatics::GetFogOfWarTextureFilter();
		DynamicTexture->NeverStream = true;

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

	// bIsInitialScale = true;
	bViewportResized = false;
	return true;
}

void UFogOfWarSubsystem::SetTextureParameter() const
{
	if (!FogOfWarMaterial || !DynamicTexture) {return;}
	FogOfWarMaterial->SetTextureParameterValue(*UFogOfWarComponentStatics::GetFogOfWarTextureParameterName(), DynamicTexture);
}

void UFogOfWarSubsystem::GetFogOfWarActorData(TArray<FIntPoint>& ActorPositions, TArray<FVector2f>& ActorVision, TArray<int32>& RadiusSqList) const
{
	const TOptional<FBox2D> ScreenBox{UFogOfWarComponentStatics::GetCameraFrustumGroundIntersections(this)};
	if (!ScreenBox.IsSet()) { return; }

	for (const TWeakObjectPtr<UFogOfWarComponent> WeakComponentPtr : FogOfWarComponents)
	{
		if (!WeakComponentPtr.IsValid())
		{
			bHasInvalidComponents = true;

			continue;
		}

		if (!WeakComponentPtr->IsActive()) { continue; }

		const UFogOfWarComponent* Component = WeakComponentPtr.Get();

		const TOptional<FFogOfWarData> Data{Component->GetFogOfWarData()};
		if (!Data.IsSet()) { continue; }
		const TOptional<FGridSizeType> GridSize{UFogOfWarComponentStatics::GetGridSize(EGridType::World, this)};
		const TOptional<FGridSizeType> ScreenGridSize{ UFogOfWarComponentStatics::GetGridSize(EGridType::Screen, this) };
		const TOptional<FBox2D> LandBoundingBox{UFogOfWarComponentStatics::GetLandBoundingBox(this)};
		const TOptional<FIntPoint> ScreenSize{ GetScreenSize() };
		if (!GridSize.IsSet() || !ScreenGridSize.IsSet() || !LandBoundingBox.IsSet() || !ScreenSize.IsSet()) { return; }

		const TOptional<FIntPoint> ActorPositionOnScreen{UFogOfWarComponentStatics::GetGridPositionOnScreen(Data.GetValue().ActorLocation, this, EAllowMinusPosition::Yes)};
		if (!ActorPositionOnScreen.IsSet()) {continue;}
		ActorPositions.Add(ActorPositionOnScreen.GetValue());
		ActorVision.Add(FVector2f{UFogOfWarComponentStatics::ProjectWorldDirectionToScreen(Data.GetValue().ActorVisionLeft)});
		ActorVision.Add(FVector2f{UFogOfWarComponentStatics::ProjectWorldDirectionToScreen(Data.GetValue().ActorVisionRight)});
		RadiusSqList.Add(FMath::CeilToInt32(FMath::Pow(Data.GetValue().Radius / ScreenGridSize.GetValue().GetGridSizeOnScreenCoordinate().X, 2.f)));
	}
}

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

void UFogOfWarSubsystem::SetComputeShaderOutputTextureCache(FRDGTextureRef& ShaderOutputTexture, FFogOfWarComputeShader::FParameters& Parameter, FRDGBuilder& GraphBuilder, const bool bCreateNewOne)
{
	if (!ShaderOutputTexture /*|| !CachedOutputTexture.IsValid() */)
	{
		const TOptional<FIntPoint> ScreenSize{ GetScreenSize() };
		if (!ScreenSize.IsSet()) { return; }

		FRDGTextureDesc TextureDesc = FRDGTextureDesc::Create2D(
			ScreenSize.GetValue(),
			UFogOfWarComponentStatics::GetFogOfWarTexturePixelFormat(),
			FClearValueBinding::Black,
			TexCreate_ShaderResource | TexCreate_UAV
		);
		ShaderOutputTexture = GraphBuilder.CreateTexture(TextureDesc, TEXT("OutputTexture"));
	}

	Parameter.OutputTexture = GraphBuilder.CreateUAV(ShaderOutputTexture);
}

TOptional<FIntPoint> UFogOfWarSubsystem::ProjectWorldToLand(const FVector2D& WorldLocation, const FBox2D& LandBoundingBox) const
{
	const TOptional<FIntPoint> ScreenSize{ GetScreenSize() };
	if (!ScreenSize.IsSet()) { return NullOpt; }

	const TOptional<FBox2D> ScreenAABB {UFogOfWarComponentStatics::GetCameraFrustumGroundIntersections(this)};
	if (!ScreenAABB.IsSet()) { return NullOpt; }

	// 3. 检查交集包围盒是否有效（防止除以零或负尺寸）
	const FVector2D BoxSize = ScreenAABB.GetValue().GetSize();
	if (BoxSize.X <= 0.0f || BoxSize.Y <= 0.0f) { return NullOpt; }

	// 4. 计算网格坐标
	const FVector2D NormalizedPos = (WorldLocation - ScreenAABB.GetValue().Min) / BoxSize;
	const FIntPoint GridPos{
		FMath::FloorToInt32(NormalizedPos.X * ScreenSize.GetValue().X),
		FMath::FloorToInt32(NormalizedPos.Y * ScreenSize.GetValue().Y)
	};

	return GridPos;
}

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

	bIsInitialScale = true;
	OnViewportSizeChangedDelegate.Broadcast();
}

void UFogOfWarSubsystem::OnViewportResized(FViewport* Viewport, uint32 Unused)
{
	bViewportResized = true;
	SetupScaleFactor();
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