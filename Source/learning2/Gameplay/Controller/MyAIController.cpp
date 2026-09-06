#include "MyAIController.h"
// #include "WorldPauseSubsystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"

AMyAIController::AMyAIController()
{
	CreateAIPerceptionComponent();
	CreateBehaviorTreeComponent();
}

void AMyAIController::Freeze_Implementation()
{
	BehaviorTreeComponent->StopLogic(TEXT("Freeze"));
	AIPerceptionComponent->Deactivate();
	bIsFreezing = true;
}

void AMyAIController::Unfreeze_Implementation()
{
	BehaviorTreeComponent->StartLogic();
	AIPerceptionComponent->Activate();
	bIsFreezing = false;
}

void AMyAIController::BeginPlay()
{
	Super::BeginPlay();

	RunBehaviorTree(BehaviorTreeAsset);
	PostBehaviorTreeRunDelegate.Broadcast();
}

void AMyAIController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);

	if (bStartAILogicOnPossess && IFreezableInterface::Execute_IsFreezing(this)) { BehaviorTreeComponent->StopLogic(TEXT("Freeze")); }
}

void AMyAIController::CreateAIPerceptionComponent()
{
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(FName("AIPerceptionComponent"));
}

void AMyAIController::CreateBehaviorTreeComponent()
{
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(FName("BehaviorTreeComponent"));
}