// Fill out your copyright notice in the Description page of Project Settings.


#include "DynamicTextureComponent.h"
#include "Kismet\KismetMathLibrary.h"


// Sets default values for this component's properties
UDynamicTextureComponent::UDynamicTextureComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

UDynamicTextureComponent::UDynamicTextureComponent(int32 TextureWidth, int32 TextureHeight) : TextureWidth(TextureWidth), TextureHeight(TextureHeight)
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UDynamicTextureComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}

// Called every frame
void UDynamicTextureComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UDynamicTextureComponent::InitializeTexture(int32 Width, int32 Height, UMaterialInterface* TargetMaterial, FName ParameterName)
{
	TextureWidth = Width;
	TextureHeight = Height;

	//DynamicTexture = UTexture2D::CreateTransient(TextureWidth, TextureHeight, EPixelFormat::PF_R8G8B8A8);
	DynamicTexture = UTexture2D::CreateTransient(TextureWidth, TextureHeight, EPixelFormat::PF_G8);
	if (DynamicTexture)
	{
		DynamicTexture->CompressionSettings = TC_EditorIcon;
		DynamicTexture->SRGB = true;
		DynamicTexture->Filter = TF_MAX;
		//DynamicTexture->SRGB = false;
		//DynamicTexture->CompressionSettings = TC_Grayscale;
		//DynamicTexture->Filter = TF_Nearest;
		//DynamicTexture->MipGenSettings = TMGS_NoMipmaps;

		DynamicTexture->UpdateResource();
	}

	if (TargetMaterial)
	{
		MaterialInstanceDynamic = UMaterialInstanceDynamic::Create(TargetMaterial, this);
		if (MaterialInstanceDynamic)
		{
			MaterialInstanceDynamic->SetTextureParameterValue(ParameterName, DynamicTexture);
		}
	}
}

void UDynamicTextureComponent::InitColorArray(UPARAM(ref)TArray<FColor>& ColorArray) const
{
	ColorArray.SetNumZeroed(TextureWidth * TextureHeight);
}

void UDynamicTextureComponent::InitColorArrayPFG8(UPARAM(ref)TArray<uint8>& ColorArray) const
{
	ColorArray.SetNumZeroed(TextureWidth * TextureHeight);
}

void UDynamicTextureComponent::SetFloorComponent(UMeshComponent* InFloorComponent)
{
	FloorMeshComponent = InFloorComponent;
	if (FloorMeshComponent.IsValid() && MaterialInstanceDynamic)
	{
		FloorMeshComponent->SetMaterial(0, MaterialInstanceDynamic);
	}
}

void UDynamicTextureComponent::SetPostProcessVolume(APostProcessVolume* InPostProcessVolume)
{
	PostProcessVolume = InPostProcessVolume;
	if (PostProcessVolume.IsValid() && MaterialInstanceDynamic)
	{
		PostProcessVolume->Settings.AddBlendable(MaterialInstanceDynamic, 1.f);
	}
}

void UDynamicTextureComponent::UpdateTextureFromArray(const TArray<FColor>& ColorArray)
{

	if (DynamicTexture) { UpdateTextureResource(ColorArray); }
}

void UDynamicTextureComponent::UpdateTextureFromArrayPFG8(const TArray<uint8>& ColorArray)
{
	if (DynamicTexture) { UpdateTextureResource(ColorArray); }
}

