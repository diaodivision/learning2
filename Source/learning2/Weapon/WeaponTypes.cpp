#include "WeaponTypes.h"
#include "Abilities/GameplayAbility.h"

bool FWeaponAbilityInfo::operator==(const FWeaponAbilityInfo& Other) const
{
	return AbilityClass == Other.AbilityClass;
}

FWeaponAbilityInfo::operator bool() const
{
	return AbilityClass != nullptr;
}