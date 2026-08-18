// Fill out your copyright notice in the Description page of Project Settings.
#include "FogOfWarComponentStatics.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetMathLibrary.h"
#include <cmath>
#include "WorldHeightSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "FogOfWarSubsystem.h"
#include "Components/SceneComponent.h"

#if !FOGOFWAR_INLINE_ENABLED
#define FOGOFWAR_ALLOW_INCLUDE_INL
#include "FogOfWarComponentStatics.inl"
#undef FOGOFWAR_ALLOW_INCLUDE_INL
#endif

FGridBoundsDataType UFogOfWarComponentStatics::MakeGridBoundsTypeFromActor(const AActor& Actor)
{
	return FGridBoundsDataType{ Actor.GetComponentsBoundingBox(true) };
}

TOptional<FGridSizeType> UFogOfWarComponentStatics::GetGridSize(const EGridType GridType, const UObject* WorldContextObject)
{
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World) { return NullOpt; }
	
	if (GridType == EGridType::World)
	{
		const UWorldHeightSubsystem* WorldHeightSubsystem{ World->GetSubsystem<UWorldHeightSubsystem>() };
		return WorldHeightSubsystem ? WorldHeightSubsystem->GetGridSize() : NullOpt;
	}
	else if (GridType == EGridType::Screen)
	{
		const ULocalPlayer* LocalPlayer{ World->GetFirstLocalPlayerFromController() };
		const UFogOfWarSubsystem* FogOfWarSubsystem{ LocalPlayer ? LocalPlayer->GetSubsystem<UFogOfWarSubsystem>(): nullptr };

		return FogOfWarSubsystem ? FogOfWarSubsystem->GetGridSize() : NullOpt;
	}
	
	return NullOpt;
}

TOptional<FIntPoint> UFogOfWarComponentStatics::GetScreenSize(const UObject* WorldContextObject)
{
	return GetFogOfWarSubsystem(WorldContextObject) ? GetFogOfWarSubsystem(WorldContextObject)->GetScreenSize() : NullOpt;
}

// FVector UFogOfWarComponentStatics::GetIntersectionFromCameraToGround(const UObject* WorldContextObject)
// {
// 	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
// 	if (!World) { return FogOfWarConst::kInvalidVector; }

// 	const APlayerController* PlayerController = World->GetFirstPlayerController();
// 	if (!PlayerController) { return FogOfWarConst::kInvalidVector; }

// 	FVector CameraLocation;
// 	FRotator CameraRotation;
// 	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

// 	//地面的法向量
// 	const FVector NormalOfGround = FVector::UpVector;

// 	return CameraLocation + CameraRotation.Vector() * (-CameraLocation.Z / CameraRotation.Vector().Z);
// }

TOptional<FBox2D> UFogOfWarComponentStatics::GetCameraFrustumGroundIntersections(const UObject* WorldContextObject, const double GroundHeight)
{
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	const APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	if (!PC) { return NullOpt; }

	// 1. 获取当前视口（屏幕）的尺寸（像素）
	int32 SizeX, SizeY;
	PC->GetViewportSize(SizeX, SizeY);
	if (SizeX <= 0 || SizeY <= 0) { return NullOpt; }

	// 2. 定义屏幕的四个角点（像素坐标）
	// 顺序：左上 -> 右上 -> 右下 -> 左下
	const TArray<FVector2D> ScreenCorners{FVector2D(0.f, SizeY), FVector2D(SizeX, 0.f)};
	TArray<FVector2D> Result;
	// 3. 循环处理四个角
	for (const FVector2D& ScreenPos : ScreenCorners)
	{
		FVector RayOrigin;
		FVector RayDirection;

		// 核心函数：将屏幕像素坐标转换为世界空间中的 射线起点 和 单位方向向量
		if (UGameplayStatics::DeprojectScreenToWorld(PC, ScreenPos, RayOrigin, RayDirection))
		{
			// 4. 使用之前推导的水平面求交简化公式
			// 确保射线是在向下看（指向地面）
			if (RayDirection.Z < -0.001f)
			{
				// 计算射线达到地面高度所需的步长 t
				const double t{ (GroundHeight - RayOrigin.Z) / RayDirection.Z };

				// 计算精确的世界坐标交点
				const FVector IntersectionPoint{ RayOrigin + RayDirection * t };

				Result.Add(FVector2D{ IntersectionPoint });
			}
			else
			{
				// 如果镜头抬得太高，某些角发出的射线可能会射向天空，此时无法与地面相交
				// 你可以根据业务需求决定如何处理这些“朝天”的边界点
				UE_LOG(LogTemp, Warning, TEXT("射线未射向地面，可能相机仰角过大。"));
			}
		}
	}

	return Result.Num() == 2 ? TOptional<FBox2D>{ Result } : NullOpt;
}

