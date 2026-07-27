#pragma once

#include "UObject/Object.h"
#include "Delegates/DelegateCombinations.h"
#include "InputRecordedDataTypes/RecordedDataDefines.h"
#include "RecordedDataDelegates.generated.h"

DECLARE_MULTICAST_DELEGATE_ThreeParams(FPreviewOperationDelegate, const FRecordedDataObjectHandle& Handle, const IRecordedDataObjectInterface* RecordedData, bool bWillExecute);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPreviewOperationDelegateDynamic, const FRecordedDataObjectHandle&, Handle, bool, bWillExecute);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnOperationPreviewDelegate, const bool bIsPreview, const IRecordedDataObjectInterface* Data);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOperationRecordedDelegate, const FRecordedDataObjectHandle&, Handle);

UCLASS(BlueprintType, MinimalAPI)
class URecordedDataPreviewOperationDelegateWrapper : public UObject
{
	GENERATED_BODY()

public:
	inline void Broadcast(const FRecordedDataObjectHandle& Handle, const IRecordedDataObjectInterface& RecordedData, const bool bWillExecute)
	{
		if (ensure(PreviewOperationDelegate.IsBound() || PreviewOperationDelegateDynamic.IsBound()))
		{
			PreviewOperationDelegate.Broadcast(Handle, &RecordedData, bWillExecute);
			PreviewOperationDelegateDynamic.Broadcast(Handle, bWillExecute);
		}
	}

	FPreviewOperationDelegate PreviewOperationDelegate;

	UPROPERTY(BlueprintAssignable, Category = "RewindSystem|Callbacks")
	FPreviewOperationDelegateDynamic PreviewOperationDelegateDynamic;
};