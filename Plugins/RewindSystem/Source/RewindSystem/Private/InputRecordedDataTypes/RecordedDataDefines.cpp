#include "InputRecordedDataTypes/RecordedDataDefines.h"
#include "InputRecordComponent.h"

FRecordedDataObjectHandle::FRecordedDataObjectHandle(RewindSystemTickType Tick, UInputRecordComponent& InputRecordComponent) :
	Tick(Tick),
	InputRecordComponent(&InputRecordComponent)
{
	GenerateNewHandle();
}

void FRecordedDataObjectHandle::GenerateNewHandle()
{
	static int32 GHandle = 1;
	Handle = GHandle++;
}

IRecordedDataObjectInterface::IRecordedDataObjectInterface() : IRecordedDataObjectInterface(ERecordableActionType::Instant)
{
}

IRecordedDataObjectInterface::IRecordedDataObjectInterface(ERecordableActionType RecordableActionType) : RecordableActionType(RecordableActionType)
{
	//Handle.GenerateNewHandle();
}

FObjectToken::FObjectToken()
{
	Token = MakeShared<FObjectToken::ObjectTokenType, ESPMode::NotThreadSafe>(true);
}
TWeakPtr<FObjectToken::ObjectTokenType, ESPMode::NotThreadSafe> FObjectToken::GetToken() const
{
	return { Token };
}