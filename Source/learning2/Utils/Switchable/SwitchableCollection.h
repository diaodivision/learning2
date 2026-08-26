// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SwitchableInterface.h"
#include "Delegates/DelegateCombinations.h"
#include "Containers/ArrayView.h"
#include <type_traits>
#include <concepts>
#include "SwitchableCollection.generated.h"

/**
 *
 */
DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(bool, FPredicateFunction, const UObject*, Object);
DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(int32, FProjectionFunction, const UObject*, Object);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectAddedDelegate, UObject*, AddedObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectRemovedDelegate, UObject*, RemovedObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnControlledObjectChangedDelegate, UObject*, OldObject, UObject*, NewObject);

template<class T> requires std::is_base_of_v<UObject, T>
struct FSwitchableCollectionIterator
{
	FSwitchableCollectionIterator(USwitchableCollection& Collection);

	void operator++();
	T* operator*() const;
	operator bool() const;

	TWeakObjectPtr<USwitchableCollection> Collection;
	int32 CurrentIndex{ 0 };
};

UCLASS(Blueprintable, EditInlineNew)
class LEARNING2_API USwitchableCollection : public UObject
{
	GENERATED_BODY()

	template <class T> requires std::is_base_of_v<UObject, T> friend struct FSwitchableCollectionIterator;

public:
	UFUNCTION(BlueprintCallable, Category = "UI")
	void CreateUI();

	UFUNCTION(BlueprintCallable, Category = Container)
	UObject* SwitchObject(const UObject* Object);

	UFUNCTION(BlueprintCallable, Category = Container)
	UObject* SwitchObjectByIndex(const int32 Index);

	UFUNCTION(BlueprintCallable, Category = Container)
	[[nodiscard]] bool AddObject(UObject* Object);

	UFUNCTION(BlueprintCallable, Category = Container)
	[[nodiscard]] bool AddObjectByClass(const TSubclassOf<UObject> ObjectClass);

	UFUNCTION(BlueprintCallable, Category = Container)
	UObject* RemoveObject(const UObject* Object);

	UFUNCTION(BlueprintCallable, Category = Container)
	UObject* RemoveObjectByIndex(const int32 Index);

	UFUNCTION(BlueprintCallable, Category = Container)
	inline UObject* GetControlledObject() const { return ControlledObject.Get(); }

	UFUNCTION(BlueprintCallable, Category = Container)
	inline int32 GetControlledObjectIndex() const { return GetControlledObjectIndex_Internal(); }

	UFUNCTION(BlueprintCallable, Category = Container)
	UObject* GetObjectByIndex(const int32 Index) const;

	UFUNCTION(BlueprintCallable, Category = Container)
	UObject* GetObjectByPredicate(FPredicateFunction Predicate) const;

	UFUNCTION(BlueprintPure)
	int32 Find(const UObject* Object) const;

	FORCEINLINE bool Contains(const UObject* Object) const { return IsValidIndex(Find(Object)); }
	FORCEINLINE bool IsValidIndex(const int32 Index) const { return Collection.IsValidIndex(Index); }

	UFUNCTION(BlueprintCallable, Category = "Array")
	inline int32 Num() { return Collection.Num(); }

	UFUNCTION(BlueprintCallable, Category = "Array")
	inline void Empty() { Collection.Empty(); }

	UFUNCTION(BlueprintCallable, Category = "Array")
	inline bool IsEmpty() { return Collection.IsEmpty(); }

	UFUNCTION(BlueprintCallable, Category = "Array")
	virtual inline bool CanAddObject(const TSubclassOf<UObject> InClass) const { return CanAddObject_Internal(InClass); };

	UFUNCTION(BlueprintCallable, Category = "Collection")
	inline void EnableSort(FProjectionFunction InProjectionFunction)
	{
		bShouldCollectionSort = true;
		ProjectionFunction = InProjectionFunction;

		{
			const TFunction<int32(const UObject*)> Projection = [this](const UObject* Object) {return this->TryExecuteProjection(Object); };
			Collection.Sort([&Projection](const UObject& a, const UObject& b) {return Projection(&a) < Projection(&b); });
		}
	}
	UFUNCTION(BlueprintCallable, Category = "Collection")
	inline void DisableSort()
	{
		bShouldCollectionSort = false;
		ProjectionFunction.Unbind();
	}
	inline bool ShouldCollectionSort() const { return bShouldCollectionSort; }