TOptional<FBox2D> UFogOfWarComponentStatics::GetLandBoundingBox(const UObject* WorldContextObject)
{
	return GetWorldHeightSubsystem(WorldContextObject) ? GetWorldHeightSubsystem(WorldContextObject)->GetLandBoundingBox() : NullOpt;
}

bool UFogOfWarComponentStatics::GetActorOrientBox(FOrientedBox& OrientedBox, const AActor* Actor)
{
	if (!Actor) { return false; }

	OrientedBox.Center = Actor->GetActorLocation();
	OrientedBox.AxisX = Actor->GetActorForwardVector();
	OrientedBox.AxisY = Actor->GetActorRightVector();
	OrientedBox.AxisZ = Actor->GetActorUpVector();

	USceneComponent* RootComponent = Actor->GetRootComponent();
	// 1. 获取组件在“未应用任何世界变换（即局部空间）”下的原始 Bounding Box
	// 很多底层组件（如 Mesh, Shape）都会重写这个函数来返回自己最原始的盒体大小
	FTransform Transform{ FTransform::Identity };
	Transform.SetScale3D(Actor->GetActorScale3D());
	FBox LocalBox = RootComponent->CalcBounds(Transform).GetBox();

	OrientedBox.ExtentX = FMath::Abs(LocalBox.GetExtent().X);
	OrientedBox.ExtentY = FMath::Abs(LocalBox.GetExtent().Y);
	OrientedBox.ExtentZ = FMath::Abs(LocalBox.GetExtent().Z);

	return true;
}

bool UFogOfWarComponentStatics::GetComponentOrientBox(FOrientedBox& OrientedBox, const USceneComponent* Component)
{
	if (!Component) { return false; }

	OrientedBox.Center = Component->GetComponentLocation();
	OrientedBox.AxisX = Component->GetForwardVector();
	OrientedBox.AxisY = Component->GetRightVector();
	OrientedBox.AxisZ = Component->GetUpVector();

	FTransform Transform{ FTransform::Identity };
	Transform.SetScale3D(Component->GetComponentScale());
	FBox LocalBox = Component->CalcBounds(Transform).GetBox();

	OrientedBox.ExtentX = FMath::Abs(LocalBox.GetExtent().X);
	OrientedBox.ExtentY = FMath::Abs(LocalBox.GetExtent().Y);
	OrientedBox.ExtentZ = FMath::Abs(LocalBox.GetExtent().Z);

	return true;
}

TOptional<FIntPoint> UFogOfWarComponentStatics::GetGridPositionOnScreen(const FVector& WorldLocation, const UObject* WorldContextObject, const EAllowMinusPosition AllowMinusPosition)
{
	if (!WorldContextObject) {return NullOpt;}

	const TOptional<FIntPoint> ScreenSize{GetScreenSize(WorldContextObject)};
	const TOptional<FBox2D> ScreenBoundingBox{GetCameraFrustumGroundIntersections(WorldContextObject)};
	const TOptional<FBox2D> LandBoundingBox{UFogOfWarComponentStatics::GetLandBoundingBox(WorldContextObject)};

	if (!ScreenSize.IsSet() || !ScreenBoundingBox.IsSet() || !LandBoundingBox.IsSet()) {return NullOpt;}
	return GetGridPositionOnScreen(WorldLocation, ScreenSize.GetValue(), ScreenBoundingBox.GetValue(), LandBoundingBox.GetValue(), AllowMinusPosition);
}

FogOfWarTypes::GridIndexType UFogOfWarComponentStatics::GetGridIndexOnScreen(const FVector& WorldLocation, const UObject* WorldContextObject)
{
	if (!WorldContextObject) {return INDEX_NONE;}

	const TOptional<FIntPoint> ScreenSize{GetScreenSize(WorldContextObject)};
	const TOptional<FBox2D> ScreenBoundingBox{GetCameraFrustumGroundIntersections(WorldContextObject)};
	const TOptional<FBox2D> LandBoundingBox{UFogOfWarComponentStatics::GetLandBoundingBox(WorldContextObject)};

	if (!ScreenSize.IsSet() || !ScreenBoundingBox.IsSet() || !LandBoundingBox.IsSet()) {return INDEX_NONE;}
	return GetGridIndexOnScreen(WorldLocation, ScreenSize.GetValue(), ScreenBoundingBox.GetValue(), LandBoundingBox.GetValue());
}

TOptional<FIntPoint> UFogOfWarComponentStatics::GetGridPosition(const FVector& WorldLocation, const UObject* WorldContextObject, const EAllowMinusPosition AllowMinusPosition)
{
	return GetWorldHeightSubsystem(WorldContextObject) ? GetWorldHeightSubsystem(WorldContextObject)->GetGridPosition(WorldLocation, AllowMinusPosition) : NullOpt;
}

