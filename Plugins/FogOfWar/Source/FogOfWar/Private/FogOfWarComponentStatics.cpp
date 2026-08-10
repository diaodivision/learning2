// Fill out your copyright notice in the Description page of Project Settings.
#include "FogOfWarComponentStatics.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetMathLibrary.h"
#include <cmath>
#include "WorldHeightSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "FogOfWarSubsystem.h"
#include "Components/SceneComponent.h"

bool UFogOfWarComponentStatics::IsActorInsideCube(const AActor* Cube, const AActor* Actor)
{
	if (!Cube || !Actor) { return false; }

	FOrientedBox CubeObb;
	{
		CubeObb.Center = Cube->GetActorLocation();
		CubeObb.AxisX = Cube->GetActorForwardVector();
		CubeObb.AxisY = Cube->GetActorRightVector();
		CubeObb.AxisZ = Cube->GetActorUpVector();

		FVector CubeOrigin, CubeExtent;
		Cube->GetActorBounds(false, CubeOrigin, CubeExtent);
		CubeObb.Center = CubeOrigin;

		USceneComponent* RootComp = Cube->GetRootComponent();
		// 1. 获取组件在“未应用任何世界变换（即局部空间）”下的原始 Bounding Box
		// 很多底层组件（如 Mesh, Shape）都会重写这个函数来返回自己最原始的盒体大小
		FTransform Transform{ FTransform::Identity };
		Transform.SetScale3D(Cube->GetActorScale3D());
		FBox LocalBox = RootComp->CalcBounds(Transform).GetBox();


		CubeObb.ExtentX = FMath::Abs(LocalBox.GetExtent().X);
		CubeObb.ExtentY = FMath::Abs(LocalBox.GetExtent().Y);
		CubeObb.ExtentZ = FMath::Abs(LocalBox.GetExtent().Z);
	}

	const FVector Direction{ Actor->GetActorLocation() - CubeObb.Center };
	const bool bConditionX{ FMath::Abs(FVector::DotProduct(Direction, CubeObb.AxisX)) <= CubeObb.ExtentX };
	const bool bConditionY{ FMath::Abs(FVector::DotProduct(Direction, CubeObb.AxisY)) <= CubeObb.ExtentY };
	const bool bConditionZ{ FMath::Abs(FVector::DotProduct(Direction, CubeObb.AxisZ)) <= CubeObb.ExtentZ };
	UE_LOG(LogTemp, Error, TEXT("UFogOfWarComponentStatics::IsActorInsideCube Direction %s"), *Direction.ToString());
	UE_LOG(LogTemp, Error, TEXT("UFogOfWarComponentStatics::IsActorInsideCube CubeObb.AxisX %s"), *CubeObb.AxisX.ToString());
	UE_LOG(LogTemp, Error, TEXT("UFogOfWarComponentStatics::IsActorInsideCube CubeObb.AxisY %s"), *CubeObb.AxisY.ToString());
	UE_LOG(LogTemp, Error, TEXT("UFogOfWarComponentStatics::IsActorInsideCube CubeObb.AxisZ %s"), *CubeObb.AxisZ.ToString());
	UE_LOG(LogTemp, Error, TEXT("UFogOfWarComponentStatics::IsActorInsideCube FVector::DotProduct(Direction, CubeObb.AxisX) %f"), FVector::DotProduct(Direction, CubeObb.AxisX));
	UE_LOG(LogTemp, Error, TEXT("UFogOfWarComponentStatics::IsActorInsideCube FVector::DotProduct(Direction, CubeObb.AxisY) %f"), FVector::DotProduct(Direction, CubeObb.AxisY));
	UE_LOG(LogTemp, Error, TEXT("UFogOfWarComponentStatics::IsActorInsideCube FVector::DotProduct(Direction, CubeObb.AxisZ) %f"), FVector::DotProduct(Direction, CubeObb.AxisZ));
	UE_LOG(LogTemp, Error, TEXT("UFogOfWarComponentStatics::IsActorInsideCube CubeObb.ExtentX %f"), CubeObb.ExtentX);
	UE_LOG(LogTemp, Error, TEXT("UFogOfWarComponentStatics::IsActorInsideCube CubeObb.ExtentY %f"), CubeObb.ExtentY);
	UE_LOG(LogTemp, Error, TEXT("UFogOfWarComponentStatics::IsActorInsideCube CubeObb.ExtentZ %f"), CubeObb.ExtentZ);

	return bConditionX && bConditionY && bConditionZ;
}

