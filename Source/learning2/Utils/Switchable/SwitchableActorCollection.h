// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SwitchableCollection.h"
#include "GameFramework/Actor.h"
#include "SwitchableActorCollection.generated.h"

/**
 *
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActorAddedDelegate, AActor*, AddedActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActorRemovedDelegate, AActor*, RemovedActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnControlledActorChangedDelegate, AActor*, OldActor, AActor*, NewActor);

UCLASS(Blueprintable, EditInlineNew, meta = (HideFunctions = "SwitchObject SwitchObjectByIndex AddObject AddObjectByClass RemoveObject RemoveObjectByIndex GetControlledObject GetControlledObjectIndex GetObjectByIndex CanAddObject"))
class LEARNING2_API USwitchableActorCollection : public USwitchableCollection
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = Container)
	AActor* SwitchActor(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = Container)
	AActor* SwitchActorByIndex(int32 Index);

	UFUNCTION(BlueprintCallable, Category = Container)
	[[nodiscard]] bool AddActor(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = Container)
	[[nodiscard]] bool AddActorByClass(TSubclassOf<AActor> ActorClass);

	UFUNCTION(BlueprintCallable, Category = Container)
	AActor* RemoveActorByIndex(int32 Index);

	UFUNCTION(BlueprintCallable, Category = Container)
	AActor* RemoveActor(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = Container)
	inline AActor* GetControlledActor() const { return Cast<AActor>(GetControlledObject()); }

	UFUNCTION(BlueprintCallable, Category = Container)
	int32 GetControlledActorIndex() const { return GetControlledObjectIndex(); };

	UFUNCTION(BlueprintCallable, Category = Container)
	inline AActor* GetActorByIndex(int32 Index) const { return Cast<AActor>(GetObjectByIndex(Index)); };

	UFUNCTION(BlueprintCallable, Category = Container)
	inline AActor* GetActorByPredicate(FPredicateFunction Predicate) const { return Cast<AActor>(GetObjectByPredicate(Predicate)); }

	UFUNCTION(BlueprintCallable, Category = "Array")
	virtual inline bool CanAddActor(TSubclassOf<AActor> InClass) const { return CanAddObject(InClass); };

protected:
	virtual UObject* SwitchObject_Internal(UObject* Object) override;
	[[nodiscard]] virtual bool AddObject_Internal(UObject* Object) override;
	[[nodiscard]] virtual bool AddObject_Internal(TSubclassOf<UObject> ObjectClass) override;
	virtual UObject* RemoveObject_Internal(int32 Index) override;

public:
	UPROPERTY(BlueprintAssignable)
	FOnActorAddedDelegate OnActorAddedDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnActorRemovedDelegate OnActorRemovedDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnControlledActorChangedDelegate OnControlledActorChangedDelegate;
};