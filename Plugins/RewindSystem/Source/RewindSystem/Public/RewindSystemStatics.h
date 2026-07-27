// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputRecordedDataTypes/RecordedDataDefines.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include <type_traits>
#include "RewindSystemStatics.generated.h"

class URewindSubsystem;
class UObject;
class URecordedDataPreviewOperationDelegateWrapper;

/**
 *
 */
UCLASS()
class REWINDSYSTEM_API URewindSystemStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static URewindSubsystem* GetRewindSubsystem(const UObject* WorldContextObject);

	static ERecordState GetRewindSubsystemState(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "RewindSystem|Delegate")
	[[nodiscard]] static URecordedDataPreviewOperationDelegateWrapper* GetPreviewOperationDelegateWrapper(bool& bSuccess, const FRecordedDataObjectHandle& RecordedDataObjectHandle);

	static UInputRecordComponent* GetInputRecordComponentFromOwner(const AActor* AvatarActor);

	static IRecordedDataObjectInterface* GetRecordedDataObjectInterfaceByHandle(const FRecordedDataObjectHandle& Handle, const AActor* AvatarActor);

private:
	static_assert(sizeof(int32) >= sizeof(RewindSystemTickType), "If using of RewindSystemTickType change, consider updating all function here which us int16 replaced RewindSystemTickType.");

	UFUNCTION(BlueprintPure, Category = "RewindSystem|Data")
	static int32 GetTickFromRecordedDataObjectHandle(const FRecordedDataObjectHandle& Handle);
};