//bool UFogOfWarComponentStatics::IsActorInsideCube(const AActor* Cube, const AActor* Actor)
//{
//	if (!Actor || !Cube) return false;
//
//	// 1. 构建 Cube 的 OBB 数据
//	FOrientedBox Obb;
//	Obb.Center = Cube->GetActorLocation();
//	Obb.AxisX = Cube->GetActorForwardVector();
//	Obb.AxisY = Cube->GetActorRightVector();
//	Obb.AxisZ = Cube->GetActorUpVector();
//
//
//	FVector Origin, Extents;
//	Cube->GetActorBounds(false, Origin, Extents);
//	Obb.ExtentX = Extents.X;
//	Obb.ExtentY = Extents.Y;
//	Obb.ExtentZ = Extents.Z;
//
//	// 2. 获取目标 Actor (网格/角色) 自身的世界包围盒大小 (AABB Extent)
//	FVector ActorOrigin, ActorExtent;
//	Actor->GetActorBounds(false, ActorOrigin, ActorExtent);
//
//	UE_LOG(LogTemp, Error, TEXT("UFogOfWarComponentStatics::IsActorInsideCube Extents %s"), *Extents.ToString());
//	UE_LOG(LogTemp, Error, TEXT("UFogOfWarComponentStatics::IsActorInsideCube ActorExtent %s"), *ActorExtent.ToString());
//	UE_LOG(LogTemp, Error, TEXT("UFogOfWarComponentStatics::IsActorInsideCube Obb.AxisX %s"), *Obb.AxisX.ToString());
//	UE_LOG(LogTemp, Error, TEXT("UFogOfWarComponentStatics::IsActorInsideCube Obb.AxisY %s"), *Obb.AxisY.ToString());
//	UE_LOG(LogTemp, Error, TEXT("UFogOfWarComponentStatics::IsActorInsideCube Obb.AxisZ %s"), *Obb.AxisZ.ToString());
//
//	// 3. 计算目标 Actor 在 Cube 三个局部轴上的投影半径
//	// 这一步能完美处理“未旋转的网格”与“任意旋转的Cube”相交时的边缘厚度
//	float ActorRadiusX = ActorExtent.X * FMath::Abs(Obb.AxisX.X) + ActorExtent.Y * FMath::Abs(Obb.AxisX.Y) + ActorExtent.Z * FMath::Abs(Obb.AxisX.Z);
//	float ActorRadiusY = ActorExtent.X * FMath::Abs(Obb.AxisY.X) + ActorExtent.Y * FMath::Abs(Obb.AxisY.Y) + ActorExtent.Z * FMath::Abs(Obb.AxisY.Z);
//	float ActorRadiusZ = ActorExtent.X * FMath::Abs(Obb.AxisZ.X) + ActorExtent.Y * FMath::Abs(Obb.AxisZ.Y) + ActorExtent.Z * FMath::Abs(Obb.AxisZ.Z);
//
//	// 4. 计算中心点相对位移
//	FVector Direction = Actor->GetActorLocation() - Obb.Center;
//	UE_LOG(LogTemp, Error, TEXT("UFogOfWarComponentStatics::IsActorInsideCube Direction | Obb.AxisX %f"), Direction | Obb.AxisX);
//	UE_LOG(LogTemp, Error, TEXT("UFogOfWarComponentStatics::IsActorInsideCube Direction | Obb.AxisY %f"), Direction | Obb.AxisY);
//	UE_LOG(LogTemp, Error, TEXT("UFogOfWarComponentStatics::IsActorInsideCube Direction | Obb.AxisZ %f"), Direction | Obb.AxisZ);
//	UE_LOG(LogTemp, Error, TEXT("UFogOfWarComponentStatics::IsActorInsideCube Obb.ExtentX + ActorRadiusX %f"), Obb.ExtentX + ActorRadiusX);
//	UE_LOG(LogTemp, Error, TEXT("UFogOfWarComponentStatics::IsActorInsideCube Obb.ExtentY + ActorRadiusY %f"), Obb.ExtentY + ActorRadiusY);
//	UE_LOG(LogTemp, Error, TEXT("UFogOfWarComponentStatics::IsActorInsideCube Obb.ExtentZ + ActorRadiusZ %f"), Obb.ExtentZ + ActorRadiusZ);
//
//	// 5. 核心修正：对比时，将 Cube 自身的 Extent 加上 Actor 的投影半径
//	bool bInsideX = FMath::Abs(Direction | Obb.AxisX) <= (Obb.ExtentX + ActorRadiusX);
//	bool bInsideY = FMath::Abs(Direction | Obb.AxisY) <= (Obb.ExtentY + ActorRadiusY);
//	bool bInsideZ = FMath::Abs(Direction | Obb.AxisZ) <= (Obb.ExtentZ + ActorRadiusZ);
//
//	return bInsideX && bInsideY && bInsideZ;
//}

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

