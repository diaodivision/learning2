#pragma once

#include "MMC_WeaponReloadBase.h"
#include "MMC_Ammo2WeaponReload.generated.h"

UCLASS(BlueprintType, Blueprintable, MinimalAPI)
class UMMC_Ammo2WeaponReload : public UMMC_WeaponReloadBase
{
	GENERATED_BODY()

	virtual inline EWeaponSlot GetAssociatedWeaponSlot() const override { return EWeaponSlot::Ammo2; }
};