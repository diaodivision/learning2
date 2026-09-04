// Fill out your copyright notice in the Description page of Project Settings.


//#include "FogOfWarComponentStatics.h"
//#include "GameFramework/Actor.h"
//
//FWorldLocationOnScreen2::FWorldLocationOnScreen2(FVector2D MinPosition, FVector2D MaxPosition, FVector2D InForwardVector, FVector2D InRightVector)
//	:LeftDownLocation(MinPosition), RightVector(InRightVector.GetSafeNormal())
//{
//	FVector2D DiagonalVector{ MaxPosition - MinPosition };
//
//	InForwardVector.Normalize();
//	InRightVector.Normalize();
//
//	Width = FVector2D::DotProduct(DiagonalVector, InRightVector);
//	Height = FVector2D::DotProduct(DiagonalVector, InForwardVector);
//}
//
//bool FWorldLocationOnScreen2::IsValid() const
//{
//	bool bLocationValid = LeftDownLocation.GetMax() != DBL_MAX && LeftDownLocation.GetMin() != DBL_MIN;
//	bool bSizeValid = FMath::Max(Width, Height) != DBL_MAX && FMath::Min(Width, Height) > .0;
//
//	return bLocationValid && bSizeValid && !RightVector.IsNearlyZero();
//}
//
//FVector2D FWorldLocationOnScreen2::GetCorner(ECorner Corner) const
//{
//	switch (Corner)
//	{
//	case ECorner::LeftDown:
//		return LeftDownLocation;
//	case ECorner::LeftTop:
//		return LeftDownLocation + ForwardVector() * Height;
//	case ECorner::RightDown:
//		return LeftDownLocation + RightVector * Width;
//	case ECorner::RightTop:
//		return LeftDownLocation + RightVector * Width + ForwardVector() * Height;
//	default:
//		return FVector2D{ DBL_MAX, DBL_MAX };
//	}
//}
//
//FWorldLocationOnScreen2 UFogOfWarComponentStatics::GetActorWorldLocationsOnScreen(const AActor* Actor)
//{
//	if (!Actor) { return FWorldLocationOnScreen2{}; }
//	if (UWorld* World = Actor->GetWorld(); !World) { return FWorldLocationOnScreen2{}; }
//
//	const APlayerController* Controller = Actor->GetWorld()->GetFirstPlayerController();
//	if (!Controller) { return FWorldLocationOnScreen2{}; }
//
//	FVector Origin;
//	FVector BoxExtent;
//	Actor->GetActorBounds(false, Origin, BoxExtent);
//	Origin.Z += (Actor->GetActorUpVector() * BoxExtent.Z).Z;
//
//
//	const FVector ForwardVector = Actor->GetActorForwardVector();
//	const FVector RightVector = Actor->GetActorRightVector();
//
//
//	TArray<FVector> Corners;
//	Corners.Add({ Origin - RightVector * BoxExtent.X - ForwardVector * BoxExtent.Y });	// left down
//	Corners.Add({ Origin - RightVector * BoxExtent.X + ForwardVector * BoxExtent.Y });	// left top
//	Corners.Add({ Origin + RightVector * BoxExtent.X - ForwardVector * BoxExtent.Y });	// right down
//	Corners.Add({ Origin + RightVector * BoxExtent.X + ForwardVector * BoxExtent.Y });	// right top
//
//	/*FVector2D MinScreenPosition{ DBL_MAX, DBL_MAX };
//	FVector2D MaxScreenPosition{ DBL_MIN, DBL_MIN };*/
//	const double Infinity = std::numeric_limits<double>::infinity();
//	FVector2D LeftDownScreenPosition{ Infinity , -Infinity };
//	FVector2D RightTopScreenPosition{ -Infinity, Infinity };
//	int32 count{ 0 };
//
//
//	int32 ViewportWidth{ 0 };
//	int32 ViewportHeight{ 0 };
//	Controller->GetViewportSize(ViewportWidth, ViewportHeight);
//	for (const FVector& Corner : Corners)
//	{
//		FVector2D ScreenPosition;
//		if (Controller->ProjectWorldLocationToScreen(Corner, ScreenPosition))
//		{
//
//			ScreenPosition.X = FMath::Clamp(ScreenPosition.X, 0, ViewportWidth);
//			ScreenPosition.Y = FMath::Clamp(ScreenPosition.Y, 0, ViewportHeight);
//
//			LeftDownScreenPosition.X = FMath::Min(LeftDownScreenPosition.X, ScreenPosition.X);
//			LeftDownScreenPosition.Y = FMath::Max(LeftDownScreenPosition.Y, ScreenPosition.Y);
//
//			RightTopScreenPosition.X = FMath::Max(RightTopScreenPosition.X, ScreenPosition.X);
//			RightTopScreenPosition.Y = FMath::Min(RightTopScreenPosition.Y, ScreenPosition.Y);
//		}
//	}
//
//	if (LeftDownScreenPosition.ContainsNaN() || RightTopScreenPosition.ContainsNaN()) { return FWorldLocationOnScreen2{}; }
//
//	FVector LeftDownLocation;
//	FVector RightTopLocation;
//	FVector WorldDirection;
//	Controller->DeprojectScreenPositionToWorld(LeftDownScreenPosition.X, LeftDownScreenPosition.Y, LeftDownLocation, WorldDirection);
//	Controller->DeprojectScreenPositionToWorld(RightTopScreenPosition.X, RightTopScreenPosition.Y, RightTopLocation, WorldDirection);
//
//	return FWorldLocationOnScreen2{ FVector2D{LeftDownLocation}, FVector2D{RightTopLocation}, FVector2D{ForwardVector}, FVector2D{RightVector} };
//}
//
//bool UFogOfWarComponentStatics::IsPositionOnScreen(const FVector2D& ScreenPosition)
//{
//	return ScreenPosition.GetMin() > 0.;
//}
//
//void UFogOfWarComponentStatics::DrawOnScreen(APlayerController* Controller, FVector2f DrawCenter, int32 HalfSize, TArray<FColor>& ColorArray)
//{
//	if (!Controller) { return; }
//
//	int32 ViewportWidth{ 0 }, ViewportHeight{ 0 };
//	if (Controller->GetViewportSize(ViewportWidth, ViewportHeight); FMath::Min(ViewportWidth, ViewportHeight) <= 0) { return; }
//
//	if (DrawCenter.X > ViewportWidth || DrawCenter.Y > ViewportHeight) { return; }
//
//	ColorArray.SetNum(ViewportWidth * ViewportHeight);
//	for (FColor& Color : ColorArray)
//	{
//		Color.R = 255;
//		Color.G = 255;
//		Color.B = 255;
//		Color.A = 255 / 2;
//	}
//
//	for (int32 y = DrawCenter.Y - HalfSize; y < DrawCenter.Y + HalfSize; y++)
//	{
//		for (int32 x = DrawCenter.X - HalfSize; x < DrawCenter.X + HalfSize; x++)
//		{
//			int32 Index = y * ViewportWidth + x;
//			if (Index < ColorArray.Num())
//			{
//				ColorArray[Index].R = 255;
//				ColorArray[Index].G = 0;
//				ColorArray[Index].B = 0;
//				ColorArray[Index].A = 255;
//			}
//		}
//	}
//}