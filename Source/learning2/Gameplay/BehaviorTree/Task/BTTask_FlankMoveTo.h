// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "BTTask_FlankMoveTo.generated.h"

/**
 *
 */
UCLASS()
class LEARNING2_API UBTTask_FlankMoveTo : public UBTTask_MoveTo
{
	GENERATED_BODY()

	virtual EBTNodeResult::Type PerformMoveTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
