#pragma once

#include "MMC_WeaponFireBase.h"
#include "MMC_Ammo3WeaponFire.generated.h"

UCLASS(BlueprintType, Blueprintable, MinimalAPI)
class UMMC_Ammo3WeaponFire : public UMMC_WeaponFireBase
{
	GENERATED_BODY()

	virtual inline EWeaponSlot GetAssociatedWeaponSlot() const override { return EWeaponSlot::Ammo3; }
};