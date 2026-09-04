// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_FlankMoveTo.h"
#include "Battle/BattleSubsystem.h"
#include "Battle/BattleSubsystemStatics.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "AIController.h"
#include "Tasks/AITask_MoveTo.h"

EBTNodeResult::Type UBTTask_FlankMoveTo::PerformMoveTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type Result = EBTNodeResult::Failed;

	if (UBattleSubsystem* BattleSubsystem{ UBattleSubsystemStatics::GetBattleSubsystem(this) }; !BattleSubsystem)
	{
		Result = Super::PerformMoveTask(OwnerComp, NodeMemory);
	}
	else
	{
		const UBlackboardComponent* MyBlackboard = OwnerComp.GetBlackboardComponent();
		const AAIController* MyController = OwnerComp.GetAIOwner();

		FVector TargetLocation;
		if (MyController && MyBlackboard)
		{

			if (BlackboardKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
			{
				if (const AActor* TargetActor = Cast<AActor>(MyBlackboard->GetValue<UBlackboardKeyType_Object>(BlackboardKey.GetSelectedKeyID())))
				{
					TargetLocation = TargetActor->GetActorLocation();
				}
			}
			else if (BlackboardKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
			{
				TargetLocation = MyBlackboard->GetValue<UBlackboardKeyType_Vector>(BlackboardKey.GetSelectedKeyID());
			}

			FNavigationModifyHandle Handle{ BattleSubsystem->ActivateNavModify(MyController->GetNavAgentLocation(), TargetLocation) };

			Result = Super::PerformMoveTask(OwnerComp, NodeMemory);

			BattleSubsystem->DeactivateNavModify(Handle);
		}
	};

	return Result;
}