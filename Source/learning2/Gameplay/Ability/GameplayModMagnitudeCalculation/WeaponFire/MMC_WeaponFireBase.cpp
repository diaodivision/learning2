#include "MMC_WeaponFireBase.h"
#include "BlueprintFunctionLibrary/WeaponActorBlueprintLibrary.h"

float UMMC_WeaponFireBase::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	if (!Spec.Def) { return 0.f; }

	const EWeaponSlot WeaponSlot{ static_cast<EWeaponSlot>(Spec.GetSetByCallerMagnitude(WeaponSlotTag)) };

	if (GetAssociatedWeaponSlot() != WeaponSlot) { return 0.f; }

	FGameplayAttribute MatchedMagazineAmmoAttribute, MatchedReserveAmmoAttribute;
	if (!ensure(UWeaponActorBlueprintLibrary::GetWeaponAttributeFromAttributeSetByWeaponSlot(MatchedMagazineAmmoAttribute, MatchedReserveAmmoAttribute, WeaponSlot)))
	{
		return 0.f;
	}

	const int32 FireCost{ static_cast<int32>(Spec.GetSetByCallerMagnitude(FireCostTag)) };
	if (!ensure(FireCost >= 0)) { return 0.f; }

	for (const FGameplayModifierInfo& ModifierInfo : Spec.Def->Modifiers)
	{
		const FGameplayAttribute& Attribute = ModifierInfo.Attribute;
		if (Attribute.IsValid() && (Attribute == MatchedMagazineAmmoAttribute))
		{
			return -FireCost;
		}
	}

	return 0.f;
}