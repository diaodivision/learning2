// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponActorBase.h"
#include "PredictionLineProvider/PredictionLineProviderInterface.h"
#include "GrenadeActorBase.generated.h"

class UCurveFloat;
class USplineComponent;
class USplineMeshComponent;
class AGrenadeBulletBase;
class UGameplayEffect;
class AGrenadeTargetActor;

UCLASS(Blueprintable, BlueprintType)
class LEARNING2_API AGrenadeActorBase : public AWeaponActorBase, public IPredictionLineProviderInterface
{
	GENERATED_BODY()

	friend class UWeaponActorBlueprintLibrary;

public:
	AGrenadeActorBase();

	virtual void OnControl_Implementation(UObject* InOwner) override;
	virtual void OnControlReleased_Implementation() override;

	virtual void CalculatePredictionLine();

	virtual void ShowPredictionLine(const FPredictionLineParams& Params) override;
	virtual void HidePredictionLine() override;
	virtual bool IsPredictionLineVisible() const override;

	FORCEINLINE void OnStartLocationUpdated(const FVector& NewStartLocation)
	{
		if (!IsPredictionLineVisible()) { return; }

		SetBulletSpawnLocatioOffset(NewStartLocation - GetActorLocation());
	}

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetBulletSpawnLocatioOffset(const FVector& Offset) { BulletSpawnLocationOffset = Offset; };
	UFUNCTION(BlueprintPure)
	FORCEINLINE FVector GetBulletSpawnLocatioOffset() const { return BulletSpawnLocationOffset; }

	virtual AActor* SpawnBullet_Implementation() const override;
	UFUNCTION(BlueprintNativeEvent)
	AActor* SpawnBulletByParameters(const FSpawnGrenadeParameters& SpawnGrenadeParameters) const;
	virtual AActor* SpawnBulletByParameters_Implementation(const FSpawnGrenadeParameters& SpawnGrenadeParameters) const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	virtual void Tick(float DeltaTime) override;

	void CreateSplineComponent();

	virtual void InitilizeWeaponAttribute() override;

	virtual void InitialzeDelegates();
	virtual void DeinitialzeDelegates();

	inline FCollisionObjectQueryParams GetBulletCollisionObjectQueryParams() const;

	UFUNCTION()
	virtual void HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

private:
	virtual bool Reload() const override final { return false; }

	virtual int32 CalculateReloadAmount_Implementation() const override final { return 0; };

	virtual inline void SetWeaponLevel(int32 NewLevel) override final { WeaponLevel = 1; }

	virtual inline float GetReloadTime() const override final { return 0.f; }
	virtual inline int32 GetChamberCapacity() const override final { return 0; }
	virtual inline int32 GetReserveAmmo() const override final { return 0; }
	virtual inline float GetAccuracy() const override final { return 0.f; }
	virtual inline void SetReserveAmmo(const int32 NewReserveAmmo) override final {}
	virtual inline bool CanReload_Implementation() const override final { return false; }
	virtual inline bool ShouldRackOnReload_Implementation() const override final { return false; }
	virtual inline void OnWeaponLevelChanged(const int32 OldLevel, const int32 NewLevel) override final {};

protected:
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bullet Class")
	//TSubclassOf<ABulletBase> BulletClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect Class")
	TSubclassOf<UGameplayEffect> EffectClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USplineComponent> SplineComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TArray<TObjectPtr<USplineMeshComponent>> SplineMeshComponents;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute|PredictionLine|Mesh")
	TSoftObjectPtr<UStaticMesh> PredictionLineMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute|PredictionLine")
	int32 IterationNum{ 1 };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bullet Attribute")
	TObjectPtr<UCurveFloat> GrenadeBulletSpeedCurve;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TSubclassOf<AGrenadeTargetActor> GrenadeTargetActorClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<AGrenadeTargetActor> GrenadeTargetActor;

	FVector BulletSpawnLocationOffset{ FVector::ZeroVector };
};