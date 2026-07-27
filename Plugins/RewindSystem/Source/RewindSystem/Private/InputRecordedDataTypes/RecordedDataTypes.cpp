#include "InputRecordedDataTypes/RecordedDataTypes.h"

//bool FRecordedLocationData::TryHandlePayload(TSharedPtr<IRecordedDataObjectInterface, ESPMode::NotThreadSafe> SharedThis)
//{
//	return false;
//}

//void FRecordedLocationData::PreviewOnHandlePayload(bool bIsPreview)
//{
//	if (!IsPayloadValid()) { return; }
//
//	if (bIsPreview)
//	{
//		Payload.Actor->SetActorLocation(Payload.NewLocation);
//	}
//	else
//	{
//		Payload.Actor->SetActorLocation(Payload.OldLocation);
//	}
//}

bool FRecordedLocationData::PrepareToHandleRecordedData()
{
	return true;
}

bool FRecordedLocationData::ConsumeAndTryHandlePayload(TSharedPtr<IRecordedDataObjectInterface, ESPMode::NotThreadSafe> RecordedData)
{
	if (!IsPayloadValid()) { return false; }

	if (!Payload.Actor.IsValid()) { return false; }

	constexpr bool bSweep{ true };
	return Payload.Actor->SetActorLocation(Payload.NewLocation, bSweep);
}

bool FRecordedLocationData::PrepareToPreview()
{
	return true;
}

void FRecordedLocationData::Preview(const bool bIsPreview)
{
	if (!IsPayloadValid()) { return; }

	if (bIsPreview) { Payload.Actor->SetActorLocation(Payload.NewLocation); }
	else { Payload.Actor->SetActorLocation(Payload.OldLocation); }
}

bool FRecordedLocationData::ShouldStopRewindWhenHandleThisUnsuccessful() const
{
	return false;
}

//bool FRecordedRotationData::TryHandlePayload(TSharedPtr<IRecordedDataObjectInterface, ESPMode::NotThreadSafe> SharedThis)
//{
//	return false;
//}

//void FRecordedRotationData::PreviewOnHandlePayload(bool bIsPreview)
//{
//	if (!IsPayloadValid()) { return; }
//
//	if (bIsPreview)
//	{
//		Payload.Actor->SetActorRotation(Payload.NewRotation);
//	}
//	else
//	{
//		Payload.Actor->SetActorRotation(Payload.OldRotation);
//	}
//}

bool FRecordedRotationData::PrepareToHandleRecordedData()
{
	return true;
}

bool FRecordedRotationData::ConsumeAndTryHandlePayload(TSharedPtr<IRecordedDataObjectInterface, ESPMode::NotThreadSafe> RecordedData)
{
	if (!IsPayloadValid()) { return false; }

	if (!Payload.Actor.IsValid()) { return false; }

	return Payload.Actor->SetActorRotation(Payload.NewRotation);
}

bool FRecordedRotationData::PrepareToPreview()
{
	return true;
}

void FRecordedRotationData::Preview(const bool bIsPreview)
{
	if (!IsPayloadValid()) { return; }

	if (bIsPreview) { Payload.Actor->SetActorRotation(Payload.NewRotation); }
	else { Payload.Actor->SetActorRotation(Payload.OldRotation); }
}

bool FRecordedRotationData::ShouldStopRewindWhenHandleThisUnsuccessful() const
{
	return false;
}
