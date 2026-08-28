#include "DoorDeploymentAbilityBase.h"
#include "Ability/WeaponOperation/WeaponOperationTypes.h"
#include "RewindSystemStatics.h"
#include "RewindSubsystem.h"
#include "Character/Base/MyCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "Door/DoorBase.h"
#include "WeaponBase/GrenadeActorBase.h"

void UDoorDeploymentAbilityBase::Record(const FGameplayEventData* TriggerEventData)
{
	URewindSubsystem* System = URewindSystemStatics::GetRewindSubsystem(this);
	if (!System) { return; }

	AMyCharacterBase* AvatarActor{ Cast<AMyCharacterBase>(GetRecordAvatar()) };
	if (!AvatarActor) { return; }

	TUniquePtr<FRecordedGrenadeFireAbilityData> Data = MakeUnique<FRecordedGrenadeFireAbilityData>();
	FRecordedGrenadeFireAbilityDataPayload& Payload = Data->Payload;
	FRecordedCombinableAbilityDataPayloadBase& RecordedCombinableAbilityDataPayload = Data->Payload.RecordedCombinableAbilityData.Payload;
	Payload.WeaponOwner = AvatarActor;
	Payload.GrenadeActor = Cast<AGrenadeActorBase>(Cast<AGrenadeActorBase>(AvatarActor->GetControlledWeapon()));
	Payload.LastControlWeapon = LastControlWeapon;
	Payload.RecordedCombinableAbilityData = MakeRecordedCombinableAbilityData();
	if (const URecordedGrenadeFireAbilityData * AbilityData{ Cast<URecordedGrenadeFireAbilityData>(RecordedCombinableAbilityDataPayload.EventDataToBoundAbility->OptionalObject) })
	{
		Payload.GrenadeSpawnsLocation = AbilityData->GrenadeSpawnsLocation;
		Payload.TargetLocation = AbilityData->CursorLocation;
		RecordedCombinableAbilityDataPayload.EventDataToBoundAbility->OptionalObject = nullptr;
	}
	if (BoundAbilityInfo.IsValid()) { Payload.RedoBindCallback = BoundAbilityInfo.RedoBindCallback; }

	FRecordedDataObjectHandle RecordedDataObjectHandle = System->Record(MoveTemp(Data), *AvatarActor,
		[WeakThis = MakeWeakObjectPtr(this)](const FRecordedDataObjectHandle& Handle)
		{
			if (!WeakThis.IsValid()) { return; }

			for (FCombinedAbilityIterator It{ *WeakThis }; It; ++It) { UMyGameplayAbilityBase::InvokePostRecord(*It, Handle); }
		});
	
	if (LastControlWeapon.IsValid())
	{
		AvatarActor->SwitchWeaponByActor(LastControlWeapon.Get());
		LastControlWeapon = nullptr;
	}
}

bool UDoorDeploymentAbilityBase::TryHandleRecordedData(TSharedPtr<IRecordedDataObjectInterface, ESPMode::NotThreadSafe> InRecordedData)
{
	FRecordedGrenadeFireAbilityData* Data = CustomCast<FRecordedGrenadeFireAbilityData>(InRecordedData.Get());
	if (!Data || !Data->IsPayloadValid()) { return false; }

	const FRecordedGrenadeFireAbilityDataPayload& Payload = Data->Payload;
	const FRecordedCombinableAbilityDataPayloadBase& RecordedCombinableAbilityDataPayload = Data->Payload.RecordedCombinableAbilityData.Payload;
	if (GetCombinedAbilityIDList(*this) != RecordedCombinableAbilityDataPayload.AbilityIDList) { return false; }

	bool bActivateAbilitySuccessful{ false };
	NotifyStartDurativeAction(InRecordedData);
	SetAutoReleaseRecordedDataPolicy(EReleaseRecordedDataPolicy::ReleaseManual);
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

	LastControlWeapon = Payload.LastControlWeapon;

	if (!bActivateAbilitySuccessful)
	{
		NotifyEndDurativeAction();
	}

	return bActivateAbilitySuccessful;
}

void UDoorDeploymentAbilityBase::OnPreview(const bool bIsPreview, const IRecordedDataObjectInterface* InRecordedData)
{
	const FRecordedGrenadeFireAbilityData* Data = CustomCast<FRecordedGrenadeFireAbilityData>(InRecordedData);
	if (!Data || !Data->IsPayloadValid() || Data->Payload.RecordedCombinableAbilityData.Payload.AbilityIDList != GetCombinedAbilityIDList(*this)) { return; }

	OnPreview_Internal(bIsPreview, Data->Payload.RecordedCombinableAbilityData);


	if (ADoorBase * Door{ Cast<ADoorBase>(GetAvatarActorFromActorInfo()) })
	{
		if (bIsPreview)
		{
			const FRecordedGrenadeFireAbilityDataPayload& Payload = Data->Payload;
			Door->ShowPredictionLineStatic({ Payload.GrenadeActor->GetWeaponBulletClass(), Payload.WeaponOwner.Get() }, Payload.TargetLocation);
			Door->NotifyOptionActivate();
		}
		else { Door->HidePredictionLine(); }
	}
}

void UDoorDeploymentAbilityBase::PreActivateInteractiveOption()
{
	if (!BoundAbilityInfo.Ability.IsValid()) { return; }
	
 	if (AMyCharacterBase* Character{ Cast<AMyCharacterBase>(BoundAbilityInfo.Ability->GetAvatarActorFromActorInfo()) })
	{
		LastControlWeapon = Character->GetControlledWeapon();
		Character->SwitchWeaponByClass(AGrenadeActorBase::StaticClass());
		Super::PreActivateInteractiveOption();
	}
}

void UDoorDeploymentAbilityBase::PostExecuteBindAbility()
{
	Super::PostExecuteBindAbility();

	if (LastControlWeapon.IsValid())
	{
		if (AMyCharacterBase* Character{ Cast<AMyCharacterBase>(BoundAbilityInfo.Ability->GetAvatarActorFromActorInfo()) })
		{
			Character->SwitchWeaponByClass(LastControlWeapon->GetClass());
		}

		LastControlWeapon = nullptr;
	}
}