#include "FirearmActorBase.h"
#include "Bullet/FireArmBulletBase.h"
#include "Bullet/BulletBaseTypes.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/Character.h"
#include "BlueprintFunctionLibrary/WeaponActorBlueprintLibrary.h"

void AFirearmActorBase::OnControl_Implementation(UObject* InOwner)
{
	Super::OnControl_Implementation(InOwner);

	if (!AvatarAbilitySystemComponent.IsValid()) { return; }

	ReloadAbilityHandle = AvatarAbilitySystemComponent->K2_GiveAbility(ReloadAbilityClass, GetWeaponLevel());
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
	BulletInitData->DamageClass = DamageClass;

	Bullet->SetInstigator(Cast<APawn>(GetOwner()));
	IBulletInterface::Execute_InitializeBulletData(Bullet, BulletInitData);
	UWeaponActorBlueprintLibrary::FinishSpawningOfPoolingActor(Bullet);

	return Bullet;
}