FogOfWarTypes::GridIndexType UFogOfWarComponentStatics::GetGridIndexOnWorld(const FVector& WorldLocation, const UObject* WorldContextObject, const EAllowMinusPosition AllowMinusPosition)
{
	return GetWorldHeightSubsystem(WorldContextObject) ? GetGridIndexOnWorld(WorldLocation, *GetWorldHeightSubsystem(WorldContextObject), AllowMinusPosition) : INDEX_NONE;
}

FogOfWarTypes::GridIndexType UFogOfWarComponentStatics::GetGridIndexOnWorld(const FVector& WorldLocation, const UWorldHeightSubsystem& WorldHeightSubsystem, const EAllowMinusPosition AllowMinusPosition)
{
	return WorldHeightSubsystem.GetGridIndex(WorldLocation, AllowMinusPosition);
}

TOptional<FIntPoint> UFogOfWarComponentStatics::GetGridPositionOnWorld(const FVector& WorldLocation, const UObject* WorldContextObject, const EAllowMinusPosition AllowMinusPosition)
{
	return GetWorldHeightSubsystem(WorldContextObject) ? GetGridPositionOnWorld(WorldLocation, *GetWorldHeightSubsystem(WorldContextObject), AllowMinusPosition) : NullOpt;
}

TOptional<FIntPoint> UFogOfWarComponentStatics::GetGridPositionOnWorld(const FVector& WorldLocation, const UWorldHeightSubsystem& WorldHeightSubsystem, const EAllowMinusPosition AllowMinusPosition)
{
	return WorldHeightSubsystem.GetGridPosition(WorldLocation, AllowMinusPosition);
}

TOptional<FIntPoint> UFogOfWarComponentStatics::GetGridPositionOnWorld(const FVector2D& WorldLocation, const UObject* WorldContextObject, const EAllowMinusPosition AllowMinusPosition)
{
	return GetWorldHeightSubsystem(WorldContextObject) ? GetGridPositionOnWorld(WorldLocation, *GetWorldHeightSubsystem(WorldContextObject), AllowMinusPosition) : NullOpt;
}

TOptional<FIntPoint> UFogOfWarComponentStatics::GetGridPositionOnWorld(const FVector2D& WorldLocation, const UWorldHeightSubsystem& WorldHeightSubsystem, const EAllowMinusPosition AllowMinusPosition)
{
	return WorldHeightSubsystem.GetGridPosition(WorldLocation, AllowMinusPosition);
}

UFogOfWarSubsystem* UFogOfWarComponentStatics::GetFogOfWarSubsystem(const UObject* WorldContextObject)
{
	const UWorld* World{GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull)};
	const ULocalPlayer* LocalPlayer{ World ? World->GetFirstLocalPlayerFromController() : nullptr };
	return LocalPlayer ? LocalPlayer->GetSubsystem<UFogOfWarSubsystem>() : nullptr;
}

UWorldHeightSubsystem* UFogOfWarComponentStatics::GetWorldHeightSubsystem(const UObject* WorldContextObject)
{
	const UWorld* World{GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull)};
	return World ? World->GetSubsystem<UWorldHeightSubsystem>() : nullptr;
}

bool UFogOfWarComponentStatics::IsPrime(NumberType N)
{
	if (N == 2 || N == 3) { return true; }
	if (N < 4) { return false; }

	return IsPrime(UKismetMathLibrary::RandomIntegerInRange(2, N - 2), N - 1, N);
}

bool UFogOfWarComponentStatics::IsPrime(NumberType A, NumberType i, NumberType N)
{
	if (N == 2 || N == 3) { return true; }
	if (N < 4) { return false; }

	return Witness(A, i, N) == 1;
}

bool UFogOfWarComponentStatics::IsPrimeMillerRabinTest(NumberType N, const int8 IterationNum)
{
	bool bPrime{ false };

	for (std::remove_const<decltype(IterationNum)>::type i = 0; i < IterationNum; i++)
	{
		bPrime = IsPrime(N);
		if (!bPrime) { return false; }
	}

	return bPrime;
}

UFogOfWarComponentStatics::NumberType UFogOfWarComponentStatics::Witness(NumberType A, NumberType i, NumberType N)
{
	if (N == 2 || N == 3) { return 1; }
	if (N < 4) { return 0; }

	NumberType x, y;

	if (i == 0) { return 1; }

	x = Witness(A, i / 2, N);
	if (x == 0) { return 0; }

	y = (x * x) % N;
	if (y == 1 && x != 1 && x != N - 1) { return 0; }

	if (i % 2 != 0) { y = (A * y) % N; }

	return y;
}