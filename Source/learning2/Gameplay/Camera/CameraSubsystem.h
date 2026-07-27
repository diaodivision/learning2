// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Tickable.h"
#include "CameraSubsystem.generated.h"

class ACameraActor;
class ACameraBoundsVolume;

/**
 *
 */
UCLASS()
class LEARNING2_API UCameraSubsystem : public ULocalPlayerSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	const ACameraBoundsVolume* GetCameraBoundsVolume();
	//UFUNCTION()
	//void OnPlayerControllerSet(ULocalPlayer* InLocalPlayer, APlayerController* InPlayerController);

protected:
	UFUNCTION()
	void OnPossessedPawnChanged(APawn* InOldPawn, APawn* InNewPawn);

	virtual void PlayerControllerChanged(APlayerController* NewPlayerController) override;

	virtual void Tick(float DeltaTime) override;
	virtual FORCEINLINE bool IsTickable() const override { return !IsTemplate(); }//²»ÊÇCDO²ÅTick
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UCameraSubsystem, STATGROUP_Tickables); }

private:
	UPROPERTY()
	TObjectPtr<ACameraActor> CameraActor;

	UPROPERTY()
	TWeakObjectPtr<ACameraBoundsVolume> CameraBoundsVolume;

	inline static constexpr float kSpeed{ 500.f };
	inline static constexpr float kThreshold{ .95f };
};
