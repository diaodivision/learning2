#include "FirearmActorBase.h"
#include "Bullet/Base/FireArmBulletBase.h"
#include "Bullet/Base/BulletBaseTypes.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/Character.h"
#include "BlueprintFunctionLibrary/WeaponActorBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

void AFirearmActorBase::OnControl_Implementation(UObject* InOwner)
{
	Super::OnControl_Implementation(InOwner);

	if (!AvatarAbilitySystemComponent.IsValid()) { return; }

	if (ReloadAbilityClass.InstancingPolicy == EWeaponAbilityInstancingPolicy::InstancedOnPossession)
	{
		ReloadAbilityHandle = AvatarAbilitySystemComponent->K2_GiveAbility(ReloadAbilityClass.AbilityClass, GetWeaponLevel());
	}
}

void AFirearmActorBase::OnControlReleased_Implementation()
{
	Super::OnControlReleased_Implementation();

	if (AvatarAbilitySystemComponent.IsValid() && ReloadAbilityHandle.IsValid()) { return; }

	AvatarAbilitySystemComponent->ClearAbility(ReloadAbilityHandle);
	ReloadAbilityHandle = FGameplayAbilitySpecHandle{};
}

bool AFirearmActorBase::Reload() const
{
	if (AvatarAbilitySystemComponent.IsValid()) { return  AvatarAbilitySystemComponent->TryActivateAbility(ReloadAbilityHandle); }

	return false;
}

void AFirearmActorBase::NotifyShootCooldownFinished() const
{
	OnShootCooldownFinishedDelegate.Broadcast();
}

void AFirearmActorBase::NotifyExecutingShoot()
{
	OnShoot();
}

void AFirearmActorBase::NotifyShootFinish()
{
	OnShootStop();
}

bool AFirearmActorBase::ExecuteFireOnce()
{
	if (!AvatarAbilitySystemComponent.IsValid()) { return false; }

	OnShoot();
	const FGameplayAbilitySpec* Spec{ AvatarAbilitySystemComponent->FindAbilitySpecFromHandle(FireAbilityHandle) };
	if (const UGameplayAbility* Instance{ Spec->GetPrimaryInstance() }; Instance && Instance->IsActive()){ return true; }
	else { return AvatarAbilitySystemComponent->TryActivateAbility(FireAbilityHandle); }
}

AActor* AFirearmActorBase::SpawnBullet_Implementation() const
{
	AFireArmBulletBase* Bullet = Cast<AFireArmBulletBase>(UWeaponActorBlueprintLibrary::SpawnActorDeferredFromPool(this, BulletClass, GetActorTransform()));
	if (!Bullet) { return nullptr; }

	UBulletBaseInitData* BulletInitData = NewObject<UBulletBaseInitData>();
	BulletInitData->Damage = GetDamage();
	BulletInitData->Penetration = GetPenetration();
	BulletInitData->Speed = GetMuzzleSpeed();
	BulletInitData->Direction = RandomSpread();
	BulletInitData->bEnableGravity = false;
	BulletInitData->EffectClass = EffectClass;

	Bullet->SetInstigator(Cast<APawn>(GetOwner()));
	IBulletInterface::Execute_InitializeBulletData(Bullet, BulletInitData);
	UWeaponActorBlueprintLibrary::FinishSpawningOfPoolingActor(Bullet);

	return Bullet;
}

void AFirearmActorBase::InstantiateAbilityOnBeginPlay()
{
	Super::InstantiateAbilityOnBeginPlay();

	if (ReloadAbilityClass.InstancingPolicy == EWeaponAbilityInstancingPolicy::InstancedOnAddition)
	{
		ReloadAbilityHandle = AvatarAbilitySystemComponent->K2_GiveAbility(ReloadAbilityClass.AbilityClass, GetWeaponLevel());
	}
}