#include "GrenadeBulletBase.h"
#include "BlueprintFunctionLibrary/WeaponActorBlueprintLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "BulletBaseTypes.h"
#include "FreezableComponent/FreezableNiagaraComponent.h"
#include "WorldPauseSubsystem.h"

AGrenadeBulletBase::AGrenadeBulletBase()
{
	PrimaryActorTick.bCanEverTick = false;

	CreateMeshComponent();
	CreateProjectileMovement();
	CreateTimelineComponent();
	CreateNiagaraComponent();
}

void AGrenadeBulletBase::InitializeBulletData_Implementation(const UObject* InData)
{
	const UGrenadeBulletBaseInitData* Data = Cast<UGrenadeBulletBaseInitData>(InData);
	if (!ensure(Data)) { return; }

	GrenadeBulletData.Damage = Data->Damage;
	GrenadeBulletData.Penetration = Data->Penetration;
	GrenadeBulletData.Direction = Data->Direction.GetSafeNormal();
	GrenadeBulletData.SpeedRate = Data->SpeedRate;
	GrenadeBulletData.LifeTime = Data->LifeTime;
	EffectClass = Data->EffectClass;

	UWeaponActorBlueprintLibrary::GetGrenadeBulletAttributeFromDataTable(GrenadeBulletData, this);

	IBulletInterface::Execute_PostInitializedBulletData(this);
}

inline UCurveFloat* AGrenadeBulletBase::GetGrenadeBulletSpeedFloatCurveFromTable() const
{
	FGrenadeBulletAttributeData GrenadeBulletData_Temp;

	UWeaponActorBlueprintLibrary::GetGrenadeBulletAttributeFromDataTable(GrenadeBulletData_Temp, this);
	return GrenadeBulletData_Temp.GrenadeBulletSpeedFloatCurve;
}

void AGrenadeBulletBase::Freeze_Implementation()
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

void AGrenadeBulletBase::Unfreeze_Implementation()
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

void AGrenadeBulletBase::BeginPlay()
{
	Super::BeginPlay();

	InitProjectileMovement();

	FTimerHandle Handle;
	FTimerDynamicDelegate Delegate;
	Delegate.BindDynamic(this, &AGrenadeBulletBase::OnReachLifeTime);
	GetWorld()->GetTimerManager().SetTimer(Handle, Delegate, FMath::Max(.01f, GrenadeBulletData.LifeTime), false, -1.f);

	if (GetInstigator())
	{
		MeshComponent->IgnoreActorWhenMoving(GetInstigator(), true);
	}

	InitializeDelegates();
	TimelineComponent->Play();
}

void AGrenadeBulletBase::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();

	UWorldPauseSubsystem* WorldPauseSubsystem{ GetWorld()->GetSubsystem<UWorldPauseSubsystem>() };
	if (!WorldPauseSubsystem) { return; }

	WorldPauseSubsystem->OnFreezableObjectRegistered(this);
}

void AGrenadeBulletBase::PostUnregisterAllComponents()
{
	Super::PostUnregisterAllComponents();

	UWorldPauseSubsystem* WorldPauseSubsystem{ GetWorld()->GetSubsystem<UWorldPauseSubsystem>() };
	if (!WorldPauseSubsystem) { return; }

	WorldPauseSubsystem->OnFreezableObjectUnregistered(this);
}

//void AGrenadeBulletBase::Tick(float DeltaSeconds)
//{
//
//}

void AGrenadeBulletBase::UpdateBulletVolecity_Implementation(float NewVelocityRate)
{
	ProjectileMovement->Velocity = ProjectileMovement->Velocity.GetSafeNormal() * NewVelocityRate * GrenadeBulletData.SpeedRate;
	ProjectileMovement->UpdateComponentVelocity();
}

void AGrenadeBulletBase::CreateMeshComponent()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletMesh"));
	RootComponent = MeshComponent;
	MeshComponent->PrimaryComponentTick.bCanEverTick = false;

	MeshComponent->SetEnableGravity(false);

	////ÉèÖÃÅö×²Ô¤Éè
	MeshComponent->SetNotifyRigidBodyCollision(false);
	MeshComponent->SetGenerateOverlapEvents(false);
	MeshComponent->SetCollisionProfileName(UCollisionProfile::CustomCollisionProfileName);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
	MeshComponent->SetNotifyRigidBodyCollision(true);
	MeshComponent->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeshComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	MeshComponent->SetCollisionResponseToChannel(ECC_Destructible, ECR_Block);
}

