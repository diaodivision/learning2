// Copyright Epic Games, Inc. All Rights Reserved.

#include "WorldPauseSubsystem.h"
//#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "Interface/FreezableInterface.h"
#include "GameFramework/GameModeBase.h"
#include "Interface/WorldPauseSubsystemProviderInterface.h"

//void UWorldPauseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
//{
	//	Super::Initialize(Collection);
//}
//
//void UWorldPauseSubsystem::Deinitialize()
//{
//	Super::Deinitialize();
//}

bool UWorldPauseSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer)) { return false; }

	const UWorld* World{ Cast<UWorld>(Outer) };
	const AWorldSettings* WorldSettings{ World && World->IsGameWorld() ? World->GetWorldSettings() : nullptr };
	const TSubclassOf<AGameModeBase> DefaultGameMode{ WorldSettings ? WorldSettings->DefaultGameMode : nullptr };
	if (const UObject* GameMode{ DefaultGameMode ? DefaultGameMode->GetDefaultObject() : nullptr }; GameMode && GameMode->Implements<UWorldPauseSubsystemProviderInterface>())
	{
		return IWorldPauseSubsystemProviderInterface::Execute_ShouldCreateWorldPauseSubsystem(GameMode);
	}
	return false;
}

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