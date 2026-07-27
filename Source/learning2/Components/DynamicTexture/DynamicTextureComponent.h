// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include <concepts>
#include "DynamicTextureComponent.generated.h"

class BresenhamAlgo
{
public:
	BresenhamAlgo(FIntVector2 StartPos, FIntVector2 EndPos);

	[[nodiscard]] FIntVector2 GetNext();

private:
	int32 DistanceX : 30;
	int32 DistanceY : 30;
	int16 CurrentX;
	int16 CurrentY;
	int8 StepX : 2;
	int8 StepY : 2;

	int32 P;
	//const FIntVector2 StartPos;
	const FIntVector2 EndPos;

	/*float Error;

	int32 CurrentX;
	int32 CurrentY;
	int8 StepX;
	int8 StepY;

	FIntVector2 StartPos;
	FIntVector2 EndPos;
	bool Steep;
	float DeltaError;*/
};
template<typename T>
concept SupportedType = std::same_as<T, uint8> || std::same_as<T, FColor>;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class LEARNING2_API UDynamicTextureComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UDynamicTextureComponent();
	UDynamicTextureComponent(int32 TextureWidth, int32 TextureHeight);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Dynamic Texture")
	void InitializeTexture(int32 Width, int32 Height, UMaterialInterface* TargetMaterial, FName ParameterName = "DynamicTexture");

	UFUNCTION(BlueprintCallable, Category = "Array", BlueprintPure = false)
	void InitColorArray(UPARAM(ref)TArray<FColor>& ColorArray) const;

	UFUNCTION(BlueprintCallable, Category = "Array", BlueprintPure = false, meta = (DisplayName = "Init Color Array"))
	void InitColorArrayPFG8(UPARAM(ref)TArray<uint8>& ColorArray) const;

	UFUNCTION(BlueprintCallable, Category = "Dynamic Texture")
	void SetFloorComponent(UMeshComponent* InFloorComponent);

	UFUNCTION(BlueprintCallable, Category = "Dynamic Texture")
	void SetPostProcessVolume(APostProcessVolume* InPostProcessVolume);

	UFUNCTION(BlueprintCallable, Category = "Dynamic Texture")
	void UpdateTextureFromArray(const TArray<FColor>& ColorArray);

	UFUNCTION(BlueprintCallable, Category = "Dynamic Texture", meta = (DisplayName = "Update Texture From Array"))
	void UpdateTextureFromArrayPFG8(const TArray<uint8>& ColorArray);

	UFUNCTION(BlueprintCallable, Category = "Test", BlueprintPure = false)
	void CalculateVisionArea(UPARAM(ref)TArray<FColor>& ColorArray, const TArray<FVector2D>& VisionStartPos, TArray<FVector2f> Vision, const FVector2D& GroundOrigin, const FIntVector2& GroundSize) const;

	UFUNCTION(BlueprintCallable, Category = "Test", BlueprintPure = false)
	void CalculateVisionAreaPFG8(UPARAM(ref)TArray<uint8>& ColorArray, const TArray<FVector2D>& VisionStartPos, TArray<FVector2f> Vision, const FVector2D& GroundOrigin, const FIntVector2& GroundSize) const;

	UFUNCTION(BlueprintCallable, Category = "Test", BlueprintPure = false)
	void InitColorArrayCompressed(UPARAM(ref)TArray<FColor>& ColorArray) const;

	UFUNCTION(BlueprintCallable, Category = "Dynamic Texture")
	void UpdateTextureFromArrayCompressed(const TArray<FColor>& ColorArray);

	UFUNCTION(BlueprintCallable, CAtegory = "Test", BlueprintPure = false)
	void CalculateVisionAreaCompressed(UPARAM(ref)TArray<FColor>& ColorArray, const TArray<FVector2D>& VisionStartPos, TArray<FVector2f> Vision, const FVector2D& GroundOrigin, const FIntVector2& GroundSize) const;

	UFUNCTION(BlueprintCallable, Category = "Test")
	int32 RandomArray(UPARAM(ref) TArray<FColor>& ColorArray) const;

	UFUNCTION(BlueprintCallable, Category = "Test")
	void DrawLineTest(FIntVector2 StartPos, FIntVector2 EndPos, UPARAM(ref) TArray<FColor>& ColorArray);

	UFUNCTION(BlueprintCallable, CAtegory = "Test", BlueprintPure = false)
	bool Equals(const TArray<FColor>& Arr1, const TArray<FColor>& Arr2) const;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	template<SupportedType T>
	void UpdateTextureResource(const TArray<T>& ColorArray);
	//void UpdateTextureResource(const TArray<uint8>& ColorArray);

	static bool IsGridInVision(const FIntVector2& GridPos, const TArray<FIntVector2>& VisionStartPos, const TArray<FVector2f>& VisionDirection);

	static bool IsGridVisible(const FColor& ColorNode) { return static_cast<bool>(ColorNode.R); };

	// 令 (VisionStartPos - GridPos) 为向量 a, 向量 a 和单位向量 direction 的叉积为以它们为边的平行四边形的面积.
	// 设 GridPos 到向量 direction 所在直线的距离为 d. 已知上述平行四边形的面积 S = d * |direction|, 又由于 |direction| = 1
	// 所以 S = d
	static float GridDistanceToEdge(const FIntVector2& GridPos, const FIntVector2& VisionStartPos, const FVector2f& EdgeVector);

	float GetNormalizedDistanceToBoundary(const FIntVector2& GridPos, const FIntVector2& VisionStartPos, const FVector2f& EdgeVector, const FVector2f& DirectionToEdge) const;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Texture Size")
	int32 TextureWidth{ 128 };

	UPROPERTY(BlueprintReadOnly, Category = "Texture Size")
	int32 TextureHeight{ 128 };

	UPROPERTY()
	TObjectPtr<UTexture2D> DynamicTexture;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UMaterialInstanceDynamic> MaterialInstanceDynamic;

	UPROPERTY(BlueprintReadWrite, Category = "Mesh Component")
	TWeakObjectPtr<UMeshComponent> FloorMeshComponent;

	UPROPERTY(BlueprintReadWrite, Category = "Post Process Volume")
	TWeakObjectPtr<APostProcessVolume> PostProcessVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loop")
	int32 Loop{ 100 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lerp")
	float SmoothStepThreshold{ .9 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lerp")
	float SmoothStepDistanceThreshold{ 100.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lerp")
	float SmoothStepDistanceThresholdToEdge{ 100.f };
};

template<SupportedType T>
void UDynamicTextureComponent::UpdateTextureResource(const TArray<T>& ColorArray)
{
	FTexture2DMipMap& Mip = DynamicTexture->GetPlatformData()->Mips[0];
	void* TextureData = Mip.BulkData.Lock(LOCK_READ_WRITE);

	if (TextureData)
	{
		FMemory::Memcpy(TextureData, ColorArray.GetData(), ColorArray.Num() * sizeof(T));

		Mip.BulkData.Unlock();

		DynamicTexture->UpdateResource();
	}
}

//template<SupportedType T>
//void UDynamicTextureComponent::UpdateTextureResource(const TArray<T>& ColorArray)
//{
//	if (!DynamicTexture || !DynamicTexture->GetResource()) { return; }
//
//	if constexpr (std::same_as<T, uint8>)
//	{
//		FUpdateTextureRegion2D Region(0, 0, 0, 0, TextureWidth, TextureHeight);
//
//		DynamicTexture->UpdateTextureRegions(0, 1, &Region, TextureWidth, sizeof(uint8), const_cast<uint8*>(ColorArray.GetData()));
//	}
//	else
//	{
//		FTexture2DMipMap& Mip = DynamicTexture->GetPlatformData()->Mips[0];
//		void* TextureData = Mip.BulkData.Lock(LOCK_READ_WRITE);
//
//		if (TextureData)
//		{
//			FMemory::Memcpy(TextureData, ColorArray.GetData(), ColorArray.Num() * sizeof(T));
//
//			Mip.BulkData.Unlock();
//
//			DynamicTexture->UpdateResource();
//		}
//	}
//}