void AGrenadeBulletBase::CreateProjectileMovement()
{
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	//ProjectileMovement->PrimaryComponentTick.bCanEverTick = false;
	ProjectileMovement->SetUpdatedComponent(RootComponent);
	ProjectileMovement->bRotationFollowsVelocity = false;
	ProjectileMovement->bInitialVelocityInLocalSpace = false;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 1.f;
	ProjectileMovement->Friction = 0.f;
	ProjectileMovement->BounceVelocityStopSimulatingThreshold = 5.f;
	ProjectileMovement->bForceSubStepping = true;
}

void AGrenadeBulletBase::CreateTimelineComponent()
{
	TimelineComponent = CreateDefaultSubobject<UTimelineComponent>(TEXT("TimelineComponent"));
	//TimelineComponent->PrimaryComponentTick.bCanEverTick = false;
}

void AGrenadeBulletBase::CreateNiagaraComponent()
{
	NiagaraComponent = CreateDefaultSubobject<UFreezableNiagaraComponent>(TEXT("NiagaraComponent"));
	NiagaraComponent->SetupAttachment(RootComponent);
	NiagaraComponent->SetAutoActivate(false);
}

void AGrenadeBulletBase::InitProjectileMovement()
{
	{
		float SpeedMin{ GrenadeBulletData.SpeedRate };
		float SpeedMax{ GrenadeBulletData.SpeedRate };
		GrenadeBulletData.GrenadeBulletSpeedFloatCurve->GetValueRange(SpeedMin, SpeedMax);

		ProjectileMovement->InitialSpeed = GrenadeBulletData.SpeedRate * GrenadeBulletData.GrenadeBulletSpeedFloatCurve->GetFloatValue(0);;
		ProjectileMovement->MaxSpeed = GrenadeBulletData.SpeedRate * SpeedMax;
	}

	ProjectileMovement->Velocity = GrenadeBulletData.Direction * ProjectileMovement->InitialSpeed;
	SetActorRotation(GrenadeBulletData.Direction.Rotation());
}

void AGrenadeBulletBase::OnReachLifeTime_Implementation()
{
	if (NiagaraComponent->IsActive() && NiagaraComponent->IsComplete())
	{
		OnSystemFinished(NiagaraComponent);
		return;
	}

	NiagaraComponent->OnSystemFinished.AddUniqueDynamic(this, &AGrenadeBulletBase::OnSystemFinished);

	if (!NiagaraComponent->IsActive()) { NiagaraComponent->Activate(); }
}

void AGrenadeBulletBase::OnSystemFinished_Implementation(UNiagaraComponent* PSystem)
{
	DeinitializeDelegates();
	Destroy();
}

void AGrenadeBulletBase::InitializeDelegates()
{
	FOnTimelineFloat UpdateFunctionFloat;
	UpdateFunctionFloat.BindDynamic(this, &AGrenadeBulletBase::UpdateBulletVolecity);
	if (GrenadeBulletData.GrenadeBulletSpeedFloatCurve)
	{
		{
			float MinTime, MaxTime;
			GrenadeBulletData.GrenadeBulletSpeedFloatCurve->GetTimeRange(MinTime, MaxTime);
			const float TimeRange{ FMath::Max(.0f, MaxTime - MinTime) };

			TimelineComponent->SetPlayRate(GrenadeBulletData.LifeTime > 0.f ? TimeRange / GrenadeBulletData.LifeTime : 1.f);
		}

		TimelineComponent->AddInterpFloat(GrenadeBulletData.GrenadeBulletSpeedFloatCurve, UpdateFunctionFloat);
	}
}

void AGrenadeBulletBase::DeinitializeDelegates()
{
}

void AGrenadeBulletBase::PostInitializedBulletData_Implementation()
{
	NiagaraComponent->SetFloatParameter(ScaleParameterName, GetEffectiveRange());
}