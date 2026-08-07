#include "InteractionOption.h"
#include "Abilities/GameplayAbility.h"

bool FOptionInfo::operator==(const FOptionInfo& Other) const
{
	return AbilityClass == Other.AbilityClass;
}

FOptionInfo::operator bool() const
{
	return AbilityClass != nullptr;
}