// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Interface/FreezableInterface.h"
#include "Delegates/DelegateCombinations.h"
#include "Delegates/DelegateCombinations.h"
#include "MyAIController.generated.h"

class UAIPerceptionComponent;
class UBehaviorTreeComponent;
class UBehaviorTree;
class APawn;

DECLARE_MULTICAST_DELEGATE(FPostBehaviorTreeRunDelegate);

UCLASS(Blueprintable, BlueprintType)
class LEARNING2_API AMyAIController : public AAIController, public IFreezableInterface
{
	GENERATED_BODY()

public:
	AMyAIController();

	virtual void Freeze_Implementation() override;
	virtual void Unfreeze_Implementation() override;
	virtual FORCEINLINE bool IsFreezing_Implementation() override { return bIsFreezing; };

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* NewPawn) override;

	void CreateAIPerceptionComponent();
	void CreateBehaviorTreeComponent();

public:
	FPostBehaviorTreeRunDelegate PostBehaviorTreeRunDelegate;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

	bool bIsFreezing{ false };
};