void UDynamicTextureComponent::CalculateVisionArea(UPARAM(ref)TArray<FColor>& ColorArray, const TArray<FVector2D>& VisionStartPos, TArray<FVector2f> Vision, const FVector2D& GroundOrigin, const FIntVector2& GroundSize) const
{
	if (const int32 ExpectedSize = TextureWidth * TextureHeight; ColorArray.Num() != ExpectedSize)
	{
		return;
	}

	if (VisionStartPos.Num() * 2 != Vision.Num() || Vision.Num() % 2)
	{
		return;
	}

	for (FVector2f& V : Vision) { V.Normalize(); }

	TArray<FIntVector2> VisionStartPosOnGrid;
	for (const FVector2D& Pos : VisionStartPos)
	{
		FVector2D PosOnGrid{ Pos - GroundOrigin };
		PosOnGrid.X = FMath::Floor(PosOnGrid.X * TextureWidth / GroundSize.X);
		PosOnGrid.Y = FMath::Floor(PosOnGrid.Y * TextureHeight / GroundSize.Y);

		VisionStartPosOnGrid.Add(FIntVector2(PosOnGrid.X, PosOnGrid.Y));
	}

	TArray<FVector2f> DirectionFromCenterToEdge;
	for (int32 i = 0; i < Vision.Num(); i += 2)
	{
		FVector2f Direction = Vision[i] + Vision[i + 1];
		Direction.Normalize();
		DirectionFromCenterToEdge.Add(Direction.GetRotated(-90));
		DirectionFromCenterToEdge.Add(Direction.GetRotated(90));
	}

	ParallelFor(TextureHeight, [&, this](int32 y) {
		int Count{ 0 };

		for (int32 x = 0; x < TextureWidth; x++)
		{
			int32 Index = y * TextureWidth + x;
			if (Index >= ColorArray.Num()) { continue; }

			FColor& Color = ColorArray[Index];

			bool bGridVisible = IsGridInVision(FIntVector2(x, y), VisionStartPosOnGrid, Vision);
			if (bGridVisible)
			{
				float Weight = 0.f;

				if (FVector2f::DotProduct(FVector2f(FIntVector2(x, y) - VisionStartPosOnGrid[0]), Vision[0]) > FVector2f::DotProduct(FVector2f(FIntVector2(x, y) - VisionStartPosOnGrid[0]), Vision[1]))
				{
					Weight = GetNormalizedDistanceToBoundary(FIntVector2(x, y), VisionStartPosOnGrid[0], Vision[0], DirectionFromCenterToEdge[0]);
				}
				else
				{
					Weight = GetNormalizedDistanceToBoundary(FIntVector2(x, y), VisionStartPosOnGrid[0], Vision[1], DirectionFromCenterToEdge[1]);
				}

				FVector2f GridDirection = FVector2f(FIntVector2(x, y) - VisionStartPosOnGrid[0]);

				Weight = FMath::Clamp(Weight, 0.f, 1.f);

				if (Weight > SmoothStepThreshold && FMath::Abs(FVector2f::DotProduct((Vision[0] + Vision[1]).GetSafeNormal(), GridDirection)) < SmoothStepDistanceThreshold)
				{
					Color.B = 255 * (1 - FMath::SmoothStep(SmoothStepThreshold, 1.f, Weight));
				}
				else { Color.B = 255; }
			}
			else { Color.B = 0; }
			//Color.B = IsGridInVision(FIntVector2(x, y), VisionStartPosOnGrid, Vision) ? 255 : 0;
			Color.R = 0;
			Color.G = 0;
			Color.A = 255;

			/*if (FMath::Abs(x - VisionStartPosOnGrid[0].X) < 10 && FMath::Abs(y - VisionStartPosOnGrid[0].Y) < 10)
			{
				Color.B = 0;
				Color.R = 255;
			}*/
		}
		}
	);
}

void UDynamicTextureComponent::CalculateVisionAreaPFG8(UPARAM(ref)TArray<uint8>& ColorArray, const TArray<FVector2D>& VisionStartPos, TArray<FVector2f> Vision, const FVector2D& GroundOrigin, const FIntVector2& GroundSize) const
{
	if (const int32 ExpectedSize = TextureWidth * TextureHeight; ColorArray.Num() != ExpectedSize)
	{
		return;
	}

	if (VisionStartPos.Num() * 2 != Vision.Num() || Vision.Num() % 2)
	{
		return;
	}

	for (FVector2f& V : Vision) { V.Normalize(); }

	TArray<FIntVector2> VisionStartPosOnGrid;
	for (const FVector2D& Pos : VisionStartPos)
	{
		FVector2D PosOnGrid{ Pos - GroundOrigin };
		PosOnGrid.X = FMath::Floor(PosOnGrid.X * TextureWidth / GroundSize.X);
		PosOnGrid.Y = FMath::Floor(PosOnGrid.Y * TextureHeight / GroundSize.Y);

		VisionStartPosOnGrid.Add(FIntVector2(PosOnGrid.X, PosOnGrid.Y));
	}

	TArray<FVector2f> DirectionFromCenterToEdge;
	for (int32 i = 0; i < Vision.Num(); i += 2)
	{
		FVector2f Direction = Vision[i] + Vision[i + 1];
		Direction.Normalize();
		DirectionFromCenterToEdge.Add(Direction.GetRotated(-90));
		DirectionFromCenterToEdge.Add(Direction.GetRotated(90));
	}

	ParallelFor(TextureHeight, [&, this](int32 y) {
		int Count{ 0 };

		for (int32 x = 0; x < TextureWidth; x++)
		{
			int32 Index = y * TextureWidth + x;
			if (Index >= ColorArray.Num()) { continue; }

			uint8& Color = ColorArray[Index];

			bool bGridVisible = IsGridInVision(FIntVector2(x, y), VisionStartPosOnGrid, Vision);
			if (bGridVisible)
			{
				float Weight = 0.f;

				if (FVector2f::DotProduct(FVector2f(FIntVector2(x, y) - VisionStartPosOnGrid[0]), Vision[0]) > FVector2f::DotProduct(FVector2f(FIntVector2(x, y) - VisionStartPosOnGrid[0]), Vision[1]))
				{
					Weight = GetNormalizedDistanceToBoundary(FIntVector2(x, y), VisionStartPosOnGrid[0], Vision[0], DirectionFromCenterToEdge[0]);
				}
				else
				{
					Weight = GetNormalizedDistanceToBoundary(FIntVector2(x, y), VisionStartPosOnGrid[0], Vision[1], DirectionFromCenterToEdge[1]);
				}

				FVector2f GridDirection = FVector2f(FIntVector2(x, y) - VisionStartPosOnGrid[0]);

				Weight = FMath::Clamp(Weight, 0.f, 1.f);

				if (Weight > SmoothStepThreshold && FMath::Abs(FVector2f::DotProduct((Vision[0] + Vision[1]).GetSafeNormal(), GridDirection)) < SmoothStepDistanceThreshold)
				{
					Color = 255 * (1 - FMath::SmoothStep(SmoothStepThreshold, 1.f, Weight));
				}
				else { Color = 255; }
			}
			else { Color = 0; }
		}
		}
	);
}