FWorldLocationOnScreen2 UFogOfWarComponentStatics::GetActorWorldLocationsOnScreen(const AActor* Actor)
{
	if (!Actor) { return FWorldLocationOnScreen2{}; }
	if (UWorld* World = Actor->GetWorld(); !World) { return FWorldLocationOnScreen2{}; }

	const APlayerController* Controller = Actor->GetWorld()->GetFirstPlayerController();
	if (!Controller) { return FWorldLocationOnScreen2{}; }

	FVector Origin;
	FVector BoxExtent;
	Actor->GetActorBounds(false, Origin, BoxExtent);
	Origin.Z += (Actor->GetActorUpVector() * BoxExtent.Z).Z;

	const FVector ForwardVector = Actor->GetActorForwardVector();
	const FVector RightVector = Actor->GetActorRightVector();

	TArray<FVector> Corners;
	Corners.Add({ Origin - RightVector * BoxExtent.X - ForwardVector * BoxExtent.Y });	// left down
	Corners.Add({ Origin - RightVector * BoxExtent.X + ForwardVector * BoxExtent.Y });	// left top
	Corners.Add({ Origin + RightVector * BoxExtent.X - ForwardVector * BoxExtent.Y });	// right down
	Corners.Add({ Origin + RightVector * BoxExtent.X + ForwardVector * BoxExtent.Y });	// right top

	FVector2D LeftDownScreenPosition{ FogOfWarConst::kInfinity , -FogOfWarConst::kInfinity };
	FVector2D RightTopScreenPosition{ -FogOfWarConst::kInfinity, FogOfWarConst::kInfinity };
	int32 count{ 0 };

	int32 ViewportWidth{ 0 };
	int32 ViewportHeight{ 0 };
	Controller->GetViewportSize(ViewportWidth, ViewportHeight);

	for (const FVector& Corner : Corners)
	{
		FVector2D ScreenPosition;
		if (Controller->ProjectWorldLocationToScreen(Corner, ScreenPosition))
		{
			ScreenPosition.X = FMath::Clamp(ScreenPosition.X, 0, ViewportWidth);
			ScreenPosition.Y = FMath::Clamp(ScreenPosition.Y, 0, ViewportHeight);

			LeftDownScreenPosition.X = FMath::Min(LeftDownScreenPosition.X, ScreenPosition.X);
			LeftDownScreenPosition.Y = FMath::Max(LeftDownScreenPosition.Y, ScreenPosition.Y);

			RightTopScreenPosition.X = FMath::Max(RightTopScreenPosition.X, ScreenPosition.X);
			RightTopScreenPosition.Y = FMath::Min(RightTopScreenPosition.Y, ScreenPosition.Y);
		}
	}

	if (LeftDownScreenPosition.ContainsNaN() || RightTopScreenPosition.ContainsNaN()) { return FWorldLocationOnScreen2{}; }

	FVector LeftDownLocation;
	FVector RightTopLocation;
	FVector WorldDirection;
	Controller->DeprojectScreenPositionToWorld(LeftDownScreenPosition.X, LeftDownScreenPosition.Y, LeftDownLocation, WorldDirection);
	Controller->DeprojectScreenPositionToWorld(RightTopScreenPosition.X, RightTopScreenPosition.Y, RightTopLocation, WorldDirection);

	return FWorldLocationOnScreen2{ FVector2D{LeftDownLocation}, FVector2D{RightTopLocation} };
}

