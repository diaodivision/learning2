#pragma once

#include "CoreMinimal.h"
#include "WeaponTypes.h"
#include "GameplayModMagnitudeCalculation.h"
#include "Ability/WeaponOperation/WeaponOperationTypes.h"
#include "MMC_WeaponFireBase.generated.h"

UCLASS(BlueprintType, Blueprintable, Abstract, MinimalAPI)
class UMMC_WeaponFireBase : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

	virtual EWeaponSlot GetAssociatedWeaponSlot() const { return EWeaponSlot::None; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Tag")
	FGameplayTag FireCostTag = WeaponOperationTypesPublic::Tag_FireCost.GetTag();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Tag")
	FGameplayTag WeaponSlotTag = WeaponOperationTypesPublic::Tag_WeaponSlot.GetTag();
};