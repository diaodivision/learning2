// Fill out your copyright notice in the Description page of Project Settings.


#include "FireArmBulletBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GrenadeBulletBase.h"
#include "BulletBaseTypes.h"
#include "BlueprintFunctionLibrary/WeaponActorBlueprintLibrary.h"
#include "FreezableComponent/FreezableNiagaraComponent.h"
#include "WorldPauseSubsystem.h"

FBulletAttributeData::FBulletAttributeData(float Damage, float Penetration) :Damage(Damage), Penetration(Penetration)
{
}

AFireArmBulletBase::AFireArmBulletBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CreateCollisionComponent();
	RootComponent = CollisionComponent;

	CreateMeshComponent();
	CreateProjectileMovement();
}

void AFireArmBulletBase::InitializeBulletData_Implementation(const UObject* InData)
{
	const UBulletBaseInitData* Data = Cast<UBulletBaseInitData>(InData);
	if (!ensure(Data)) { return; }

	EffectClass = Data->EffectClass;
	//LifeTime = Data->LifeTime;

	Speed = Data->Speed;
	bEnableGravity = Data->bEnableGravity;
	BulletData.Damage = Data->Damage;
	BulletData.Penetration = Data->Penetration;
	Direction = Data->Direction.GetSafeNormal();

	IBulletInterface::Execute_PostInitializedBulletData(this);
}

//void AFireArmBulletBase::Tick(float DeltaSeconds)
//{
//	Super::Tick(DeltaSeconds);
//
//	ExistedTime += DeltaSeconds;
//	if (ExistedTime > LifeTime) { Destroy(); }
//}

void AFireArmBulletBase::Freeze_Implementation()
{
	CustomTimeDilation = 0.f;
	bIsFreezing = true;

#if WITH_EDITOR
	if (!bIsDebug)
	{
#endif
		SetHidden(true);
#if WITH_EDITOR
	}
#endif
}

void AFireArmBulletBase::Unfreeze_Implementation()
{
	CustomTimeDilation = 1.f;
	bIsFreezing = false;

#if WITH_EDITOR
	if (!bIsDebug)
	{
#endif
		SetHidden(false);
#if WITH_EDITOR
	}
#endif
}

void AFireArmBulletBase::BeginPlay()
{
	Super::BeginPlay();

	const UWorld* World = GetWorld();
	if (!ensure(World))
	{
		Destroy();

		return;
	}

	//FTimerDynamicDelegate Delegate;
	//Delegate.BindDynamic(this, &AFireArmBulletBase::OnReachLifeTime);
	//World->GetTimerManager().SetTimer(TimerHandle, Delegate, LifeTime, false, -1.f);
}

void AFireArmBulletBase::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();

	UWorldPauseSubsystem* WorldPauseSubsystem{ GetWorld()->GetSubsystem<UWorldPauseSubsystem>() };
	if (!WorldPauseSubsystem) { return; }

	WorldPauseSubsystem->OnFreezableObjectRegistered(this);
}

void AFireArmBulletBase::PostUnregisterAllComponents()
{
	Super::PostUnregisterAllComponents();

	UWorldPauseSubsystem* WorldPauseSubsystem{ GetWorld()->GetSubsystem<UWorldPauseSubsystem>() };
	if (!WorldPauseSubsystem) { return; }

	WorldPauseSubsystem->OnFreezableObjectUnregistered(this);
}

void AFireArmBulletBase::K2_DestroyActor()
{
	UWeaponActorBlueprintLibrary::ReleaseActorToPool(this);
}

void AFireArmBulletBase::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	InitCollisionComponent();
	InitProjectileMovement();
}

void AFireArmBulletBase::CreateCollisionComponent()
{
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));

	CollisionComponent->PrimaryComponentTick.bCanEverTick = false;
	CollisionComponent->InitSphereRadius(5.f);

	CollisionComponent->SetEnableGravity(bEnableGravity);

	////ÉèÖÃÅö×²Ô¤Éè
	CollisionComponent->SetCollisionProfileName(UCollisionProfile::CustomCollisionProfileName);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetNotifyRigidBodyCollision(true);
	CollisionComponent->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
}

void AFireArmBulletBase::CreateMeshComponent()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->PrimaryComponentTick.bCanEverTick = false;

	MeshComponent->SetRelativeRotation(UKismetMathLibrary::RotatorFromAxisAndAngle(FVector::UpVector, -90.f));
	//MeshComponent->PrimaryComponentTick.bStartWithTickEnabled
	//MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	MeshComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

	MeshComponent->SetupAttachment(RootComponent);
}

void AFireArmBulletBase::CreateProjectileMovement()
{
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->SetUpdatedComponent(CollisionComponent);
	ProjectileMovement->bRotationFollowsVelocity = false;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->bForceSubStepping = true;
	ProjectileMovement->bInitialVelocityInLocalSpace = false;

	//InitProjectileMovement();
}

void AFireArmBulletBase::CreateNiagaraComponent()
{
	NiagaraComponent = CreateDefaultSubobject<UFreezableNiagaraComponent>(TEXT("NiagaraComponent"));
	NiagaraComponent->SetupAttachment(RootComponent);
}

void AFireArmBulletBase::InitCollisionComponent()
{
	if (GetInstigator())
	{
		CollisionComponent->IgnoreActorWhenMoving(GetInstigator(), true);
	}
}

void AFireArmBulletBase::InitProjectileMovement()
{
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	ProjectileMovement->Velocity = Direction * Speed;
	SetActorRotation(Direction.Rotation());
	//ProjectileMovement->Velocity = GetActorForwardVector();
	ProjectileMovement->ProjectileGravityScale = bEnableGravity ? 1.f : 0.f;
}

void AFireArmBulletBase::OnReachLifeTime_Implementation()
{
	K2_DestroyActor();
}



void AFireArmBulletBase::PostInitializedBulletData_Implementation()
{
	InitProjectileMovement();

	if (GetWorld()->GetTimerManager().TimerExists(TimerHandle)) { GetWorld()->GetTimerManager().ClearTimer(TimerHandle); }
	FTimerDynamicDelegate Delegate;
	Delegate.BindDynamic(this, &AFireArmBulletBase::OnReachLifeTime);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, Delegate, LifeTime, false, -1.f);

	if (GetInstigator())
	{
		CollisionComponent->IgnoreActorWhenMoving(GetInstigator(), true);
	}
}