bool UFogOfWarComponentStatics::IsPositionOnScreen(const FVector2D& ScreenPosition)
{
	return ScreenPosition.GetMin() > 0.;
}

FOGOFWAR_API bool UFogOfWarComponentStatics::GetScreenCornersWorldLocation(TArray<FVector>& OutCorners, const UObject* WorldContextObject, const double GroundHeight)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World) { return false; }

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!PlayerController) { return false; }
	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer || !LocalPlayer->ViewportClient) { return false; }

	int32 SizeX, SizeY;
	PlayerController->GetViewportSize(SizeX, SizeY);

	FVector WorldLocation;
	FVector WorldDirection;
	if (PlayerController->DeprojectScreenPositionToWorld(0.f, 0.f, WorldLocation, WorldDirection))
	{
		OutCorners.Add(FMath::LinePlaneIntersection(WorldLocation, WorldLocation + WorldDirection * 1000000.f, FVector(0, 0, GroundHeight), FVector::UpVector));
	}

	if (PlayerController->DeprojectScreenPositionToWorld(SizeX, 0.f, WorldLocation, WorldDirection))
	{
		OutCorners.Add(FMath::LinePlaneIntersection(WorldLocation, WorldLocation + WorldDirection * 1000000.f, FVector(0, 0, GroundHeight), FVector::UpVector));
	}

	if (PlayerController->DeprojectScreenPositionToWorld(0.f, SizeY, WorldLocation, WorldDirection))
	{
		OutCorners.Add(FMath::LinePlaneIntersection(WorldLocation, WorldLocation + WorldDirection * 1000000.f, FVector(0, 0, GroundHeight), FVector::UpVector));
	}

	if (PlayerController->DeprojectScreenPositionToWorld(SizeX, SizeY, WorldLocation, WorldDirection))
	{
		OutCorners.Add(FMath::LinePlaneIntersection(WorldLocation, WorldLocation + WorldDirection * 1000000.f, FVector(0, 0, GroundHeight), FVector::UpVector));
	}

	return true;
}

double UFogOfWarComponentStatics::GetSignedAngleBetweenVectors2D(const FVector2D& A, const FVector2D& B)
{
	int Sign = FMath::Sign(A.X * B.Y - B.X * A.Y);
	double Angle = FMath::Acos(FVector2D::DotProduct(A.GetSafeNormal(), B.GetSafeNormal()));

	return Sign * Angle;
}

