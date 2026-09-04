// Fill out your copyright notice in the Description page of Project Settings.


#include "TopDownCameraSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/WorldSettings.h"
#include "TopDownCameraActor.h"
#include "Camera/CameraComponent.h"
#include "CameraBoundsVolume.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/GameViewportClient.h"
#include "UnrealClient.h"
#include "Widgets/SViewport.h"
#include "GameFramework/GameModeBase.h"
#include "TopDownCameraSubsystemProviderInterface.h"
#include "Configs/TopDownCameraSettings.h"

bool UTopDownCameraSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer)) { return false; }

	const UWorld* World{ Cast<UWorld>(Outer) };
	const AWorldSettings* WorldSettings{ World && World->IsGameWorld() ? World->GetWorldSettings() : nullptr };
	const TSubclassOf<AGameModeBase> DefaultGameMode{ WorldSettings ? WorldSettings->DefaultGameMode : nullptr };
	if (const UObject* GameMode{ DefaultGameMode ? DefaultGameMode->GetDefaultObject() : nullptr }; GameMode && GameMode->Implements<UTopDownCameraSubsystemProviderInterface>())
	{
		return ITopDownCameraSubsystemProviderInterface::Execute_ShouldCreateTopDownCameraSubsystem(GameMode);
	}
	return false;
}

void UTopDownCameraSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (APlayerController* PC{ GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr })
	{
		SetupCameraForPlayerController(PC);
	}
	else{ FGameModeEvents::OnGameModePostLoginEvent().AddUObject(this, &UTopDownCameraSubsystem::OnGameModePostLogin); }

	InitializeViewportInfo();
}

void UTopDownCameraSubsystem::Deinitialize()
{
	Super::Deinitialize();

	if (APlayerController * PlayerController{ GetWorld()->GetFirstPlayerController() })
	{
		PlayerController->OnPossessedPawnChanged.RemoveAll(this);
	}

	if (FViewport * Viewport{ ViewportInfo.ViewportClient.IsValid() ? ViewportInfo.ViewportClient->Viewport : nullptr })
	{
		Viewport->ViewportResizedEvent.RemoveAll(this);
	}
}

const ACameraBoundsVolume* UTopDownCameraSubsystem::GetCameraBoundsVolume()
{
	if (!CameraBoundsVolume.IsValid())
	{
		CameraBoundsVolume = Cast<ACameraBoundsVolume>(UGameplayStatics::GetActorOfClass(GetWorld(), ACameraBoundsVolume::StaticClass()));
	}

	return CameraBoundsVolume.Get();
}

void UTopDownCameraSubsystem::CameraMoveTo(const FVector& TargetLocation)
{
	if (CameraActor) 
	{
		CameraActor->SetForceTarget(FVector{ TargetLocation.X, TargetLocation.Y, CameraActor->GetActorLocation().Z }, EForceMovementType::Interpolate);
	}
}

// void UTopDownCameraSubsystem::SetCameraWidth(const float Width)
// {
// 	CameraActor->GetCameraComponent()->SetOrthoWidth(Width);
// }

void UTopDownCameraSubsystem::InitializeViewportInfo()
{
	if (!ViewportInfo.ViewportClient.IsValid()) { ViewportInfo.ViewportClient = GetWorld() ? GetWorld()->GetGameViewport() : nullptr; }

	FViewport* Viewport{ ViewportInfo.ViewportClient.IsValid() ? ViewportInfo.ViewportClient->Viewport : nullptr };
	if (!Viewport) { return; }
	if (!Viewport->ViewportResizedEvent.IsBoundToObject(this)) { Viewport->ViewportResizedEvent.AddUObject(this, &UTopDownCameraSubsystem::OnViewportResized); }

	FVector2D ViewportSize;
	ViewportInfo.ViewportClient->GetViewportSize(ViewportSize);
	if (ViewportSize.IsNearlyZero()) { return; }

	ViewportInfo.ScreenSize = ViewportSize;
}

void UTopDownCameraSubsystem::OnViewportResized(FViewport* Viewport, uint32 Unused)
{
	ViewportInfo.ScreenSize.Reset();

	InitializeViewportInfo();
}

void UTopDownCameraSubsystem::OnPossessedPawnChanged(APawn* InOldPawn, APawn* InNewPawn)
{
	if (APlayerController* PlayerController{ GetWorld()->GetFirstPlayerController() }; PlayerController && CameraActor)
	{
		PlayerController->SetViewTarget(CameraActor);
		if (InNewPawn)
		{
			const FVector TargetLocation{ InNewPawn->GetActorLocation().X, InNewPawn->GetActorLocation().Y, CameraActor->GetActorLocation().Z };
			CameraActor->SetForceTarget(TargetLocation, EForceMovementType::Interpolate);
		}
	}
}

