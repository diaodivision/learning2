// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "Delegates/DelegateCombinations.h"
#include "PredictionLineProvider/PredictionLineProviderInterface.h"
#include "GrenadeTargetActor.generated.h"

class UDecalComponent;
class ABulletBase;
class USplineComponent;
class USplineMeshComponent;
class UStaticMesh;

DECLARE_DELEGATE_OneParam(FOnStartLocationUpdatedDelegate, const FVector& NewStartLocation);

/**
 *
 */
UCLASS(Blueprintable, BlueprintType)
class LEARNING2_API AGrenadeTargetActor : public AGameplayAbilityTargetActor, public IPredictionLineProviderInterface
{
	GENERATED_BODY()

public:
	AGrenadeTargetActor();

	virtual void StartTargeting(UGameplayAbility* Ability) override;

	//virtual void ConfirmTargetingAndContinue() override;

	virtual void ConfirmTargetingAndContinue() override;

	virtual void SetActorHiddenInGame(bool bNewHidden) override;

	void SetRadius(const float InRadius);
	FORCEINLINE float GetRadius() const { return Radius; }

	virtual void ShowPredictionLine(const FPredictionLineParams& Params) override;
	virtual void HidePredictionLine() override;
	virtual bool IsPredictionLineVisible() const override;

	void ShowPredictionLineStatic(const FPredictionLineParams& Params, const FVector& TargetLocation);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	virtual void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

	virtual void CalculatePredictionLine(TOptional<FVector> TargetLocation = {});

	void UpdatePredictionLine();

	FCollisionObjectQueryParams GetBulletCollisionObjectQueryParams() const;

public:
	FORCEINLINE void SetPredictionLineMesh(const TSoftObjectPtr<UStaticMesh>& Mesh) { PredictionLineMesh = Mesh; }
	FORCEINLINE TSoftObjectPtr<UStaticMesh> GetPredictionLineMesh() const { return PredictionLineMesh; }

	FORCEINLINE void SetIterationNum(const int32 InIterationNum) { IterationNum = InIterationNum; }
	FORCEINLINE int32 GetIterationNum() const { return IterationNum; }

	FORCEINLINE void SetBulletClass(const TSubclassOf<ABulletBase>& InBulletClass) { BulletClass = InBulletClass; }
	FORCEINLINE TSubclassOf<ABulletBase> GetBulletClass() const { return BulletClass; }

public:
	FOnStartLocationUpdatedDelegate OnStartLocationUpdatedDelegate;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Component", meta = (AllowPrivateAccess = true))
	TObjectPtr<UDecalComponent> DecalComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability Settings", meta = (ExposeOnSpawn = true, AllowPrivateAccess = true))
	float Radius;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USplineComponent> SplineComponent;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TArray<TObjectPtr<USplineMeshComponent>> SplineMeshComponents;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute|PredictionLine|Mesh", meta = (AllowPrivateAccess = true))
	TSoftObjectPtr<UStaticMesh> PredictionLineMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute|PredictionLine", meta = (AllowPrivateAccess = true))
	int32 IterationNum{ 1 };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bullet Class", meta = (AllowPrivateAccess = true))
	TSubclassOf<ABulletBase> BulletClass;
};
