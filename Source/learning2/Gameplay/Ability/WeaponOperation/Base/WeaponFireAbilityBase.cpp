#include "WeaponFireAbilityBase.h"
#include "Character/Base/MyCharacterBase.h"
#include "WeaponBase/WeaponActorBase.h"
#include "AbilitySystemComponent.h"

UWeaponFireAbilityBase::UWeaponFireAbilityBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UWeaponFireAbilityBase::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	const AMyCharacterBase* AvatarCharacter = ActorInfo ? Cast<AMyCharacterBase>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (!AvatarCharacter) { return false; }

	if (const AWeaponActorBase* Weapon = AvatarCharacter->GetControlledWeapon(); Weapon && CanExecuteFire(Weapon))
	{
		return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
	}

	return false;
}

void UWeaponFireAbilityBase::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	const UGameplayEffect* CooldownEffectCDO = GetCooldownGameplayEffect();
	if (!ensure(CooldownEffectCDO)) { return; }

	const AMyCharacterBase* AvatarCharacter = ActorInfo ? Cast<AMyCharacterBase>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (!AvatarCharacter) { return; }

	const AWeaponActorBase* Weapon = AvatarCharacter->GetControlledWeapon();
	if (!Weapon) { return; }

	FGameplayEffectSpecHandle CooldownSpecHandle = MakeOutgoingGameplayEffectSpec(CooldownEffectCDO->GetClass(), GetAbilityLevel(Handle, ActorInfo));
	if (!CooldownSpecHandle.IsValid()) { return; }

	CooldownSpecHandle.Data.Get()->SetSetByCallerMagnitude(FireCooldownTag, 1.f / Weapon->GetFireRate());

	if (UAbilitySystemComponent* AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get())
	{
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*CooldownSpecHandle.Data.Get());
	}
}

bool UWeaponFireAbilityBase::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	const AMyCharacterBase* AvatarCharacter = ActorInfo ? Cast<AMyCharacterBase>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (!AvatarCharacter) { return false; }

	const AWeaponActorBase* Weapon = AvatarCharacter->GetControlledWeapon();
	if (!Weapon) { return false; }

	return CanExecuteFire(Weapon);
}

void UWeaponFireAbilityBase::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	const UGameplayEffect* CostEffectCDO = GetCostGameplayEffect();
	if (!ensure(CostEffectCDO)) { return; }

	const AMyCharacterBase* AvatarCharacter = ActorInfo ? Cast<AMyCharacterBase>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (!AvatarCharacter) { return; }

	const AWeaponActorBase* Weapon = AvatarCharacter->GetControlledWeapon();
	if (!Weapon) { return; }
	
	FGameplayEffectSpecHandle CooldownSpecHandle = MakeOutgoingGameplayEffectSpec(CostEffectCDO->GetClass(), GetAbilityLevel(Handle, ActorInfo));
	if (!CooldownSpecHandle.IsValid()) { return; }
	
	CooldownSpecHandle.Data.Get()->SetSetByCallerMagnitude(FireCostTag, Weapon->GetFireCost());
	CooldownSpecHandle.Data.Get()->SetSetByCallerMagnitude(WeaponSlotTag, static_cast<float>(Weapon->GetWeaponSlot()));
	
	if (UAbilitySystemComponent* AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get())
	{
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*CooldownSpecHandle.Data.Get());
	}
}

bool UWeaponFireAbilityBase::CanExecuteFire(const AWeaponActorBase* Weapon) const
{
	return Weapon && Weapon->GetMagazineAmmo() >= Weapon->GetFireCost();
}

void UWeaponFireAbilityBase::OnExecuteShoot() const
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->AddLooseGameplayTag(ShotCountTag);
	}
}

bool UWeaponFireAbilityBase::CanExecuteShoot() const
{
	const AMyCharacterBase* AvatarCharacter = Cast<AMyCharacterBase>(GetAvatarActorFromActorInfo());
	if (!AvatarCharacter) { return false; }

	const AWeaponActorBase* Weapon = AvatarCharacter->GetControlledWeapon();
	if (!Weapon) { return false; }

	const UAbilitySystemComponent* ASC = GetCurrentActorInfo()->AbilitySystemComponent.Get();
	if (!ASC || !ASC->HasMatchingGameplayTag(OnShootTag) /*(ASC->GetGameplayTagCount(ShotCountTag) > 1 && !ASC->HasMatchingGameplayTag(OnShootTag))*/) { return false; }

	return Weapon->IsAutomaticWeapon() || !ASC->HasMatchingGameplayTag(ShotCountTag);
}