void UDynamicTextureComponent::InitColorArrayCompressed(UPARAM(ref)TArray<FColor>& ColorArray) const
{
	int32 ArraySize = FMath::CeilToInt32(static_cast<float>(TextureWidth) * static_cast<float>(TextureHeight) / 32);
	ColorArray.SetNumZeroed(ArraySize);
}

void UDynamicTextureComponent::UpdateTextureFromArrayCompressed(const TArray<FColor>& ColorArray)
{
	if (const int32 ExpectedSize = TextureWidth * TextureHeight; ColorArray.Num() * 32 != ExpectedSize)
	{
		return;
	}

	if (!DynamicTexture) { return; }

	UpdateTextureResource(ColorArray);
}

void UDynamicTextureComponent::CalculateVisionAreaCompressed(UPARAM(ref)TArray<FColor>& ColorArray, const TArray<FVector2D>& VisionStartPos, TArray<FVector2f> Vision, const FVector2D& GroundOrigin, const FIntVector2& GroundSize) const
{
	if (const int32 ExpectedSize = TextureWidth * TextureHeight; ColorArray.Num() * 32 != ExpectedSize)
	{
		return;
	}

	if (VisionStartPos.Num() * 2 != Vision.Num() || Vision.Num() % 2)
	{

		return;
	}

	for (FVector2f& V : Vision) { V.Normalize(); }

	TArray<FIntVector2> VisionStartPosOnGrid;
	for (const FVector2D& Pos : VisionStartPos)
	{
		FVector2D PosOnGrid{ Pos - GroundOrigin };
		PosOnGrid.X = FMath::Floor(PosOnGrid.X * TextureWidth / GroundSize.X);
		PosOnGrid.Y = FMath::Floor(PosOnGrid.Y * TextureHeight / GroundSize.Y);

		VisionStartPosOnGrid.Add(FIntVector2(PosOnGrid.X, PosOnGrid.Y));
	}

	/*for (int32 y = 0, Count = 0; y < TextureWidth; y++)
	{
		for (int32 x = 0; x < TextureHeight; x++)
		{*/
		//int32 Index = x * TextureHeight + y;
	for (int32 y = 0; y < TextureHeight; y++)
	{
		for (int32 x = 0; x < TextureWidth; x++)
		{
			int32 Index = y * TextureWidth + x;
			int32 IndexOnArray = Index / 32;
			int8 IndexOnFColor = Index % 32;	//todo assert( (y * TextureWidth) % 32 == 0); int8 IndexOnFColor = Index % x;

			if (IndexOnArray >= ColorArray.Num()) { return; }

			FColor& Color = ColorArray[IndexOnArray];

			uint8* ColorByte = &Color.R;
			if (8 <= IndexOnFColor && IndexOnFColor < 16) { ColorByte = &Color.G; }
			else if (16 <= IndexOnFColor && IndexOnFColor < 24) { ColorByte = &Color.B; }
			else if (24 <= IndexOnFColor) { ColorByte = &Color.A; }

			//Color.B = IsGridInVision(FIntVector2(x, y), VisionStartPosOnGrid, Vision, bLogged) ? 255 : 0;
			uint8 OffsetOfColorByte = 1 << (IndexOnFColor % 8);

			//if (op) { *ColorByte |= OffsetOfColorByte; }
			if (IsGridInVision(FIntVector2(x, y), VisionStartPosOnGrid, Vision)) { *ColorByte |= OffsetOfColorByte; }
			else if (*ColorByte & OffsetOfColorByte) { *ColorByte ^= OffsetOfColorByte; }
		}
	}
}

