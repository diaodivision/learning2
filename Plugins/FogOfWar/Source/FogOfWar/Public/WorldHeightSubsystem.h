#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "FogOfWarComponentStatics.h"
#include "FogOfWarTypes.h"
#include "Tickable.h"
#include "WorldHeightSubsystem.generated.h"

class AWorldHeightVolume;
class AActor;
class UWorld;
class ALandscape;

UCLASS()
class FOGOFWAR_API UWorldHeightSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UWorldHeightSubsystem();

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; };

	/**
	* Initializes the world subsystem.
	* Will execute PostInitialize if the world has already been Initialize
	* Will execute OnWorldBeginPlay if the world has already begun play*/
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	void RequestUpdateWorldHeightData(const AWorldHeightVolume& Volume);
	void RequestUpdateWorldHeightData(const AActor& OtherActor, FWorldHeightBoundsUpdateRequest::Type RequestType);

	inline void SetGridNum(FogOfWarTypes::GridNumType InGridX, FogOfWarTypes::GridNumType InGridY)
	{
		GridNumX = InGridX;
		GridNumY = InGridY;

		OnGridSizeUpdated();
	};

	inline void GetGridNum(FogOfWarTypes::GridNumType& OutGridX, FogOfWarTypes::GridNumType& OutGridY) const
	{
		OutGridX = GridNumX;
		OutGridY = GridNumY;
	}

	bool GetGridSize(FVector2D& GridSize) const;
	FogOfWarTypes::GridIndexType GetGridIndex(const FVector2D& Location2D) const;
	FogOfWarTypes::GridIndexType GetGridIndex(const FVector& Location) const;
	bool GetGridLocationByIndex(FVector& Location, const FogOfWarTypes::GridIndexType Index) const;

	bool IsWorldHeightVolumeOverlapWithGround(const AWorldHeightVolume& Volume) const;

	virtual void OnWorlHeightVolumeRegisteredComponents(AWorldHeightVolume& Volume);
	virtual void OnWorlHeightVolumeUnregisteredComponents(AWorldHeightVolume& Volume);

private:
	FWorldHeightData::FWorldHeightMapType& GetWorldHeightMap(const UWorldHeightSubsystem* self);

public:
	inline const FWorldHeightData::FWorldHeightMapType& GetWorldHeightMap() const { return WorldHeightData.WorldHeightMap; };
	inline uint32 GetWorldHeightDataVersion() const { return WorldHeightDataVersion; };

	FORCEINLINE TOptional<FBox2D> GetLandBoundingBox()
	{
		if (!LandBounds.IsSet()) { return NullOpt; }

		return FBox2D{ FVector2D{ LandBounds.GetValue().Min }, FVector2D{ LandBounds.GetValue().Max } };
	}

	static bool CanAddActor(const AActor& Actor);

protected:
#if WITH_EDITOR
	void DrawVisualization() const;
	void DrawVisualization(const FogOfWarTypes::GridIndexType Index) const;
#endif

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

#if WITH_EDITOR
	virtual void OnActorMoved(AActor* Actor);

	virtual void OnActorRegistered(AActor* Actor);
	virtual void OnActorUnregistered(AActor* Actor);
#endif

	UFUNCTION()
	virtual void OnActorDestroyed(AActor* Actor);

	void OnGridSizeUpdated();

	//protected:
public:
	UPROPERTY()
	TArray<TWeakObjectPtr<const AWorldHeightVolume>> WorldHeightVolumes;

	UPROPERTY();
	FWorldHeightData WorldHeightData;
	uint32 WorldHeightDataVersion{ 0 };

	UPROPERTY()
	TArray<FWorldHeightBoundsUpdateRequest> PendingWorldHeightBoundsUpdates;

	UPROPERTY(EditAnywhere, Category = Rendering, meta = (UIMin = "0", ClampMin = "0"))
	float BoxDefaultHeight{ 100.f };

	UPROPERTY(EditAnywhere, Category = Rendering, meta = (UIMin = "0", UIMax = "1", ClampMin = "0", ClampMax = "1"))
	float BoxDefaultSize{ .5f };

private:
	UPROPERTY()
	TWeakObjectPtr<AActor> Land;
	TOptional<FGridBounds> LandBounds;

	FogOfWarTypes::GridNumType GridNumX{ FogOfWarConst::kTextureWidth };
	FogOfWarTypes::GridNumType GridNumY{ FogOfWarConst::kTextureHeight };
	FTimerHandle TimerHandle;
	FTimerHandle CleanInvalidDataTimerHandle;

	FDelegateHandle PostLandActorRegisteredComponentsDelegateHandle;
	FDelegateHandle OnActorDestroyedDelegateHandle;
};