#include "DoorBase.h"
//#include "Kismet/GameplayStatics.h"
#include "Components/MeshComponent.h"
#include "NavAreas/NavArea_Obstacle.h"
#include "NavAreas/NavArea_Default.h"
#include "Battle/BattleSubsystemTypes.h"
#include "NavigationSystem.h"
#include "NavModifierComponent.h"
#include "NavLinkCustomComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Ability/TargetActor/GrenadeTargetActor.h"
#include "Bullet/Base/GrenadeBulletBase.h"
#include "FogOfWarComponentStatics.h"
#include "WorldHeightSubsystem.h"
#include "FogOfWarTypes.h"
#include "Components/ArrowComponent.h"
#include "FogOfWarProxyActor/FogOfWarProxyActor.h"

ADoorBase::ADoorBase()
{
	PrimaryActorTick.bCanEverTick = false;

	NavLinkCustomComponent = CreateDefaultSubobject<UNavLinkCustomComponent>(TEXT("NavLinkCustomComponent"));
	NavLinkCustomComponent->SetMoveReachedLink(this, &ADoorBase::NotifySmartLinkReached);
	NavLinkCustomComponent->SetLinkData(FVector{ -150., 0., 0. }, FVector{ 150., 0., 0. }, ENavLinkDirection::BothWays);

	FogOfWarProxyTarget1 = CreateDefaultSubobject<UArrowComponent>(TEXT("FogOfWarProxyTarget1"));
	FogOfWarProxyTarget2 = CreateDefaultSubobject<UArrowComponent>(TEXT("FogOfWarProxyTarget2"));
}

void ADoorBase::BeginPlay()
{
	Super::BeginPlay();

	//if (UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
	//{
	//	//NavSys->UnregisterActor(*this);
	//	//NavSys->UnregisterNavData();
	//	//NavSys->RegisterComponent(*NavModifierComponent);
	//	//NavSys->RegisterComponent(NavModifierComponent);
	//}

	if (UMeshComponent* DoorMesh{ GetDoorMesh() })
	{
		DoorLastRotation = DoorMesh->GetComponentRotation();

		DoorMesh->TransformUpdated.AddUObject(this, &ADoorBase::OnDoorRotated);
	}

	DeactivateModify();

	CreateGrenadeTargetActor();
	CreateFogOfWarProxyActor();
}

FOrientedBox ADoorBase::GetBounds_Implementation() const
{
	FOrientedBox OrientedBox;

	OrientedBox.Center = GetActorLocation();
	OrientedBox.AxisX = GetActorForwardVector();
	OrientedBox.AxisY = GetActorRightVector();
	OrientedBox.AxisZ = GetActorUpVector();

	FTransform Transform{ FTransform::Identity };
	Transform.SetScale3D(GetActorScale3D());
	FBox LocalBox = GetRootComponent()->CalcBounds(Transform).GetBox();

	OrientedBox.ExtentX = FMath::Abs(LocalBox.GetExtent().X);
	OrientedBox.ExtentY = FMath::Abs(LocalBox.GetExtent().Y);
	OrientedBox.ExtentZ = FMath::Abs(LocalBox.GetExtent().Z);

	return OrientedBox;
}

void ADoorBase::NotifySmartLinkReached(UNavLinkCustomComponent* LinkComp, UObject* PathingAgent, const FVector& DestPoint)
{
	if (IsDoorOpened()) { return; }

	UPathFollowingComponent* PathComp = Cast<UPathFollowingComponent>(PathingAgent);
	if (PathComp)
	{
		AActor* PathOwner = PathComp->GetOwner();
		AController* ControllerOwner = Cast<AController>(PathOwner);
		if (ControllerOwner)
		{
			PathOwner = ControllerOwner->GetPawn();
		}

		CachedDestination = PathComp->GetPathDestination();
		//PathComp->AbortMove(*this, FPathFollowingResultFlags::UserAbort);
		PathComp->PauseMove();
		UE_LOG(LogTemp, Error, TEXT("CachedDestination %s"), *CachedDestination.GetValue().ToString());
		ReceiveSmartLinkReached(PathOwner, DestPoint);
	}
}

void ADoorBase::ResumePathFollowing(AActor* Agent)
{
	if (IsDoorOpened() || !Agent || !CachedDestination.IsSet()) { return; }

	UE_LOG(LogTemp, Error, TEXT("CachedDestination 222 %s"), *CachedDestination.GetValue().ToString());

	AController* Controller{ Agent->GetInstigatorController() };
	UNavigationSystemV1* NavSys = Controller ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(Controller->GetWorld()) : nullptr;
	UPathFollowingComponent* PathComp = Controller ? Controller->FindComponentByClass<UPathFollowingComponent>() : nullptr;

	PathComp->ResumeMove();
	//if (NavSys && PathComp && PathComp->GetStatus() == EPathFollowingStatus::Idle && !PathComp->HasReached(CachedDestination.GetValue(), EPathFollowingReachMode::OverlapAgent))
	//{
	//	const FVector AgentNavLocation = Controller->GetNavAgentLocation();
	//	const ANavigationData* NavData = NavSys->GetNavDataForProps(Controller->GetNavAgentPropertiesRef(), AgentNavLocation);
	//	if (NavData)
	//	{
	//		FPathFindingQuery Query(Controller, *NavData, AgentNavLocation, CachedDestination.GetValue());
	//		FPathFindingResult Result = NavSys->FindPathSync(Query);
	//		if (Result.IsSuccessful())
	//		{
	//			PathComp->RequestMove(FAIMoveRequest(CachedDestination.GetValue()), Result.Path);
	//		}
	//	}
	//}

	CachedDestination.Reset();
}

