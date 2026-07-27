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
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	ReleaseRecordedData();
}