	inline void SetOwner(UObject* InOwner) { Owner = InOwner; }
	inline UObject* GetOwner() const { return Owner; };

protected:
	USwitchableCollection() = default;

	virtual void AutoSwitch();

	virtual UObject* SwitchObject_Internal(const UObject* Object);
	[[nodiscard]] virtual bool AddObject_Internal(UObject* Object);
	[[nodiscard]] virtual bool AddObject_Internal(const TSubclassOf<UObject> ObjectClass);
	virtual UObject* RemoveObject_Internal(const int32 Index);
	virtual inline UObject* GetControlledObject_Internal() const { return ControlledObject.Get(); }
	virtual inline int32 GetControlledObjectIndex_Internal() const { return Find(ControlledObject.Get()); }
	virtual UObject* GetObjectByIndex_Internal(const int32 Index) const;
	virtual UObject* GetObjectByPredicate_Internal(FPredicateFunction Predicate) const;
	virtual inline bool CanAddObject_Internal(const TSubclassOf<UObject> InClass) const { return InClass->IsChildOf(RequiredClass); };

	inline int32 TryExecuteProjection(const UObject* Object) const
	{
		if (ProjectionFunction.IsBound()) { return ProjectionFunction.Execute(Object); }

		return 0;
	}

public:
	UPROPERTY(BlueprintAssignable)
	FOnObjectAddedDelegate OnObjectAddedDelegate;

	UPROPERTY(BlueprintAssignable)
	FOnObjectRemovedDelegate OnObjectRemovedDelegate;

	UPROPERTY(BlueprintAssignable)
	FOnControlledObjectChangedDelegate OnControlledObjectChangedDelegate;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Controlled")
	TWeakObjectPtr<UObject> ControlledObject;

	UPROPERTY(BlueprintReadOnly, Category = "Collection")
	TArray<TObjectPtr<UObject>> Collection;

	using ElementType = TPointedToType<decltype(Collection)::ElementType>;

public:
	template <class T = ElementType> requires std::is_base_of_v<USwitchableCollection::ElementType, T>
	FSwitchableCollectionIterator<T> CreateIterator();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (ExposeOnSpawn = true))
	bool bShowUI{ true };

	UPROPERTY()
	TObjectPtr<UUserWidget> UI;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> UIClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Class", meta = (ExposeOnSpawn = true, MustImplement = "/Script/Learning2.SwitchableInterface"))
	TSubclassOf<UObject> RequiredClass;

	UPROPERTY(BlueprintReadOnly, Category = "Collection")
	bool bShouldCollectionSort{ false };

	FProjectionFunction ProjectionFunction;

	UPROPERTY(BlueprintReadOnly, Category = "Owner", meta = (ExposeOnSpawn = true))
	TObjectPtr<UObject> Owner;
};

template<class T> requires std::is_base_of_v<UObject, T>
inline FSwitchableCollectionIterator<T>::FSwitchableCollectionIterator(USwitchableCollection& Collection) :Collection(&Collection)
{
}

template<class T> requires std::is_base_of_v<UObject, T>
void FSwitchableCollectionIterator<T>::operator++()
{
	CurrentIndex++;
}

template<class T> requires std::is_base_of_v<UObject, T>
T* FSwitchableCollectionIterator<T>::operator*() const
{
	return operator bool() ? Cast<T>(Collection->Collection[CurrentIndex]) : nullptr;
}

template<class T> requires std::is_base_of_v<UObject, T>
FSwitchableCollectionIterator<T>::operator bool() const
{
	return Collection.IsValid() && Collection->Collection.IsValidIndex(CurrentIndex) && Collection->Collection[CurrentIndex];
}

template <class T> requires std::is_base_of_v<USwitchableCollection::ElementType, T>
FSwitchableCollectionIterator<T> USwitchableCollection::CreateIterator()
{
	return FSwitchableCollectionIterator<T>{ *this };
}