void UFogOfWarComponentStatics::DrawOnScreen(APlayerController* Controller, FVector2f DrawCenter, int32 HalfSize, TArray<FColor>& ColorArray)
{
	if (!Controller) { return; }

	int32 ViewportWidth{ 0 }, ViewportHeight{ 0 };
	if (Controller->GetViewportSize(ViewportWidth, ViewportHeight); FMath::Min(ViewportWidth, ViewportHeight) <= 0) { return; }

	if (DrawCenter.X > ViewportWidth || DrawCenter.Y > ViewportHeight) { return; }

	ColorArray.SetNum(ViewportWidth * ViewportHeight);
	for (FColor& Color : ColorArray)
	{
		Color.R = 255;
		Color.G = 255;
		Color.B = 255;
		Color.A = 255 / 2;
	}

	for (int32 y = DrawCenter.Y - HalfSize; y < DrawCenter.Y + HalfSize; y++)
	{
		for (int32 x = DrawCenter.X - HalfSize; x < DrawCenter.X + HalfSize; x++)
		{
			int32 Index = y * ViewportWidth + x;
			if (Index < ColorArray.Num())
			{
				ColorArray[Index].R = 255;
				ColorArray[Index].G = 0;
				ColorArray[Index].B = 0;
				ColorArray[Index].A = 255;
			}
		}
	}
}

FGridBoundsDataType UFogOfWarComponentStatics::MakeGridBoundsTypeFromActor(const AActor& Actor)
{
	FVector Min, Max;

	GetActorMinMax(Min, Max, Actor);

	return FGridBoundsDataType{ Min, Max };
}

void UFogOfWarComponentStatics::GetActorMinMax(FVector& Min, FVector& Max, const AActor& Actor)
{
	FBox Box = Actor.GetComponentsBoundingBox(true);
	Min = Box.Min;
	Max = Box.Max;
}

void UFogOfWarComponentStatics::GetBoxPlaneMin(FVector& Min, const FVector& BoxOrigin, const FVector& BoxExtent, const FVector& VectorToPlane, const FVector& ForwardVectorOnPlane, const FVector& RightVectorOnPlane)
{
	FVector Center{ BoxOrigin + VectorToPlane * BoxExtent };

	FVector PlaneLeftDownLocation;
	FVector PlaceLeftTopLocation;
	FVector PlaceRightDownLocation;
	FVector PlaceRightTopLocation;
	GetPlaneCorners(PlaneLeftDownLocation, PlaceLeftTopLocation, PlaceRightDownLocation, PlaceRightTopLocation, Center, BoxExtent, ForwardVectorOnPlane, RightVectorOnPlane);

	Min.X = FMath::Min3(FMath::Min(PlaneLeftDownLocation.X, PlaceLeftTopLocation.X), PlaceRightDownLocation.X, PlaceRightTopLocation.X);
	Min.Y = FMath::Min3(FMath::Min(PlaneLeftDownLocation.Y, PlaceLeftTopLocation.Y), PlaceRightDownLocation.Y, PlaceRightTopLocation.Y);
	Min.Z = FMath::Min3(FMath::Min(PlaneLeftDownLocation.Z, PlaceLeftTopLocation.Z), PlaceRightDownLocation.Z, PlaceRightTopLocation.Z);
}

void UFogOfWarComponentStatics::GetBoxPlaneMax(FVector& Max, const FVector& BoxOrigin, const FVector& BoxExtent, const FVector& VectorToPlane, const FVector& ForwardVectorOnPlane, const FVector& RightVectorOnPlane)
{
	FVector Center{ BoxOrigin + VectorToPlane * BoxExtent };

	FVector PlaneLeftDownLocation;
	FVector PlaceLeftTopLocation;
	FVector PlaceRightDownLocation;
	FVector PlaceRightTopLocation;
	GetPlaneCorners(PlaneLeftDownLocation, PlaceLeftTopLocation, PlaceRightDownLocation, PlaceRightTopLocation, Center, BoxExtent, ForwardVectorOnPlane, RightVectorOnPlane);

	Max.X = FMath::Max3(FMath::Max(PlaneLeftDownLocation.X, PlaceLeftTopLocation.X), PlaceRightDownLocation.X, PlaceRightTopLocation.X);
	Max.Y = FMath::Max3(FMath::Max(PlaneLeftDownLocation.Y, PlaceLeftTopLocation.Y), PlaceRightDownLocation.Y, PlaceRightTopLocation.Y);
	Max.Z = FMath::Max3(FMath::Max(PlaneLeftDownLocation.Z, PlaceLeftTopLocation.Z), PlaceRightDownLocation.Z, PlaceRightTopLocation.Z);
}

