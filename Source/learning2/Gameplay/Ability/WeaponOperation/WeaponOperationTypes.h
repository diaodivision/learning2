#pragma once

#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"
#include "Ability/MyGameplayAbilityType.h"
#include "InputRecordedDataTypes/InputRecordedDataTemplates.h"
#include "UObject/Object.h"
#include "WeaponOperationTypes.generated.h"

class AMyCharacterBase;
class AGrenadeActorBase;

namespace WeaponOperationTypesPublic
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_OnShoot);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_ShotCount);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_FireRate);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_FireCost);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_ReloadTime);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_AmountToReload);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_WeaponSlot);

	//inline const FGameplayTag OnShootTag = FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.State.OnShoot"));
	//inline const FGameplayTag ShotCountTag = FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.State.ShotCount"));

	//inline const FGameplayTag FireRateTag = FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.Attribute.FireRate"), false);
	//inline const FGameplayTag FireCostTag = FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.Cost.Fire"), false);
	//inline const FGameplayTag ReloadTimeTag = FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.Attribute.ReloadTime"), false);
	//inline const FGameplayTag AmountToReloadTag = FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.Cost.Reload"), false);
	//inline const FGameplayTag WeaponSlotTag = FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.Slot"), false);
}

struct FRecordedGrenadeFireAbilityDataPayload
{
	TWeakObjectPtr<AMyCharacterBase> WeaponOwner;

	TWeakObjectPtr<AGrenadeActorBase> GrenadeActor;

	FVector GrenadeSpawnsLocation;

	FVector TargetLocation;

	FRecordedCombinableAbilityData RecordedCombinableAbilityData;
};

struct FRecordedGrenadeFireAbilityData :
	public TRecordedDataBase<ERecordableActionType::HasDuration, FRecordedGrenadeFireAbilityData, FRecordedGrenadeFireAbilityDataPayload>
{
	virtual bool IsPayloadValid() const override;

	virtual bool PrepareToHandleRecordedData() override;
	virtual bool ConsumeAndTryHandlePayload(TSharedPtr<IRecordedDataObjectInterface, ESPMode::NotThreadSafe> RecordedData) override;

	virtual bool PrepareToPreview() override;
	virtual void Preview(const bool bIsPreview) override;

	virtual bool ShouldStopRewindWhenHandleThisUnsuccessful() const override;
};

UCLASS(BlueprintType)
class URecordedGrenadeFireAbilityData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	FVector GrenadeSpawnsLocation;

	UPROPERTY(BlueprintReadWrite)
	FVector CursorLocation;
};