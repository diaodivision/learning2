#pragma once

#include "MMC_WeaponReloadBase.h"
#include "MMC_Ammo4WeaponReload.generated.h"

UCLASS(BlueprintType, Blueprintable, MinimalAPI)
class UMMC_Ammo4WeaponReload : public UMMC_WeaponReloadBase
{
	GENERATED_BODY()

	virtual inline EWeaponSlot GetAssociatedWeaponSlot() const override { return EWeaponSlot::Ammo4; }
};