#include "InputRecordedDataTypes/RecordableInterface.h"
#include "RewindSubsystem.h"

bool ADurativeRecordableBase::TryHandleRecordedData(TSharedPtr<IRecordedDataObjectInterface, ESPMode::NotThreadSafe> InRecordedData)
{
	NotifyStartDurativeAction(InRecordedData);
	return true;
}

void ADurativeRecordableBase::NotifyStartDurativeAction(TSharedPtr<IRecordedDataObjectInterface, ESPMode::NotThreadSafe> InRecordedData)
{
	constexpr bool bIsActivateActionSuccessful{ true };

	if (bIsActivateActionSuccessful)
	{
		RecordedData = InRecordedData;
	}
}

void ADurativeRecordableBase::NotifyEndDurativeAction()
{
	ReleaseRecordedData();
}

void ADurativeRecordableBase::ReleaseRecordedData()
{
	RecordedData.Reset();
}