// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Tickable.h"
#include "TopDownCameraSubsystem.generated.h"

class ACameraBoundsVolume;
class ATopDownCameraActor;
class AGameModeBase;
class APlayerController;

DECLARE_MULTICAST_DELEGATE(FOnViewportSizeChangedDelegate);

/**
 *
 */
UCLASS()
class TOPDOWNCAMERASYSTEM_API UTopDownCameraSubsystem : public UWorldSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	virtual FORCEINLINE bool IsTickable() const override { return !IsTemplate(); }//不是CDO才Tick
	virtual FORCEINLINE TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UTopDownCameraSubsystem, STATGROUP_Tickables); }

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	const ACameraBoundsVolume* GetCameraBoundsVolume();

	FORCEINLINE TOptional<FVector2D> GetScreenSize() const { return ViewportInfo.ScreenSize; }

	void CameraMoveTo(const FVector& TargetLocation);

	// UFUNCTION(BlueprintCallable)
	// void SetCameraWidth(const float Width);

protected:
	virtual void InitializeViewportInfo();

	virtual void OnViewportResized(FViewport* Viewport, uint32 Unused);

	UFUNCTION()
	void OnPossessedPawnChanged(APawn* InOldPawn, APawn* InNewPawn);

	virtual void SetupCameraForPlayerController(APlayerController* NewPlayerController);


	virtual void Tick(float DeltaTime) override;

private:
	void OnGameModePostLogin(AGameModeBase* GameMode, APlayerController* NewPlayer);

private:
	UPROPERTY()
	TObjectPtr<ATopDownCameraActor> CameraActor;

	UPROPERTY()
	TWeakObjectPtr<ACameraBoundsVolume> CameraBoundsVolume;

	inline static constexpr float kSpeed{ 1000.f };
	// inline static constexpr float kSpeed{ 100.f };
	inline static constexpr float kThreshold{ .95f };

private:
	struct FViewportInfo
	{
		FORCEINLINE bool IsSet() const;
		FORCEINLINE void Reset();

		TWeakObjectPtr<UGameViewportClient> ViewportClient;

		TOptional<FVector2D> ScreenSize;
	};

	FViewportInfo ViewportInfo;
};

bool UTopDownCameraSubsystem::FViewportInfo::IsSet() const
{
	return ViewportClient.IsValid() && ScreenSize.IsSet();
}

void UTopDownCameraSubsystem::FViewportInfo::Reset()
{
	ViewportClient.Reset();
	ScreenSize.Reset();
}