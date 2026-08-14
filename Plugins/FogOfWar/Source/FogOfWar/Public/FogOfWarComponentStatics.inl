// ============================================================================
// Architecture Note:
// This inline compilation architecture is modeled after Unreal Engine's
// UKismetMathLibrary (KismetMathLibrary.h / .inl / .cpp).
//
// - Non-Debug (Development/Shipping):
//   Functions are inlined via .inl to eliminate call overhead for critical path math.
// - Debug (Debug/DebugGame):
//   Inlining is disabled and functions fall back to standard module API export,
//   allowing faster incremental builds and seamless breakpoint debugging.
// ============================================================================

#pragma once

#ifndef FOGOFWAR_ALLOW_INCLUDE_INL
#error "FogOfWarComponentStatics.inl should only be included after defining FOGOFWAR_ALLOW_INCLUDE_INL"
#endif

// When FOGOFWAR_INLINE_ENABLED is true this file is included in the header
// so we need to mark these functions as inline.
#if FOGOFWAR_INLINE_ENABLED
	#define FOGOFWAR_INLINE			FORCEINLINE
#else
	#define FOGOFWAR_INLINE			// nothing
#endif

FOGOFWAR_INLINE
FBox UFogOfWarComponentStatics::GetOrientedBoxAABB(const FOrientedBox& OrientedBox)
{
	const FFloatInterval ProjectToX{ OrientedBox.Project(FVector::ForwardVector) };
	const FFloatInterval ProjectToY{ OrientedBox.Project(FVector::RightVector) };
	const FFloatInterval ProjectToZ{ OrientedBox.Project(FVector::UpVector) };

	const FVector Min{ ProjectToX.Min, ProjectToY.Min, ProjectToZ.Min };
	const FVector Max{ ProjectToX.Max, ProjectToY.Max, ProjectToZ.Max };

	return FBox{ Min , Max };
}

FOGOFWAR_INLINE
FOrientedBoxAABBAndQuat UFogOfWarComponentStatics::GetOrientedBoxAABBAndQuat(const FOrientedBox& OrientedBox)
{
	ensure(OrientedBox.AxisX.IsNormalized() && OrientedBox.AxisY.IsNormalized() && OrientedBox.AxisZ.IsNormalized());

	// 构造旋转矩阵与四元数
	const FQuat Quat{FMatrix{OrientedBox.AxisX, OrientedBox.AxisY, OrientedBox.AxisZ, FVector::ZeroVector}.ToQuat()};

	//直接使用 FOrientedBox 自带的局部空间 Extent（半长）
	//假设 FOrientedBox 的 Extent 就是不带任何旋转的 Extent
	const FVector BoxExtent{OrientedBox.ExtentX, OrientedBox.ExtentY, OrientedBox.ExtentZ};

	return FOrientedBoxAABBAndQuat{FBox{OrientedBox.Center - BoxExtent, OrientedBox.Center + BoxExtent}, Quat};
}

FOGOFWAR_INLINE
FOrientedBoxAABBAndTransform UFogOfWarComponentStatics::GetOrientedBoxAABBAndTransform(const FOrientedBox& OrientedBox)
{
	ensure(OrientedBox.AxisX.IsNormalized() && OrientedBox.AxisY.IsNormalized() && OrientedBox.AxisZ.IsNormalized());

	const FVector Max{OrientedBox.ExtentX, OrientedBox.ExtentY, OrientedBox.ExtentZ};
	return FOrientedBoxAABBAndTransform{FBox{-Max, Max},  FTransform{FMatrix{OrientedBox.AxisX, OrientedBox.AxisY, OrientedBox.AxisZ, FVector::ZeroVector}.ToQuat(), OrientedBox.Center}};
}

FOGOFWAR_INLINE
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

FOGOFWAR_INLINE
TOptional<FIntPoint> UFogOfWarComponentStatics::GetGridPositionOnScreen(const FVector& WorldLocation, const FIntPoint& ScreenSize, const FBox2D& ScreenBoundingBox)
{
	const double NormalizedU{ (WorldLocation.Y - ScreenBoundingBox.Min.Y) / ScreenBoundingBox.GetSize().Y };
	const double NormalizedV{ (ScreenBoundingBox.Max.X - WorldLocation.X) / ScreenBoundingBox.GetSize().X };

	if (NormalizedU >= 0.0 && NormalizedU <= 1.0 && NormalizedV >= 0.0 && NormalizedV <= 1.0) 
	{
		return FIntPoint{ FMath::FloorToInt32(NormalizedU * ScreenSize.X), FMath::FloorToInt32(NormalizedV * ScreenSize.Y) };
	}
	return NullOpt;
}

FOGOFWAR_INLINE
FogOfWarTypes::GridIndexType UFogOfWarComponentStatics::GetGridIndexOnScreen(const FVector& WorldLocation, const FIntPoint& ScreenSize, const FBox2D& ScreenBoundingBox)
{
	const TOptional<FIntPoint> Position{ GetGridPositionOnScreen(WorldLocation, ScreenSize, ScreenBoundingBox) };
	if (Position.IsSet()) { return Position.GetValue().Y * ScreenSize.X + Position.GetValue().X; }
	return INDEX_NONE;
}

FOGOFWAR_INLINE
FVector2D UFogOfWarComponentStatics::ProjectWorldDirectionToScreen(const FVector& WorldDirection)
{
	return FVector2D{ WorldDirection.Y, -WorldDirection.X };
}

#undef FOGOFWAR_INLINE