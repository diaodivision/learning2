// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Tickable.h"
#include "CameraSubsystem.generated.h"

class ACameraActor;
class ACameraBoundsVolume;

DECLARE_MULTICAST_DELEGATE(FOnViewportSizeChangedDelegate);

/**
 *
 */
UCLASS()
class TOPDOWNCAMERASYSTEM_API UCameraSubsystem : public ULocalPlayerSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	const ACameraBoundsVolume* GetCameraBoundsVolume();

	FORCEINLINE TOptional<FVector2D> GetScreenSize() const { return ViewportInfo.ScreenSize; }

protected:
	virtual void InitializeViewportInfo();

	virtual void OnViewportResized(FViewport* Viewport, uint32 Unused);

	UFUNCTION()
	void OnPossessedPawnChanged(APawn* InOldPawn, APawn* InNewPawn);

	virtual void PlayerControllerChanged(APlayerController* NewPlayerController) override;


	virtual void Tick(float DeltaTime) override;
	virtual FORCEINLINE bool IsTickable() const override { return !IsTemplate(); }//不是CDO才Tick
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UCameraSubsystem, STATGROUP_Tickables); }

private:
	UPROPERTY()
	TObjectPtr<ACameraActor> CameraActor;

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

bool UCameraSubsystem::FViewportInfo::IsSet() const
{
	return ViewportClient.IsValid() && ScreenSize.IsSet();
}

void UCameraSubsystem::FViewportInfo::Reset()
{
	ViewportClient.Reset();
	ScreenSize.Reset();
}