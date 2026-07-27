// Fill out your copyright notice in the Description page of Project Settings.


#include "SwitchableCollection.h"
#include "Blueprint/UserWidget.h"
#include "Algo/BinarySearch.h"

void USwitchableCollection::CreateUI()
{
	if (bShowUI)
	{
		checkf(UIClass, TEXT("Need to specify UserWidget"));

		if (UI = CreateWidget<UUserWidget>(GetWorld(), UIClass))
		{
			UI->AddToViewport();
		}
	}
}

UObject* USwitchableCollection::SwitchObject(UObject* Object)
{
	return SwitchObject_Internal(Object);
}

UObject* USwitchableCollection::SwitchObjectByIndex(int32 Index)
{
	return SwitchObject(GetObjectByIndex(Index));
}

bool USwitchableCollection::AddObject(UObject* Object)
{
	return AddObject_Internal(Object);
}

bool USwitchableCollection::AddObjectByClass(TSubclassOf<UObject> ObjectClass)
{
	return AddObject_Internal(ObjectClass);
}

UObject* USwitchableCollection::RemoveObject(UObject* Object)
{
	return RemoveObjectByIndex(Find(Object));
}

UObject* USwitchableCollection::RemoveObjectByIndex(int32 Index)
{
	return RemoveObject_Internal(Index);
}

UObject* USwitchableCollection::GetObjectByIndex(int32 Index) const
{
	return GetObjectByIndex_Internal(Index);
}

UObject* USwitchableCollection::GetObjectByPredicate(FPredicateFunction Predicate) const
{
	return GetObjectByPredicate_Internal(Predicate);
}

void USwitchableCollection::AutoSwitch()
{
	if (!IsEmpty())
	{
		SwitchObjectByIndex(0);
	}
}

int32 USwitchableCollection::Find(UObject* Object) const
{
	if (!Object) { return INDEX_NONE; }

	if (ShouldCollectionSort())
	{
		const TFunction<int32(const UObject*)> Projection = [this](const UObject* Object) {return this->TryExecuteProjection(Object); };

		return Algo::BinarySearchBy(Collection, Projection(Object), [&Projection](const TObjectPtr<UObject>& Element) {return Element ? Projection(Element) : 0; });
	}

	return Collection.Find(Object);
}

bool USwitchableCollection::Contains(UObject* Object) const
{
	if (Object) { return Find(Object) != INDEX_NONE; }

	return false;
}

UObject* USwitchableCollection::SwitchObject_Internal(UObject* Object)
{
	if (UObject* OldObject{ ControlledObject.Get() }; Contains(Object) && (OldObject != Object || OldObject == nullptr))
	{
		ControlledObject = Object;
		if (OldObject) { ISwitchableInterface::Execute_OnControlReleased(OldObject); }

		ISwitchableInterface::Execute_OnControl(ControlledObject.Get(), Owner);
		OnControlledObjectChangedDelegate.Broadcast(OldObject, ControlledObject.Get());
	}

	return ControlledObject.Get();
}

bool USwitchableCollection::AddObject_Internal(UObject* Object)
{
	if (!Object || !CanAddObject_Internal(Object->GetClass())) { return false; }

	if (ShouldCollectionSort())
	{
		const TFunction<int32(const UObject*)> Projection = [this](const UObject* Object) {return this->TryExecuteProjection(Object); };

		const int32 Index = Algo::LowerBoundBy(Collection, Projection(Object), [&Projection](const TObjectPtr<UObject>& Element)
			{
				return Element ? Projection(Element) : 0;
			});
		if (Collection.IsValidIndex(Index))
		{
			if (const TObjectPtr<UObject>& Element = Collection[Index]; Element && Projection(Element)) { Collection[Index] = Object; }
			else { Collection.Insert(Object, Index); }
		}
		else { Collection.Add(Object); }
	}
	else
	{
		Collection.AddUnique(Object);
	}
	OnObjectAddedDelegate.Broadcast(Object);

	if (Collection.Num() == 1) { AutoSwitch(); }
	return true;
}

bool USwitchableCollection::AddObject_Internal(TSubclassOf<UObject> ObjectClass)
{
	if (CanAddObject_Internal(ObjectClass))
	{
		return AddObject_Internal(NewObject<UObject>(this, ObjectClass));
	}

	return false;
}

UObject* USwitchableCollection::RemoveObject_Internal(int32 Index)
{
	if (UObject* RemovedObject = GetObjectByIndex_Internal(Index))
	{
		Collection.RemoveAt(Index);

		if (RemovedObject == ControlledObject)
		{
			AutoSwitch();
		}

		OnObjectRemovedDelegate.Broadcast(RemovedObject);

		return RemovedObject;
	}

	return nullptr;
}

UObject* USwitchableCollection::GetObjectByIndex_Internal(int32 Index) const
{
	if (Collection.IsValidIndex(Index)) { return Collection[Index]; }

	return nullptr;
}

UObject* USwitchableCollection::GetObjectByPredicate_Internal(FPredicateFunction Predicate) const
{
	if (!Predicate.IsBound()) { return nullptr; }

	for (const UObject* Object : Collection)
	{
		if (Object && Predicate.Execute(Object))
		{
			return const_cast<UObject*>(Object);
		}
	}

	return nullptr;
}
