// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraSubsystem.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "CameraBoundsVolume.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

void UCameraSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]() {
		
		}));
}

void UCameraSubsystem::Deinitialize()
{
	Super::Deinitialize();

	if (APlayerController * PlayerController{ GetWorld()->GetFirstPlayerController() })
	{
		PlayerController->OnPossessedPawnChanged.RemoveAll(this);
	}
}

void UCameraSubsystem::Tick(float DeltaTime)
{
	if (!CameraActor) { return; }

	const ACameraBoundsVolume* Volume{ GetCameraBoundsVolume() };
	if (!Volume) { return; }

	const UGameViewportClient* ViewportClient{ GetWorld()->GetGameViewport() };
	if (!ViewportClient) { return; }

	FVector2D ViewportSize;
	FVector2D MousePosition;
	ViewportClient->GetViewportSize(ViewportSize);
	ViewportClient->GetMousePosition(MousePosition);

	// 1. 初始化一个二维移动输入向量 (X: 前后, Y: 左右)
	FVector2D MoveInput = FVector2D::ZeroVector;

	// 2. 精确判断鼠标触发了哪个边缘（支持对角线同时移动）
	// 靠近左边缘 -> 向左
	if (MousePosition.X > 0. && !FMath::IsNearlyEqual(MousePosition.X, 0.) && MousePosition.X < ViewportSize.X * (1.f - kThreshold))
	{
		MoveInput.Y = -1.f;
	}
	// 靠近右边缘 -> 向右 (这里用 ViewportSize.X - MousePosition.X 替代 Abs 更直观安全)
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

const ACameraBoundsVolume* UCameraSubsystem::GetCameraBoundsVolume()
{
	if (!CameraBoundsVolume.IsValid())
	{
		CameraBoundsVolume = Cast<ACameraBoundsVolume>(UGameplayStatics::GetActorOfClass(GetWorld(), ACameraBoundsVolume::StaticClass()));
	}

	return CameraBoundsVolume.Get();
}

void UCameraSubsystem::OnPossessedPawnChanged(APawn* InOldPawn, APawn* InNewPawn)
{
	if (APlayerController* PlayerController{ GetWorld()->GetFirstPlayerController() }; PlayerController && CameraActor)
	{
		PlayerController->SetViewTarget(CameraActor);
	}
}

void UCameraSubsystem::PlayerControllerChanged(APlayerController* NewPlayerController)
{
	Super::PlayerControllerChanged(NewPlayerController);

	const FTransform CameraTransform{ FRotator{ -90.f, 0.f, 0.f }.Quaternion(), FVector::ZAxisVector * 2000.f };
	CameraActor = GetWorld()->SpawnActorDeferred<ACameraActor>(ACameraActor::StaticClass(), CameraTransform);
	check(CameraActor);

	if (UCameraComponent * CameraComponent{ CameraActor->GetCameraComponent() })
	{
		CameraComponent->PrimaryComponentTick.bCanEverTick = false;
		CameraComponent->ProjectionMode = ECameraProjectionMode::Orthographic;
		CameraComponent->OrthoWidth = 1000.f;
		CameraComponent->bConstrainAspectRatio = false;
		CameraComponent->SetAutoCalculateOrthoPlanes(false);
		CameraComponent->SetOrthoNearClipPlane(-5000.f);
		CameraComponent->SetOrthoFarClipPlane(1e7);
	}

	CameraActor->FinishSpawning(CameraTransform);

	if (APlayerController* PlayerController{ GetWorld()->GetFirstPlayerController() }; PlayerController && CameraActor)
	{
		PlayerController->SetViewTarget(CameraActor);
		PlayerController->OnPossessedPawnChanged.AddUniqueDynamic(this, &UCameraSubsystem::OnPossessedPawnChanged);
	}

	//if (NewPlayerController && CameraActor)
	//{
	//	// 1. 此时 Controller 绝对完全初始化好了，果断切镜头
	//	NewPlayerController->SetViewTarget(CameraActor);

	//	// 2. 顺便绑定你的附身改变事件，防止未来切换 Pawn 时镜头被洗掉
	//	NewPlayerController->OnPossessedPawnChanged.AddUniqueDynamic(this, &UCameraSubsystem::OnPossessedPawnChanged);
	//}
}

//void UCameraSubsystem::OnPlayerControllerSet(ULocalPlayer* InLocalPlayer, APlayerController* InPlayerController)
//{
//	if (InPlayerController && CameraActor)
//	{
//		InPlayerController->SetViewTargetWithBlend(CameraActor);
//	}
//}