void UFogOfWarComponentStatics::GetPlaneCorners(FVector& LeftDown, FVector& LeftTop, FVector& RightDown, FVector& RightTop, const FVector& PlaneCenter, const FVector& Extent, const FVector& ForwardVectorOnPlane, const FVector& RightVectorOnPlane)
{
	FVector PlaneLeftDownLocation{ PlaneCenter - ForwardVectorOnPlane * Extent - RightVectorOnPlane * Extent };
	FVector PlaceLeftTopLocation{ PlaneCenter + ForwardVectorOnPlane * Extent - RightVectorOnPlane * Extent };
	FVector PlaceRightDownLocation{ PlaneCenter - ForwardVectorOnPlane * Extent + RightVectorOnPlane * Extent };
	FVector PlaceRightTopLocation{ PlaneCenter + ForwardVectorOnPlane * Extent + RightVectorOnPlane * Extent };

	LeftDown = PlaneLeftDownLocation;
	LeftTop = PlaceLeftTopLocation;
	RightDown = PlaceRightDownLocation;
	RightTop = PlaceRightTopLocation;
}

bool UFogOfWarComponentStatics::GetGridSize(FVector2D& GridSize, const EGridType GridType, const UObject* WorldContextObject)
{
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World) { return false; }

	if (GridType == EGridType::World)
	{
		const UWorldHeightSubsystem* WorldHeightSubsystem = World->GetSubsystem<UWorldHeightSubsystem>();
		WorldHeightSubsystem->GetGridSize(GridSize);
	}
	else if (GridType == EGridType::Screen)
	{
		const APlayerController* PlayerController = World->GetFirstPlayerController();
		if (!PlayerController) { return false; }

		int32 ViewportSizeX, ViewportSizeY;
		PlayerController->GetViewportSize(ViewportSizeX, ViewportSizeY);

		const UFogOfWarSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UFogOfWarSubsystem>(World->GetFirstLocalPlayerFromController());
		const TOptional<FIntPoint> ScreenSize{ Subsystem->GetScreenSize() };
		if (!ScreenSize.IsSet()) { return false; }

		GridSize = { static_cast<double>(ViewportSizeX / ScreenSize.GetValue().X), static_cast<double>(ViewportSizeY / ScreenSize.GetValue().Y) };
	}
	else { return false; }

	return true;
}

FVector UFogOfWarComponentStatics::GetIntersectionFromCameraToGround(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World) { return FogOfWarConst::kInvalidVector; }

	const APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!PlayerController) { return FogOfWarConst::kInvalidVector; }

	FVector CameraLocation;
	FRotator CameraRotation;
	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

	//地面的法向量
	const FVector NormalOfGround = FVector::UpVector;

	return CameraLocation + CameraRotation.Vector() * (-CameraLocation.Z / CameraRotation.Vector().Z);
}

