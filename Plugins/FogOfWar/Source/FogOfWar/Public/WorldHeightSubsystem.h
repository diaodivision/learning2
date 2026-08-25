#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "FogOfWarComponentStatics.h"
#include "FogOfWarTypes.h"
// #include "RenderGraphFwd.h"
#include "WorldHeightSubsystem.generated.h"

class AWorldHeightVolume;
class AActor;
class UWorld;
class ALandscape;
class UTexture2D;

UCLASS()
class FOGOFWAR_API UWorldHeightSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual FORCEINLINE bool ShouldCreateSubsystem(UObject* Outer) const override;

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	void RequestUpdateWorldHeightData(const AWorldHeightVolume& Volume);
	void RequestUpdateWorldHeightData(const AActor& OtherActor, FWorldHeightBoundsUpdateRequest::Type RequestType);

	TOptional<FGridSizeType> GetGridSize() const;

	FogOfWarTypes::GridIndexType GetGridIndex(const FVector2D& Location2D, const EAllowMinusPosition AllowMinusPosition = EAllowMinusPosition::No) const;
	FORCEINLINE FogOfWarTypes::GridIndexType GetGridIndex(const FVector& Location, const EAllowMinusPosition AllowMinusPosition = EAllowMinusPosition::No) const
	{
		if (Location.Z < LandBounds.GetValue().Max.Z) { return INDEX_NONE; }
		return GetGridIndex(FVector2D{ Location.X, Location.Y }, AllowMinusPosition);
	}
	TOptional<FIntPoint> GetGridPosition(const FVector2D& WorldLocation, const EAllowMinusPosition AllowMinusPosition = EAllowMinusPosition::No) const;
	FORCEINLINE TOptional<FIntPoint> GetGridPosition(const FVector& WorldLocation, const EAllowMinusPosition AllowMinusPosition = EAllowMinusPosition::No) const
	{ 
		if (WorldLocation.Z < LandBounds.GetValue().Max.Z) { return NullOpt; }
		return GetGridPosition(FVector2D{ WorldLocation.X, WorldLocation.Y }, AllowMinusPosition);
	}
	// TOptional<FVector> GetGridLocationByIndex(const FogOfWarTypes::GridIndexType Index, const EAllowMinusPosition AllowMinusPosition = EAllowMinusPosition::No) const;

	bool IsWorldHeightVolumeOverlapWithGround(const AWorldHeightVolume& Volume) const;

	virtual void OnWorlHeightVolumeRegisteredComponents(AWorldHeightVolume& Volume);
	virtual void OnWorlHeightVolumeUnregisteredComponents(AWorldHeightVolume& Volume);

	FORCEINLINE const FWorldHeightData::FWorldHeightMapType& GetWorldHeightMap() const { return WorldHeightData.WorldHeightMap; };
	FORCEINLINE uint32 GetWorldHeightDataVersion() const { return WorldHeightDataVersion; };

	FORCEINLINE TOptional<FBox2D> GetLandBoundingBox() const
	{
		if (!LandBounds.IsSet()) { return NullOpt; }

		return FBox2D{ FVector2D{ LandBounds.GetValue().Min }, FVector2D{ LandBounds.GetValue().Max } };
	}

	static bool CanAddActor(const AActor& Actor);

	const UTexture2D* GetWorldHeightTexture() 
	{ 
		if (!WorldHeightTexture) { CreateWorldHeightTexture(); }
		UpdateWorldHeightTexture();
		return WorldHeightTexture;
	}

	const FTextureRHIRef* GetWorldHeightTextureRef();

protected:
	void HandleWorldHeightVolumeInUpdateRequest();

	UFUNCTION()
	virtual void UpdateWorldHeightData();
	void UpdateWorldHeightData_Internal(const AWorldHeightVolume& Volume);
	void UpdateWorldHeightData_Internal(const AActor& Actor, FWorldHeightBoundsUpdateRequest::Type RequestType);
	void NotifyCleanInvalidData_Internal();
	void CleanInvalidData_Internal();
	virtual void OnActorRegisteredComponents(AActor* Actor);
	virtual void OnLandRegisteredComponents(AActor& InLand);
	virtual void InitializeDelegates();
	virtual void DeinitializeDelegates();

	UFUNCTION()
	virtual void OnActorDestroyed(AActor* Actor);

	void UpdateWorldHeightTexture();

	TOptional<FIntPoint> GetGridNum() const;

private:
	void CreateWorldHeightTexture();

#if WITH_EDITOR
protected:
	void DrawVisualization() const;

	virtual void OnActorMoved(AActor* Actor);

	virtual void OnActorRegistered(AActor* Actor);
	virtual void OnActorUnregistered(AActor* Actor);
#endif

public:
	UPROPERTY()
	TArray<TWeakObjectPtr<const AWorldHeightVolume>> WorldHeightVolumes;

	UPROPERTY()
	TArray<FWorldHeightBoundsUpdateRequest> PendingWorldHeightBoundsUpdates;

	UPROPERTY(EditAnywhere, Category = Rendering, meta = (UIMin = "0", ClampMin = "0"))
	float BoxDefaultHeight{ 100.f };

	UPROPERTY(EditAnywhere, Category = Rendering, meta = (UIMin = "0", UIMax = "1", ClampMin = "0", ClampMax = "1"))
	float BoxDefaultSize{ .5f };

private:
	UPROPERTY();
	FWorldHeightData WorldHeightData;
	uint32 WorldHeightDataVersion{ 0 };

	UPROPERTY()
	TWeakObjectPtr<AActor> Land;
	TOptional<FGridBounds> LandBounds;

    UPROPERTY()
    TObjectPtr<UTexture2D> WorldHeightTexture;
	uint32 WorldHeightTextureVersion{ 0 };

	FTimerHandle TimerHandle;
	FTimerHandle CleanInvalidDataTimerHandle;

	FDelegateHandle PostLandActorRegisteredComponentsDelegateHandle;
	FDelegateHandle OnActorDestroyedDelegateHandle;
};