void ADoorBase::ActivateModify()
{
	NavLinkCustomComponent->SetEnabled(false);
}

void ADoorBase::DeactivateModify()
{
	NavLinkCustomComponent->SetEnabled(true);
}

FBox ADoorBase::GetNavModifiedBounds()
{
	const FOrientedBox OrientedBox{ IWorldHeightEffectiveActorInterface::Execute_GetBounds(this) };

	const FFloatInterval ProjectToX{ OrientedBox.Project(FVector::ForwardVector) };
	const FFloatInterval ProjectToY{ OrientedBox.Project(FVector::RightVector) };
	const FFloatInterval ProjectToZ{ OrientedBox.Project(FVector::UpVector) };

	const FVector Min{ ProjectToX.Min, ProjectToY.Min, ProjectToZ.Min };
	const FVector Max{ ProjectToX.Max, ProjectToY.Max, ProjectToZ.Max };

	return { Min, Max };
}

void ADoorBase::ShowPredictionLine(const FPredictionLineParams& Params)
{
	GrenadeTargetActor->ShowPredictionLine(Params);
}

void ADoorBase::ShowPredictionLineStatic(const FPredictionLineParams& Params, const FVector& TargetLocation)
{
	GrenadeTargetActor->ShowPredictionLineStatic(Params, TargetLocation);
}

void ADoorBase::HidePredictionLine()
{
	GrenadeTargetActor->HidePredictionLine();
}

bool ADoorBase::IsPredictionLineVisible() const
{
	return GrenadeTargetActor->IsPredictionLineVisible();
}

void ADoorBase::NotifyDoorOpened()
{
	if (!IsDoorOpened()) { OnDoorOpenedDelegate.Broadcast(); }

	bIsDoorOpened = true;
}

void ADoorBase::CreateGrenadeTargetActor()
{
	FActorSpawnParameters ActorSpawnParameters;
	ActorSpawnParameters.Owner = this;
	//ActorSpawnParameters.Instigator = Cast<APawn>(GetOwner());
	if (!GrenadeTargetActorClass.Get()) { return; }
	GrenadeTargetActor = GetWorld()->SpawnActor<AGrenadeTargetActor>(GrenadeTargetActorClass, FTransform{ GetActorRotation(), GetActorLocation() }, ActorSpawnParameters);
	if (ensureAlways(GrenadeTargetActor))
	{
		GrenadeTargetActor->SetPredictionLineMesh(PredictionLineMesh);
		GrenadeTargetActor->SetIterationNum(IterationNum);
	}
	//GrenadeTargetActor->SetActorRotation(GetOwner()->GetActorRotation());
	//const FAttachmentTransformRules AttachRules{
	//EAttachmentRule::KeepWorld, EAttachmentRule::KeepRelative, EAttachmentRule::KeepWorld, false
	//};
	//GrenadeTargetActor->AttachToActor(GetOwner(), AttachRules);
}

void ADoorBase::CreateFogOfWarProxyActor()
{
	FActorSpawnParameters ActorSpawnParameters;
	ActorSpawnParameters.Owner = this;
	FogOfWarProxyActor = GetWorld()->SpawnActor<AFogOfWarProxyActor>(AFogOfWarProxyActor::StaticClass(), FTransform{ GetActorRotation(), GetActorLocation() }, ActorSpawnParameters);
	if (ensureAlways(FogOfWarProxyActor)) 
	{ 
		FogOfWarProxyActor->SetActorHiddenInGame(true);
	}
}

void ADoorBase::OnDoorRotated(USceneComponent* SceneComponent, EUpdateTransformFlags Flags, ETeleportType TeleportType)
{
	if (SceneComponent->GetComponentRotation().Equals(DoorLastRotation)) { return; }

	if (UWorldHeightSubsystem* WorldHeightSubsystem{ UFogOfWarComponentStatics::GetWorldHeightSubsystem(this) })
	{
		WorldHeightSubsystem->RequestUpdateWorldHeightData(*this, FWorldHeightBoundsUpdateRequest::Type::Removed);
		// WorldHeightSubsystem->RequestUpdateWorldHeightData(*this, FWorldHeightBoundsUpdateRequest::Type::Added);
	}
}

void ADoorBase::ShowFogOfWarProxy(const AActor* InInstigator)
{
	if (!InInstigator || !FogOfWarProxyActor || !FogOfWarProxyTarget1 || !FogOfWarProxyTarget2) { return; }

	const FVector InstigatorLocation{ InInstigator->GetActorLocation() };
	const FVector ProxyTargetLocation1{ FogOfWarProxyTarget1->GetComponentLocation() };
	const FVector ProxyTargetLocation2{ FogOfWarProxyTarget2->GetComponentLocation() };
	if (FVector::DistSquaredXY(InstigatorLocation, ProxyTargetLocation1) > FVector::DistSquaredXY(InstigatorLocation, ProxyTargetLocation2))
	{
		FogOfWarProxyActor->SetActorLocation(ProxyTargetLocation1);
		FogOfWarProxyActor->SetActorRotation(FogOfWarProxyTarget1->GetComponentRotation());
	}
	else
	{
		FogOfWarProxyActor->SetActorLocation(ProxyTargetLocation2);
		FogOfWarProxyActor->SetActorRotation(FogOfWarProxyTarget2->GetComponentRotation());
	}
	FogOfWarProxyActor->SetActorHiddenInGame(false);
}

void ADoorBase::HideFogOfWarProxy()
{
	FogOfWarProxyActor->SetActorHiddenInGame(true);
}