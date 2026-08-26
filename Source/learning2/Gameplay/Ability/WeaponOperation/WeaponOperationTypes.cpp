#include "WeaponOperationTypes.h"
#include "Character/Base/MyCharacterBase.h"
#include "Weapon/WeaponBase/GrenadeActorBase.h"

namespace WeaponOperationTypesPublic
{
	UE_DEFINE_GAMEPLAY_TAG(Tag_OnShoot, "Ability.Weapon.State.OnShoot");
	UE_DEFINE_GAMEPLAY_TAG(Tag_ShotCount, "Ability.Weapon.State.ShotCount");
	UE_DEFINE_GAMEPLAY_TAG(Tag_FireRate, "Ability.Weapon.Attribute.FireRate");
	UE_DEFINE_GAMEPLAY_TAG(Tag_FireCost, "Ability.Weapon.Cost.Fire");
	UE_DEFINE_GAMEPLAY_TAG(Tag_ReloadTime, "Ability.Weapon.Attribute.ReloadTime");
	UE_DEFINE_GAMEPLAY_TAG(Tag_AmountToReload, "Ability.Weapon.Cost.Reload");
	UE_DEFINE_GAMEPLAY_TAG(Tag_WeaponSlot, "Ability.Weapon.Slot");
}

bool FRecordedGrenadeFireAbilityData::IsPayloadValid() const
{
	return Payload.WeaponOwner.IsValid() && Payload.GrenadeActor.IsValid() && Payload.RecordedCombinableAbilityData.IsPayloadValid();
}

bool FRecordedGrenadeFireAbilityData::PrepareToHandleRecordedData()
{
	AMyCharacterBase* WeaponOwner{ Payload.WeaponOwner.Get() };
	AGrenadeActorBase* GrenadeActor{ Payload.GrenadeActor.Get() };
	if (!GrenadeActor || !WeaponOwner) { return false; }

	if (WeaponOwner->GetControlledWeapon() != GrenadeActor) { WeaponOwner->SwitchWeaponByActor(GrenadeActor); }
	
	if (Payload.RedoBindCallback) { Payload.RedoBindCallback(); }

	return GrenadeActor == WeaponOwner->GetControlledWeapon();
}

bool FRecordedGrenadeFireAbilityData::ConsumeAndTryHandlePayload(TSharedPtr<IRecordedDataObjectInterface, ESPMode::NotThreadSafe> RecordedData)
{
	if (!IsPayloadValid()) { return false; }

	const FRecordedCombinableAbilityDataPayloadBase& RecordedCombinableAbilityDataPayload = Payload.RecordedCombinableAbilityData.Payload;

	const FGameplayAbilitySpec* Spec = RecordedCombinableAbilityDataPayload.AbilityComponent->FindAbilitySpecFromClass(RecordedCombinableAbilityDataPayload.AbilityClass);
	if (!Spec) { return false; }

	UMyGameplayAbilityBase* Ability = Cast<UMyGameplayAbilityBase>(Spec->GetPrimaryInstance());

	if (!Ability || Ability == Ability->GetClass()->GetDefaultObject()) { return false; }

	return Ability->TryHandleRecordedData(RecordedData);
}

bool FRecordedGrenadeFireAbilityData::PrepareToPreview()
{
	return Payload.RecordedCombinableAbilityData.PrepareToPreview();
}

void FRecordedGrenadeFireAbilityData::Preview(const bool bIsPreview)
{
	Payload.RecordedCombinableAbilityData.Payload.PreviewCallback(bIsPreview, this);
}

bool FRecordedGrenadeFireAbilityData::ShouldStopRewindWhenHandleThisUnsuccessful() const
{
	return Payload.RecordedCombinableAbilityData.ShouldStopRewindWhenHandleThisUnsuccessful();
}