int32 UDynamicTextureComponent::RandomArray(UPARAM(ref) TArray<FColor>& ColorArray) const
{
	if (const int32 ExpectedSize = TextureWidth * TextureHeight; ColorArray.Num() != ExpectedSize)
	{
		ColorArray.SetNum(ExpectedSize);
	}

	int32 UpdatedCount{ 0 };
	for (FColor& Color : ColorArray)
	{
		Color.B = FMath::RandRange(0, 255);
		UpdatedCount++;
	}

	return UpdatedCount;
}

void UDynamicTextureComponent::DrawLineTest(FIntVector2 StartPos, FIntVector2 EndPos, UPARAM(ref) TArray<FColor>& ColorArray)
{
	for (FColor& Color : ColorArray)
	{
		Color.R = 0;
		Color.G = 0;
		Color.B = 0;
	}

	BresenhamAlgo LineGenerator{ StartPos, EndPos };
	FIntVector2 Pos = LineGenerator.GetNext();
	if (int32 Index = Pos.Y * TextureWidth + Pos.X; Index >= 0 && Index < ColorArray.Num())
	{
		ColorArray[Index].R = 255;
	}


	const FIntVector2 Distance{ EndPos - StartPos };
	for (bool bEnd = false; bEnd == false /*&& DrawCount < FMath::Abs(Distance.X) + FMath::Abs(Distance.Y)*/; Pos = LineGenerator.GetNext())
	{
		//DrawCount++;
		if (Pos == EndPos) { bEnd = true; }

		const int32 Index = Pos.Y * TextureWidth + Pos.X;
		if (Index < 0 || Index >= ColorArray.Num())
		{
			return;
		}

		ColorArray[Index].R = 255;
	}

}

bool UDynamicTextureComponent::Equals(const TArray<FColor>& Arr1, const TArray<FColor>& Arr2) const
{
	if (Arr1.Num() * 32 != Arr2.Num() && Arr1.Num() != Arr2.Num() * 32)
	{
		return false;
	}

	const TArray<FColor>& Common = FMath::Max(Arr1.Num(), Arr2.Num()) == Arr1.Num() ? Arr1 : Arr2;
	const TArray<FColor>& Compressed = FMath::Min(Arr1.Num(), Arr2.Num()) == Arr1.Num() ? Arr1 : Arr2;
	for (int32 i = 0; i < Common.Num(); i += 32)
	{
		for (int8 j = 0; j < 32; j++)
		{
			const int32 IndexForCompressed = i;
			const int32 IndexForCommon = IndexForCompressed * 32 + j;

			const FColor& CommonColor = Common[IndexForCommon];
			const FColor& CompressedColor = Compressed[IndexForCompressed];

			const uint8* Member = &CompressedColor.R;
			if (8 <= j && j < 16) { Member = &CompressedColor.G; }
			else if (16 <= j && j < 24) { Member = &CompressedColor.B; }
			else if (24 <= j && j < 32) { Member = &CompressedColor.A; }

			int32 ValueOfCommon = CommonColor.B;

			uint8 Offset = 1 << (j % 8);
			int32 ValueOfCompressed = *Member & Offset;

			if ((ValueOfCommon != 0 && ValueOfCompressed != 0) || ValueOfCommon == ValueOfCompressed)
			{
				return true;
			}
		}
	}

	return false;
}

//void UDynamicTextureComponent::UpdateTextureResource(const TArray<FColor>& ColorArray)
//{
//	FTexture2DMipMap& Mip = DynamicTexture->GetPlatformData()->Mips[0];
//	void* TextureData = Mip.BulkData.Lock(LOCK_READ_WRITE);
//
//	if (TextureData)
//	{
//		FMemory::Memcpy(TextureData, ColorArray.GetData(), ColorArray.Num() * sizeof(FColor));
//
//		Mip.BulkData.Unlock();
//
//		DynamicTexture->UpdateResource();
//	}
//}

