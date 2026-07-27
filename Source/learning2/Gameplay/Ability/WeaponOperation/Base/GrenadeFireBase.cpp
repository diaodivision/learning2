#include "GrenadeFireBase.h"
#include "Character/Base/MyCharacterBase.h"
#include "Weapon/WeaponBase/GrenadeActorBase.h"
#include "AbilitySystemComponent.h"
#include "InputRecordedDataTypes/RecordedDataTypes.h"
#include "RewindSystemStatics.h"
#include "RewindSubsystem.h"

void UGrenadeFireBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	const AMyCharacterBase* AvatarCharacter = Cast<AMyCharacterBase>(GetAvatarActorFromActorInfo());
	AGrenadeActorBase* GrenadeWeapon = AvatarCharacter ? Cast<AGrenadeActorBase>(AvatarCharacter->GetControlledWeapon()) : nullptr;
	if (!GrenadeWeapon)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	Instigator = AvatarCharacter;
	Weapon = GrenadeWeapon;

	if (IRecordableInterface::Execute_ShouldRecord(this))
	{
		Super::ActivateAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfoRef(), CachedTriggerEventData.GetPtrOrNull());
	}
	else
	{
		if (!CanExecuteShoot() && !(TriggerEventData && TriggerEventData->OptionalObject.IsA<URecordedGrenadeFireAbilityData>()))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}

		if (TriggerEventData) { CachedTriggerEventData = *TriggerEventData; }

		Super::ActivateAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfoRef(), CachedTriggerEventData.GetPtrOrNull());
		OnExecuteShoot();

		if (TriggerEventData)
		{
			const URecordedGrenadeFireAbilityData* GrenadeFireAbilityData{ Cast<URecordedGrenadeFireAbilityData>(TriggerEventData->OptionalObject) };
			const FSpawnGrenadeParameters SpawnGrenadeParameters{ GrenadeFireAbilityData->GrenadeSpawnsLocation ,GrenadeFireAbilityData->CursorLocation };
			for (int32 i = 0; i < GrenadeWeapon->GetBulletSpawnsOnFire(); i++)
			{
				GrenadeWeapon->SpawnBulletByParameters(SpawnGrenadeParameters);
			}
		}
		else
		{
			for (int32 i = 0; i < GrenadeWeapon->GetBulletSpawnsOnFire(); i++)
			{
				GrenadeWeapon->SpawnBullet();
			}
		}
	}
}

bool UGrenadeFireBase::ShouldRecord_Implementation() const
{
	URewindSubsystem* Subsystem = URewindSystemStatics::GetRewindSubsystem(this);
	if (!Subsystem) { return false; }

	return Subsystem->GetCurrentState() == ERecordState::Recording;
}

void UGrenadeFireBase::Record()
{
	URewindSubsystem* System = URewindSystemStatics::GetRewindSubsystem(this);
	if (!System) { return; }

	AMyCharacterBase* WeaponOwner = Cast<AMyCharacterBase>(GetAvatarActorFromActorInfo());
	if (!WeaponOwner) { return; }

	TUniquePtr<FRecordedGrenadeFireAbilityData> Data = MakeUnique<FRecordedGrenadeFireAbilityData>();
	FRecordedGrenadeFireAbilityDataPayload& Payload = Data->Payload;
	FRecordedCombinableAbilityDataPayloadBase& RecordedCombinableAbilityDataPayload = Data->Payload.RecordedCombinableAbilityData.Payload;
	Payload.WeaponOwner = WeaponOwner;
	Payload.GrenadeActor = Cast<AGrenadeActorBase>(Cast<AGrenadeActorBase>(WeaponOwner->GetControlledWeapon()));
	Payload.RecordedCombinableAbilityData = MakeRecordedCombinableAbilityData();
	if (const URecordedGrenadeFireAbilityData * AbilityData{ Cast<URecordedGrenadeFireAbilityData>(RecordedCombinableAbilityDataPayload.EventDataToBoundAbility->OptionalObject) })
	{
		Payload.GrenadeSpawnsLocation = AbilityData->GrenadeSpawnsLocation;
		Payload.TargetLocation = AbilityData->CursorLocation;
		RecordedCombinableAbilityDataPayload.EventDataToBoundAbility->OptionalObject = nullptr;
	}

	if (!ensureAlways(Data->IsPayloadValid())) { return; }

	Record_Internal(MoveTemp(Data));
}

bool UGrenadeFireBase::TryHandleRecordedData(TSharedPtr<IRecordedDataObjectInterface, ESPMode::NotThreadSafe> InRecordedData)
{
	FRecordedGrenadeFireAbilityData* Data = CustomCast<FRecordedGrenadeFireAbilityData>(InRecordedData.Get());
	if (!Data || !Data->IsPayloadValid()) { return false; }

	const FRecordedGrenadeFireAbilityDataPayload& Payload = Data->Payload;
	const FRecordedCombinableAbilityDataPayloadBase& RecordedCombinableAbilityDataPayload = Data->Payload.RecordedCombinableAbilityData.Payload;
	if (GetCombinedAbilityIDList(*this) != RecordedCombinableAbilityDataPayload.AbilityIDList) { return false; }

	bool bActivateAbilitySuccessful{ false };
	if (RecordedCombinableAbilityDataPayload.AbilityTriggerTag.IsSet())
	{
		FGameplayEventData EventData = RecordedCombinableAbilityDataPayload.EventDataToBoundAbility->Pin();
		URecordedGrenadeFireAbilityData* GrenadeFireAbilityData{ NewObject<URecordedGrenadeFireAbilityData>() };
		GrenadeFireAbilityData->GrenadeSpawnsLocation = Payload.GrenadeSpawnsLocation;
		GrenadeFireAbilityData->CursorLocation = Payload.TargetLocation;
		EventData.OptionalObject = GrenadeFireAbilityData;

		bActivateAbilitySuccessful = RecordedCombinableAbilityDataPayload.AbilityComponent->HandleGameplayEvent(RecordedCombinableAbilityDataPayload.AbilityTriggerTag.GetValue(), &EventData) > 0;
	}
	else
	{
		bActivateAbilitySuccessful = RecordedCombinableAbilityDataPayload.AbilityComponent->TryActivateAbilityByClass(RecordedCombinableAbilityDataPayload.AbilityClass);
	}

	if (bActivateAbilitySuccessful)
	{
		NotifyStartDurativeAction(InRecordedData);
	}

	return bActivateAbilitySuccessful;
}

void UGrenadeFireBase::OnPreview(const bool bIsPreview, const IRecordedDataObjectInterface* InRecordedData)
{
	const FRecordedGrenadeFireAbilityData* Data = CustomCast<FRecordedGrenadeFireAbilityData>(InRecordedData);
	if (!Data || !Data->IsPayloadValid() || Data->Payload.RecordedCombinableAbilityData.Payload.AbilityIDList != GetCombinedAbilityIDList(*this)) { return; }

	OnPreview_Internal(bIsPreview, Data->Payload.RecordedCombinableAbilityData);
}