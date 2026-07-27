// Fill out your copyright notice in the Description page of Project Settings.


#include "SwitchableActorCollection.h"

AActor* USwitchableActorCollection::SwitchActorByIndex(int32 Index)
{
	return SwitchActor(GetActorByIndex(Index));
}

AActor* USwitchableActorCollection::SwitchActor(AActor* Actor)
{
	return Cast<AActor>(SwitchObject(Actor));
}

bool USwitchableActorCollection::AddActor(AActor* Actor)
{
	return AddObject(Actor);
}

bool USwitchableActorCollection::AddActorByClass(TSubclassOf<AActor> ActorClass)
{
	return AddObjectByClass(ActorClass);
}

AActor* USwitchableActorCollection::RemoveActorByIndex(int32 Index)
{
	return Cast<AActor>(RemoveObjectByIndex(Index));
}

AActor* USwitchableActorCollection::RemoveActor(AActor* Actor)
{
	return Cast<AActor>(RemoveObject(Actor));
}

UObject* USwitchableActorCollection::SwitchObject_Internal(UObject* Object)
{
	const UObject* OldObject{ GetControlledObject() };
	const UObject* NewObject{ Super::SwitchObject_Internal(Object) };

	if (OldObject != NewObject)
	{
		OnControlledActorChangedDelegate.Broadcast(const_cast<AActor*>(Cast<AActor>(OldObject)), const_cast<AActor*>(Cast<AActor>(NewObject)));
	}

	return const_cast<UObject*>(NewObject);
}

bool USwitchableActorCollection::AddObject_Internal(UObject* Object)
{
	if (AActor* NewActor{ Cast<AActor>(Object) }; NewActor && CanAddObject_Internal(NewActor->GetClass()))
	{
		if (Super::AddObject_Internal(NewActor))
		{
			OnActorAddedDelegate.Broadcast(NewActor);
			return true;
		}
	}

	return false;
}

bool USwitchableActorCollection::AddObject_Internal(TSubclassOf<UObject> ObjectClass)
{
	if (CanAddObject_Internal(ObjectClass))
	{
		FActorSpawnParameters Params;
		Params.Owner = Cast<AActor>(Owner);
		return AddObject_Internal(GetWorld()->SpawnActor<AActor>(ObjectClass, FVector::ZeroVector, FRotator::ZeroRotator, Params));
	}

	return false;
}

UObject* USwitchableActorCollection::RemoveObject_Internal(int32 Index)
{
	AActor* RemovedActor = Cast<AActor>(Super::RemoveObject_Internal(Index));
	if (RemovedActor) { OnActorRemovedDelegate.Broadcast(RemovedActor); }

	return RemovedActor;
}


