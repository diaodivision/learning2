// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TimerHandle.h"
#include "Interface/BulletInterface.h"
#include "Interface/FreezableInterface.h"
#include "BulletBase.h"
#include "FireArmBulletBase.generated.h"

class UStaticMeshComponent;
class UProjectileMovementComponent;
class USphereComponent;
class UGameplayEffect;
class UFreezableNiagaraComponent;

USTRUCT(BlueprintType)
struct FBulletAttributeData : public FTableRowBase
{
	GENERATED_BODY()

	friend class AWeaponActorBase;

public:

	FBulletAttributeData() = default;
	FBulletAttributeData(float Damage, float Penetration);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Attribute")
	float Damage{ 0.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Attribute")
	float Penetration{ 0.f };
};

UCLASS(BlueprintType, Blueprintable)
class LEARNING2_API AFireArmBulletBase : public ABulletBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	//AFireArmBulletBase(const FObjectInitializer& ObjectInitializer);
	AFireArmBulletBase();

	//UE_DEPRECATED(5.3, "Unsafe constructor!")
	//	AFireArmBulletBase(FBulletAttributeData InBulletData);

	virtual void InitializeBulletData_Implementation(const UObject* InData) override;

	virtual void Freeze_Implementation() override;
	virtual void Unfreeze_Implementation() override;
	virtual FORCEINLINE bool IsFreezing_Implementation() override { return bIsFreezing; };

	virtual void UpdateFogOfWarTexture_Implementation(UTexture2D* FogOfWarTexture) override;

protected:
	// Called when the game starts or when spawned
	//virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	virtual void PostRegisterAllComponents() override;
	virtual void PostUnregisterAllComponents() override;
	virtual void K2_DestroyActor() override;

	virtual void PreInitializeComponents() override;

	void CreateCollisionComponent();
	void CreateMeshComponent();
	void CreateProjectileMovement();
	void CreateNiagaraComponent();
	void InitCollisionComponent();
	void InitProjectileMovement();

	//#if WITH_EDITOR
	//	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	//	virtual void PostCDOContruct() override;
	//#endif
	UFUNCTION(BlueprintNativeEvent, Category = "Life Time")
	void OnReachLifeTime();
	virtual void OnReachLifeTime_Implementation();

	virtual void PostInitializedBulletData_Implementation() override;

protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, meta = (ExposeOnSpawn = "true"), Category = "Life Time")
	float LifeTime{ 10.f };

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, meta = (ExposeOnSpawn = "true"), Category = "Movement")
	float Speed{ 3000.f };

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, meta = (ExposeOnSpawn = "true"), Category = "Physics")
	bool bEnableGravity{ false };

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, meta = (ExposeOnSpawn = "true"), Category = "Components")
	FBulletAttributeData BulletData;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, meta = (ExposeOnSpawn = "true"), Category = "Components")
	FVector Direction{ FVector::ForwardVector };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UFreezableNiagaraComponent> NiagaraComponent;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bIsDebug{ false };
#endif

private:
	FTimerHandle TimerHandle;

	bool bIsFreezing{ false };
};
