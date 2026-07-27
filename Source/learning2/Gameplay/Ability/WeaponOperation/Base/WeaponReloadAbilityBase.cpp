#include "WeaponReloadAbilityBase.h"
#include "Character/Base/MyCharacterBase.h"
#include "WeaponBase/WeaponActorBase.h"
#include "AbilitySystemComponent.h"

UWeaponReloadAbilityBase::UWeaponReloadAbilityBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UWeaponReloadAbilityBase::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	const AMyCharacterBase* AvatarCharacter = ActorInfo ? Cast<AMyCharacterBase>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (!AvatarCharacter) { return false; }

	if (const AWeaponActorBase* Weapon = AvatarCharacter->GetControlledWeapon(); Weapon && CanExecuteReload(Weapon))
	{
		return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
	}

	return false;
}

void UWeaponReloadAbilityBase::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	const UGameplayEffect* CooldownEffectCDO = GetCooldownGameplayEffect();
	if (!ensure(CooldownEffectCDO)) { return; }

	const AMyCharacterBase* AvatarCharacter = ActorInfo ? Cast<AMyCharacterBase>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (!AvatarCharacter) { return; }

	const AWeaponActorBase* Weapon = AvatarCharacter->GetControlledWeapon();
	if (!Weapon) { return; }

	FGameplayEffectSpecHandle CooldownSpecHandle = MakeOutgoingGameplayEffectSpec(CooldownEffectCDO->GetClass(), GetAbilityLevel(Handle, ActorInfo));
	if (!CooldownSpecHandle.IsValid()) { return; }

	CooldownSpecHandle.Data.Get()->SetSetByCallerMagnitude(ReloadTimeTag, Weapon->GetReloadTime());

	if (UAbilitySystemComponent* AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get())
	{
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*CooldownSpecHandle.Data.Get());
	}
}

bool UWeaponReloadAbilityBase::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	const AMyCharacterBase* AvatarCharacter = ActorInfo ? Cast<AMyCharacterBase>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (!AvatarCharacter) { return false; }

	const AWeaponActorBase* Weapon = AvatarCharacter->GetControlledWeapon();
	if (!Weapon) { return false; }

	return CanExecuteReload(Weapon);
}

void UWeaponReloadAbilityBase::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	const UGameplayEffect* CostEffectCDO = GetCostGameplayEffect();
	if (!ensure(CostEffectCDO)) { return; }

	const AMyCharacterBase* AvatarCharacter = ActorInfo ? Cast<AMyCharacterBase>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (!AvatarCharacter) { return; }

	const AWeaponActorBase* Weapon = AvatarCharacter->GetControlledWeapon();
	if (!Weapon) { return; }

	FGameplayEffectSpecHandle CooldownSpecHandle = MakeOutgoingGameplayEffectSpec(CostEffectCDO->GetClass(), GetAbilityLevel(Handle, ActorInfo));
	if (!CooldownSpecHandle.IsValid()) { return; }

	const int32 ReloadAmount{ Weapon->CalculateReloadAmount() };
	ensure(ReloadAmount > 0);

	CooldownSpecHandle.Data.Get()->SetSetByCallerMagnitude(AmountToReloadTag, ReloadAmount);
	CooldownSpecHandle.Data.Get()->SetSetByCallerMagnitude(WeaponSlotTag, static_cast<float>(Weapon->GetWeaponSlot()));

	if (UAbilitySystemComponent* AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get())
	{
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*CooldownSpecHandle.Data.Get());
	}
}

bool UWeaponReloadAbilityBase::CanExecuteReload(const AWeaponActorBase* Weapon) const
{
	return Weapon && Weapon->GetReserveAmmo() > 0 && (Weapon->GetMagazineAmmoMax() + Weapon->GetChamberCapacity()) > Weapon->GetMagazineAmmo();
}