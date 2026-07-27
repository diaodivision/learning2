#pragma once

#include "MMC_WeaponReloadBase.h"
#include "MMC_Ammo1WeaponReload.generated.h"

UCLASS(BlueprintType, Blueprintable, MinimalAPI)
class UMMC_Ammo1WeaponReload : public UMMC_WeaponReloadBase
{
	GENERATED_BODY()

	virtual inline EWeaponSlot GetAssociatedWeaponSlot() const override { return EWeaponSlot::Ammo1; }
};