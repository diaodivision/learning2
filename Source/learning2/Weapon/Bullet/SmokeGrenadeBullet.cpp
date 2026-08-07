#include "SmokeGrenadeBullet.h"
#include "Components/SphereComponent.h"
#include "FreezableComponent/FreezableNiagaraComponent.h"

ASmokeGrenadeBullet::ASmokeGrenadeBullet() : Super()
{
	CreateSphereComponent();
}

void ASmokeGrenadeBullet::CreateSphereComponent()
{
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	RootComponent = SphereComponent;
	SphereComponent->PrimaryComponentTick.bCanEverTick = false;

	SphereComponent->SetEnableGravity(false);

	////ÉèÖÃÅö×²Ô¤Éè
	SphereComponent->SetNotifyRigidBodyCollision(false);
	SphereComponent->SetGenerateOverlapEvents(false);
	SphereComponent->SetCollisionProfileName(UCollisionProfile::CustomCollisionProfileName);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SphereComponent->SetCollisionObjectType(ECC_WorldDynamic);
	SphereComponent->SetNotifyRigidBodyCollision(true);
	SphereComponent->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ASmokeGrenadeBullet::OnReachLifeTime_Implementation()
{
	Super::OnReachLifeTime_Implementation();

	if (NiagaraComponent->IsActive() && !NiagaraComponent->IsComplete())
	{
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ASmokeGrenadeBullet::OnBeginOverlap);
	}
}

void ASmokeGrenadeBullet::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}