bool UDynamicTextureComponent::IsGridInVision(const FIntVector2& GridPos, const TArray<FIntVector2>& VisionStartPos, const TArray<FVector2f>& VisionDirection)
{
	if (VisionStartPos.Num() * 2 != VisionDirection.Num()) { return false; }

	bool bIsVisible{ false };

	for (int32 i = 0; i < VisionStartPos.Num(); i++)
	{
		FVector2f GridDirection{ GridPos - VisionStartPos[i] };

		if (FVector2f::CrossProduct(VisionDirection[i * 2], GridDirection) > 0 && FVector2f::CrossProduct(GridDirection, VisionDirection[i * 2 + 1]) > 0) { return true; }
	}

	return bIsVisible;
}

float UDynamicTextureComponent::GridDistanceToEdge(const FIntVector2& GridPos, const FIntVector2& VisionStartPos, const FVector2f& EdgeVector)
{
	FVector2f GridDirection{ GridPos - VisionStartPos };

	return FMath::Abs(FVector2f::CrossProduct(GridDirection, EdgeVector));
}

float UDynamicTextureComponent::GetNormalizedDistanceToBoundary(const FIntVector2& GridPos, const FIntVector2& VisionStartPos, const FVector2f& EdgeVector, const FVector2f& DirectionToEdge) const
{
	FVector2f GridDirection{ GridPos - VisionStartPos };

	float DistanceToEdge = GridDistanceToEdge(GridPos, VisionStartPos, EdgeVector);

	FVector2f BoundaryVector = GridDirection + DirectionToEdge * GridDistanceToEdge(GridPos, VisionStartPos, EdgeVector);

	float Weight = GridDirection.Length() / BoundaryVector.Length();

	if (Weight > SmoothStepThreshold && Weight < 1.f && DistanceToEdge > SmoothStepDistanceThresholdToEdge)
	{
		return 0.f;
	}

	return Weight;
}

BresenhamAlgo::BresenhamAlgo(FIntVector2 StartPos, FIntVector2 EndPos)
	: EndPos(EndPos)
	//: StartPos(StartPos), EndPos(EndPos)
{
	/*FIntVector2 Diff{ EndPos - StartPos };
	Steep = FMath::Abs(Diff.Y) > FMath::Abs(Diff.X);

	if (Steep)
	{
		Swap(StartPos.X, StartPos.Y);
		Swap(EndPos.X, EndPos.Y);
	}

	if (StartPos.X > EndPos.X)
	{
		Swap(StartPos.X, EndPos.X);
		Swap(StartPos.Y, EndPos.Y);
	}

	Error = 0;

	DeltaError = UKismetMathLibrary::SafeDivide(EndPos.X - StartPos.X, FMath::Abs(EndPos.Y - StartPos.Y));

	StepX = 1;
	if (StartPos.X >= EndPos.X) { StepX = -1; }

	StepY = 1;
	if (StartPos.Y >= EndPos.Y) { StepY = -1; }

	CurrentX = StartPos.X;
	CurrentY = StartPos.Y;*/

	const FIntVector2 Distance{ EndPos - StartPos };

	DistanceX = Distance.X;
	DistanceY = Distance.Y;

	StepX = DistanceX >= 0 ? 1 : -1;
	StepY = DistanceY >= 0 ? 1 : -1;

	DistanceX = FMath::Abs(DistanceX);
	DistanceY = FMath::Abs(DistanceY);

	if (DistanceX > DistanceY) { P = 2 * DistanceY - DistanceX; }
	else { P = 2 * DistanceX - DistanceY; }

	CurrentX = StartPos.X;
	CurrentY = StartPos.Y;
}

FIntVector2 BresenhamAlgo::GetNext()
{
	/*FIntVector2 Result{ CurrentX , CurrentY };
	if (Steep) { Swap(Result.X, Result.Y); }

	Error += DeltaError;
	if (Error >= .5f)
	{
		CurrentY += StepY;
		Error -= 1.f;
	}
	CurrentX += StepX;

	return Result;*/

	FIntVector2 Result{ CurrentX , CurrentY };
	if (Result == EndPos) { return EndPos; }

	if (DistanceX > DistanceY)
	{
		if (P > 0)
		{
			if (CurrentY != EndPos.Y) { CurrentY += StepY; }
			P -= 2 * DistanceX;
		}
		P += 2 * DistanceY;

		if (CurrentX != EndPos.X) { CurrentX += StepX; }
	}
	else
	{
		if (P > 0)
		{
			if (CurrentX != EndPos.X) { CurrentX += StepX; }
			P -= 2 * DistanceY;
		}
		P += 2 * DistanceX;

		if (CurrentY != EndPos.Y) { CurrentY += StepY; }
	}

	return Result;
}