TArray<FVector> UFogOfWarComponentStatics::GetCameraFrustumGroundIntersections(const UObject* WorldContextObject, const double GroundHeight)
{
	TArray<FVector> OutPoints;

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World) { return OutPoints; }

	const APlayerController* PC = World->GetFirstPlayerController();
	if (!PC) { return OutPoints; }


	// 1. 获取当前视口（屏幕）的尺寸（像素）
	int32 SizeX, SizeY;
	PC->GetViewportSize(SizeX, SizeY);
	if (SizeX <= 0 || SizeY <= 0) { return OutPoints; }

	// 2. 定义屏幕的四个角点（像素坐标）
	// 顺序：左上 -> 右上 -> 右下 -> 左下
	TArray<FVector2D> ScreenCorners;
	ScreenCorners.SetNum(4);
	ScreenCorners[static_cast<int32>(ECorner::LeftTop)] = FVector2D(0.f, 0.f);			// 左上 (Top-Left)
	ScreenCorners[static_cast<int32>(ECorner::RightTop)] = FVector2D(SizeX, 0.f);		// 右上 (Top-Right)
	ScreenCorners[static_cast<int32>(ECorner::RightDown)] = FVector2D(SizeX, SizeY);	// 右下 (Bottom-Right)
	ScreenCorners[static_cast<int32>(ECorner::LeftDown)] = FVector2D(0.f, SizeY);		// 左下 (Bottom-Left)

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
				float t = (GroundHeight - RayOrigin.Z) / RayDirection.Z;

				// 计算精确的世界坐标交点
				FVector IntersectionPoint = RayOrigin + RayDirection * t;

				OutPoints.Add(IntersectionPoint);
			}
			else
			{
				// 如果镜头抬得太高，某些角发出的射线可能会射向天空，此时无法与地面相交
				// 你可以根据业务需求决定如何处理这些“朝天”的边界点
				UE_LOG(LogTemp, Warning, TEXT("射线未射向地面，可能相机仰角过大。"));
			}
		}
	}

	return OutPoints;
}

bool UFogOfWarComponentStatics::GetActorOrientBox(FOrientedBox& OrientedBox, const AActor* Actor)
{
	if (!Actor) { return false; }

	OrientedBox.Center = Actor->GetActorLocation();
	OrientedBox.AxisX = Actor->GetActorForwardVector();
	OrientedBox.AxisY = Actor->GetActorRightVector();
	OrientedBox.AxisZ = Actor->GetActorUpVector();

	//FVector ActorOrigin, ActorExtent;
	//Actor->GetActorBounds(false, ActorOrigin, ActorExtent);
	//OrientedBox.Center = ActorOrigin;

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

	//FVector ActorOrigin, ActorExtent;
	//Actor->GetActorBounds(false, ActorOrigin, ActorExtent);
	//OrientedBox.Center = ActorOrigin;

	FTransform Transform{ FTransform::Identity };
	Transform.SetScale3D(Component->GetComponentScale());
	FBox LocalBox = Component->CalcBounds(Transform).GetBox();

	OrientedBox.ExtentX = FMath::Abs(LocalBox.GetExtent().X);
	OrientedBox.ExtentY = FMath::Abs(LocalBox.GetExtent().Y);
	OrientedBox.ExtentZ = FMath::Abs(LocalBox.GetExtent().Z);

	return true;
}

FBox UFogOfWarComponentStatics::GetOrientedBoxAABB(const FOrientedBox& OrientedBox)
{
	const FFloatInterval ProjectToX{ OrientedBox.Project(FVector::ForwardVector) };
	const FFloatInterval ProjectToY{ OrientedBox.Project(FVector::RightVector) };
	const FFloatInterval ProjectToZ{ OrientedBox.Project(FVector::UpVector) };

	const FVector Min{ ProjectToX.Min, ProjectToY.Min, ProjectToZ.Min };
	const FVector Max{ ProjectToX.Max, ProjectToY.Max, ProjectToZ.Max };

	return { Min , Max };
}

FOrientedBox UFogOfWarComponentStatics::GetAABBBoxOriented(const FBox& AABB)
{
	FOrientedBox OrientedBox;

	OrientedBox.Center = AABB.GetCenter();
	OrientedBox.AxisX = FVector::ForwardVector;
	OrientedBox.AxisY = FVector::RightVector;
	OrientedBox.AxisZ = FVector::UpVector;

	OrientedBox.ExtentX = FMath::Abs(AABB.GetExtent().X);
	OrientedBox.ExtentY = FMath::Abs(AABB.GetExtent().Y);
	OrientedBox.ExtentZ = FMath::Abs(AABB.GetExtent().Z);

	return OrientedBox;
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