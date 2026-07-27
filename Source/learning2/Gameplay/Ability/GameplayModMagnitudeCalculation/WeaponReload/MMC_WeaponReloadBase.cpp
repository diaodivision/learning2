#include "MMC_WeaponReloadBase.h"
#include "BlueprintFunctionLibrary/WeaponActorBlueprintLibrary.h"

float UMMC_WeaponReloadBase::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	if (!Spec.Def) { return 0.f; }

	const EWeaponSlot WeaponSlot{ static_cast<EWeaponSlot>(Spec.GetSetByCallerMagnitude(WeaponSlotTag)) };
	UE_LOG(LogTemp, Warning, TEXT("Spec.GetSetByCallerMagnitude(WeaponSlotTag): %f"), Spec.GetSetByCallerMagnitude(WeaponSlotTag));
	UE_LOG(LogTemp, Warning, TEXT("WeaponSlot: %d"), WeaponSlot);
	UE_LOG(LogTemp, Warning, TEXT("GetAssociatedWeaponSlot(): %d"), GetAssociatedWeaponSlot());

	if (GetAssociatedWeaponSlot() != WeaponSlot) { return 0.f; }

	FGameplayAttribute MatchedMagazineAmmoAttribute, MatchedReserveAmmoAttribute;
	if (!ensure(UWeaponActorBlueprintLibrary::GetWeaponAttributeFromAttributeSetByWeaponSlot(MatchedMagazineAmmoAttribute, MatchedReserveAmmoAttribute, WeaponSlot)))
	{
		return 0.f;
	}

	const int32 AmountToReload{ static_cast<int32>(Spec.GetSetByCallerMagnitude(AmountToReloadTag)) };
	UE_LOG(LogTemp, Warning, TEXT("AmountToReload: %d"), AmountToReload);
	if (!ensure(AmountToReload >= 0)) { return 0.f; }

	for (const FGameplayModifierInfo& ModifierInfo : Spec.Def->Modifiers)
	{
		const FGameplayAttribute& Attribute = ModifierInfo.Attribute;
		if (Attribute.IsValid() && (Attribute == MatchedReserveAmmoAttribute))
		{
			return -AmountToReload;
		}
	}

	return 0.f;
}