// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TimerHandle.h"
#include "BulletBaseTypes.h"
#include "Interface/BulletInterface.h"
#include "Interface/FreezableInterface.h"
#include "BulletBase.h"
#include "GrenadeBulletBase.generated.h"

class UStaticMeshComponent;
class UProjectileMovementComponent;
class UTimelineComponent;
class UGameplayEffect;
class UCurveFloat;
class UFreezableNiagaraComponent;

//float DistanceToTarget{ 0.f };
//{
//	FVector CursorLocation;
//	UWeaponActorBlueprintLibrary::GetLocationUnderCursorOnGround(CursorLocation, PlayerController);
//	DistanceToTarget = FVector::Distance(CursorLocation, GetActorLocation());
//}

UCLASS(Blueprintable, BlueprintType)
class LEARNING2_API AGrenadeBulletBase : public ABulletBase
{
	GENERATED_BODY()

	friend class UWeaponActorBlueprintLibrary;

public:
	AGrenadeBulletBase();

	FORCEINLINE FName GetBulletName() const { return BulletName; }
	virtual void InitializeBulletData_Implementation(const UObject* InData) override;

	UFUNCTION(BlueprintPure, Category = "Attribute")
	FORCEINLINE float GetDamage() const { return FMath::Max(0.f, GrenadeBulletData.Damage); }
	UFUNCTION(BlueprintPure, Category = "Attribute")
	FORCEINLINE float GetPenetration() const { return FMath::Max(0.f, GrenadeBulletData.Penetration); }
	UFUNCTION(BlueprintPure, Category = "Attribute")
	FORCEINLINE float GetEffectiveRange() const { return FMath::Max(0.f, GrenadeBulletData.EffectiveRange); }
	UFUNCTION(BlueprintPure, Category = "Attribute")
	FORCEINLINE float GetEffectDuration() const { return FMath::Max(0.f, GrenadeBulletData.EffectDuration); }

	UFUNCTION(BlueprintPure, Category = "Attribute")
	FORCEINLINE FVector GetDirection() const
	{
		return GrenadeBulletData.Direction.IsNormalized() ? GrenadeBulletData.Direction : GrenadeBulletData.Direction.GetSafeNormal();
	}
	UFUNCTION(BlueprintPure, Category = "Attribute")
	inline float GetSpeedRate() const { return FMath::Max(0.f, GrenadeBulletData.SpeedRate); }
	UFUNCTION(BlueprintPure, Category = "Attribute")
	inline float GetLifeTime() const { return FMath::Max(0.f, GrenadeBulletData.LifeTime); }
	UFUNCTION(BlueprintPure, Category = "Attribute")
	FORCEINLINE UCurveFloat* GetGrenadeBulletSpeedFloatCurve() const { return GrenadeBulletData.GrenadeBulletSpeedFloatCurve; }

	UCurveFloat* GetGrenadeBulletSpeedFloatCurveFromTable() const;

	virtual void Freeze_Implementation() override;
	virtual void Unfreeze_Implementation() override;
	virtual FORCEINLINE bool IsFreezing_Implementation() override { return bIsFreezing; };

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PostRegisterAllComponents() override;
	virtual void PostUnregisterAllComponents() override;

	//void InitializeBulletData();

	//virtual void Tick(float DeltaSeconds) override;
	UFUNCTION(BlueprintNativeEvent)
	void UpdateBulletVolecity(float NewVelocityRate);
	virtual void UpdateBulletVolecity_Implementation(float NewVelocityRate);

	void CreateMeshComponent();
	void CreateProjectileMovement();
	void CreateTimelineComponent();
	void CreateNiagaraComponent();
	//void InitCollisionComponent();
	void InitProjectileMovement();

	UFUNCTION(BlueprintNativeEvent, Category = "Life Time")
	void OnReachLifeTime();
	virtual void OnReachLifeTime_Implementation();

	UFUNCTION(BlueprintNativeEvent)
	void OnSystemFinished(UNiagaraComponent* PSystem);
	virtual void OnSystemFinished_Implementation(UNiagaraComponent* PSystem);

	void InitializeDelegates();
	void DeinitializeDelegates();

	virtual void PostInitializedBulletData_Implementation() override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute | Name")
	FName BulletName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTimelineComponent> TimelineComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UFreezableNiagaraComponent> NiagaraComponent;

	UPROPERTY(BlueprintReadOnly, meta = (ExposeOnSpawn = "true"), Category = "Components", meta = (AllowPrivateAccess = true))
	FGrenadeBulletAttributeData GrenadeBulletData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute | DataTable")
	TObjectPtr<const UDataTable> AttributeDataTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Niagara|Parameter")
	FName ScaleParameterName{ "Scale" };

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bIsDebug{ false };
#endif

private:
	FTimerHandle TimerHandle;

	bool bIsFreezing{ false };
};