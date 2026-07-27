// Copyright Epic Games, Inc. All Rights Reserved.

#include "WorldPauseSubsystem.h"
//#include "Kismet/GameplayStatics.h"
#include "Interface/FreezableInterface.h"

//void UWorldPauseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
//{
//	Super::Initialize(Collection);
//}
//
//void UWorldPauseSubsystem::Deinitialize()
//{
//	Super::Deinitialize();
//}

void UWorldPauseSubsystem::OnFreezableObjectRegistered(UObject* FreezableObject)
{
	if (!FreezableObject) { return; }

	FreezableObjects.Add(FreezableObject);

	if (bIsFreezeAllObject) { FreeObject(FreezableObject); }
}

void UWorldPauseSubsystem::OnFreezableObjectUnregistered(UObject* FreezableObject)
{
	if (!FreezableObject) { return; }

	FreezableObjects.Remove(FreezableObject);
}

void UWorldPauseSubsystem::FreeObject(UObject* FreezableObject)
{
	if (!FreezableObject) { return; }

	if (!ensureAlwaysMsgf(FreezableObjects.Contains(FreezableObject), TEXT("Object: {%s} were not under managed by UWorldPauseSubsystem, nothing will be happend"), *GetNameSafe(FreezableObject)))
	{
		return;
	}

	if (!ensure(FreezableObject->Implements<UFreezableInterface>())) { return; }

	IFreezableInterface::Execute_Freeze(FreezableObject);
}

void UWorldPauseSubsystem::UnfreeObject(UObject* FreezableObject)
{
	if (!FreezableObject) { return; }

	if (!ensureAlwaysMsgf(FreezableObjects.Contains(FreezableObject), TEXT("Object: {%s} were not under managed by UWorldPauseSubsystem, nothing will be happend"), *GetNameSafe(FreezableObject)))
	{
		return;
	}

	if (!ensure(FreezableObject->Implements<UFreezableInterface>())) { return; }

	IFreezableInterface::Execute_Unfreeze(FreezableObject);
}

void UWorldPauseSubsystem::FreezeAllObject()
{
	bIsFreezeAllObject = true;

	bool bHasInvalidObject{ false };

	for (TWeakObjectPtr<UObject>& Object : FreezableObjects)
	{
		if (!Object.IsValid())
		{
			bHasInvalidObject = true;
			continue;
		}

		FreeObject(Object.Get());
	}

	if (bHasInvalidObject) { ClearInvalidObject(); }
}

void UWorldPauseSubsystem::UnfreezeAllObject()
{
	bIsFreezeAllObject = false;

	bool bHasInvalidObject{ false };

	for (TWeakObjectPtr<UObject>& Object : FreezableObjects)
	{
		if (!Object.IsValid())
		{
			bHasInvalidObject = true;
			continue;
		}

		UnfreeObject(Object.Get());
	}

	if (bHasInvalidObject) { ClearInvalidObject(); }
}

void UWorldPauseSubsystem::ClearInvalidObject()
{
	for (auto It{ FreezableObjects.CreateIterator() }; It; ++It)
	{
		if (!It->IsValid()) { It.RemoveCurrent(); }
	}
}