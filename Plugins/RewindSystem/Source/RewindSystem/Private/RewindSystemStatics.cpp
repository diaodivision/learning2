#include "RewindSystemStatics.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "RewindSubsystem.h"
#include "UObject/Object.h"
#include "InputRecordedDataTypes/RecordedDataDelegates.h"
#include "InputRecordComponent.h"

URewindSubsystem* URewindSystemStatics::GetRewindSubsystem(const UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		return World->GetSubsystem<URewindSubsystem>();
	}

	return nullptr;
}

ERecordState URewindSystemStatics::GetRewindSubsystemState(const UObject* WorldContextObject)
{
	if (const URewindSubsystem* System = GetRewindSubsystem(WorldContextObject))
	{
		return System->GetCurrentState();
	}

	return ERecordState::Idle;
}

URecordedDataPreviewOperationDelegateWrapper* URewindSystemStatics::GetPreviewOperationDelegateWrapper(bool& bSuccess, const FRecordedDataObjectHandle& RecordedDataObjectHandle)
{
	bSuccess = false;
	UInputRecordComponent* InputRecordComponent = RecordedDataObjectHandle.InputRecordComponent.Get();
	if (!InputRecordComponent) { return nullptr; }

	URecordedDataPreviewOperationDelegateWrapper* DelegateWrapper = InputRecordComponent->FindOrTryAddPreviewOperationDelegateWrapper(RecordedDataObjectHandle);
	if (DelegateWrapper) { bSuccess = true; }

	return DelegateWrapper;
}

UInputRecordComponent* URewindSystemStatics::GetInputRecordComponentFromOwner(const AActor* AvatarActor)
{
	if (!AvatarActor) { return nullptr; }

	URewindSubsystem* System = GetRewindSubsystem(AvatarActor);
	if (!System) { return nullptr; }

	return System->GetInputRecordComponentFromOwner(*AvatarActor);
}

IRecordedDataObjectInterface* URewindSystemStatics::GetRecordedDataObjectInterfaceByHandle(const FRecordedDataObjectHandle& Handle, const AActor* AvatarActor)
{
	if (!AvatarActor) { return nullptr; }

	URewindSubsystem* System = GetRewindSubsystem(AvatarActor);
	if (!System) { return nullptr; }

	UInputRecordComponent* InputRecordComponent{ System->GetInputRecordComponentFromOwner(*AvatarActor) };
	if (!InputRecordComponent) { return nullptr; }

	return InputRecordComponent->GetRecordDataByHandle(Handle);
}

int32 URewindSystemStatics::GetTickFromRecordedDataObjectHandle(const FRecordedDataObjectHandle& Handle)
{
	if (Handle.IsValid()) { return Handle.GetTick(); }

	return REWIND_TICK_NONE;
}