void UTopDownCameraSubsystem::SetupCameraForPlayerController(APlayerController* NewPlayerController)
{
	if (!NewPlayerController) { return; }

	const UTopDownCameraSettings* TopDownCameraSettings{ GetDefault<UTopDownCameraSettings>() };
	if (!TopDownCameraSettings) { return; }

	if (!CameraActor)
	{
		const FTransform CameraTransform{ FRotator{ -90.f, 0.f, 0.f }.Quaternion(), FVector::ZAxisVector * 2000.f };
		CameraActor = GetWorld()->SpawnActorDeferred<ATopDownCameraActor>(ATopDownCameraActor::StaticClass(), CameraTransform);
		check(CameraActor);

		if (UCameraComponent * CameraComponent{ CameraActor->GetCameraComponent() })
		{
			CameraComponent->PrimaryComponentTick.bCanEverTick = false;
			CameraComponent->ProjectionMode = ECameraProjectionMode::Orthographic;
			CameraComponent->OrthoWidth = TopDownCameraSettings->OrthoWidth;
			CameraComponent->bConstrainAspectRatio = false;
			CameraComponent->SetAutoCalculateOrthoPlanes(false);
			CameraComponent->SetOrthoNearClipPlane(TopDownCameraSettings->OrthoNearClipPlane);
			CameraComponent->SetOrthoFarClipPlane(TopDownCameraSettings->OrthoFarClipPlane);
		}

		CameraActor->FinishSpawning(CameraTransform);
	}

	if (APlayerController* PlayerController{ GetWorld()->GetFirstPlayerController() }; PlayerController && CameraActor)
	{
		PlayerController->SetViewTarget(CameraActor);
		InitializeViewportInfo();
		PlayerController->OnPossessedPawnChanged.AddUniqueDynamic(this, &UTopDownCameraSubsystem::OnPossessedPawnChanged);
		if (const APawn* ControlledPawn{  PlayerController->GetPawn() })
		{
			const FVector TargetLocation{ ControlledPawn->GetActorLocation().X, ControlledPawn->GetActorLocation().Y, CameraActor->GetActorLocation().Z };
			CameraActor->SetForceTarget(TargetLocation, EForceMovementType::Interpolate);
		}
	}
}

void UTopDownCameraSubsystem::Tick(float DeltaTime)
{
	if (!CameraActor) { SetupCameraForPlayerController(GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr); }
	if (!ViewportInfo.IsSet()) { InitializeViewportInfo(); }

	if (!CameraActor || !ViewportInfo.IsSet()) { return; }

	const ACameraBoundsVolume* Volume{ GetCameraBoundsVolume() };
	if (!Volume) { return; }

	auto GameViewport{GEngine->GameViewport};
	const TSharedPtr<SViewport> ViewportWidget = GameViewport? GameViewport->GetGameViewportWidget() : nullptr;
	if (!ViewportWidget.IsValid()) { return;}
	const FVector2D MousePos = FSlateApplication::Get().GetCursorPos();
	// 2. 使用 FGeometry 自带的 IsUnderLocation 判定绝对坐标是否在 Widget 内部
	if (!ViewportWidget->GetCachedGeometry().IsUnderLocation(MousePos)) {return;}

	FVector2D MousePosition;
	ViewportInfo.ViewportClient->GetMousePosition(MousePosition);

	const FVector2D& ViewportSize{ ViewportInfo.ScreenSize.GetValue() };

	// // 1. 初始化一个二维移动输入向量 (X: 前后, Y: 左右)
	FVector2D MoveInput = FVector2D::ZeroVector;

	// 2. 精确判断鼠标触发了哪个边缘（支持对角线同时移动）
	// 靠近左边缘 -> 向左
	if (MousePosition.X > 0. && !FMath::IsNearlyEqual(MousePosition.X, 0.) && MousePosition.X < ViewportSize.X * (1.f - kThreshold))
	{
		MoveInput.Y = -1.f;
	}
	// 靠近右边缘 -> 向右
	else if (MousePosition.X > ViewportSize.X * kThreshold)
	{
		MoveInput.Y = 1.f;
	}

	// 靠近上边缘 -> 向前
	if (MousePosition.Y > 0. && !FMath::IsNearlyEqual(MousePosition.Y, 0.) && MousePosition.Y < ViewportSize.Y * (1.f - kThreshold))
	{
		MoveInput.X = 1.f;
	}
	// 靠近下边缘 -> 向后
	else if (MousePosition.Y > ViewportSize.Y * kThreshold)
	{
		MoveInput.X = -1.f;
	}

	// 3. 如果有任意边缘触发，执行移动
	if (!MoveInput.IsNearlyZero())
	{
		// 归一化输入，防止斜向移动时速度变成等比的 1.414 倍
		MoveInput.Normalize();

		// 4. 计算属于当前摄像机水平视角的“前”和“右”向量
		// 获取摄像机当前的旋转，但抹平 Pitch 和 Roll，只保留 Yaw（确保平行于地面移动，不往地下钻）
		const FRotator FrameRotation{ 0.f, CameraActor->GetActorRotation().Yaw, 0.f };
		const FVector ForwardDirection{ FRotationMatrix(FrameRotation).GetUnitAxis(EAxis::X) };
		const FVector RightDirection{ FRotationMatrix(FrameRotation).GetUnitAxis(EAxis::Y) };

		// 5. 组合最终的世界坐标移动向量
		const FVector WorldMoveDirection{ (ForwardDirection * MoveInput.X) + (RightDirection * MoveInput.Y) };

		// 6. 应用移动 (MoveSpeed 为你的摄像机移动速度，例如 1200.f)
		FVector NewLocation{ CameraActor->GetActorLocation() + (WorldMoveDirection * kSpeed * DeltaTime) };

		{
			const FBox Bounds{ Volume->GetBounds().GetBox() };
			NewLocation.X = FMath::Max(Bounds.Min.X, NewLocation.X);
			NewLocation.Y = FMath::Max(Bounds.Min.Y, NewLocation.Y);
			NewLocation.X = FMath::Min(Bounds.Max.X, NewLocation.X);
			NewLocation.Y = FMath::Min(Bounds.Max.Y, NewLocation.Y);
		}

		CameraActor->SetActorLocation(NewLocation);
	}
}

void UTopDownCameraSubsystem::OnGameModePostLogin(AGameModeBase* GameMode, APlayerController* NewPlayer)
{
    // 确保是当前 World 内的 PlayerController
    if (NewPlayer && NewPlayer->GetWorld() == GetWorld())
    {
        SetupCameraForPlayerController(NewPlayer);
    }
}