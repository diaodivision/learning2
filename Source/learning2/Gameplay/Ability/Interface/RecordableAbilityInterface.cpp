#include "RecordableAbilityInterface.h"
#include "InputRecordedDataTypes/RecordableInterface.h"

bool URecordableAbilityBase::TryHandleRecordedData(TSharedPtr<IRecordedDataObjectInterface, ESPMode::NotThreadSafe> InRecordedData)
{
	NotifyStartDurativeAction(InRecordedData);
	return true;
}

void URecordableAbilityBase::NotifyStartDurativeAction(TSharedPtr<IRecordedDataObjectInterface, ESPMode::NotThreadSafe> InRecordedData)
{
	constexpr bool bIsActivateActionSuccessful{ true };

	if (bIsActivateActionSuccessful)
	{
		RecordedData = InRecordedData;
	}
}

void URecordableAbilityBase::NotifyEndDurativeAction()
{
	ReleaseRecordedData();
}

void URecordableAbilityBase::ReleaseRecordedData()
{
	RecordedData.Reset();
}

void URecordableAbilityBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (!RecordedData.IsValid())
	{ 
		const AActor* Owner{ GetAvatarActorFromActorInfo() };
		UE_LOG(LogTemp, Warning, TEXT("NotifyStartDurativeAction never been called. Owner: %s, Ability: %s"), *GetNameSafe(Owner), *GetNameSafe(this));
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	ReleaseRecordedData();
}