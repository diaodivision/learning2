#include "BattleSubsystem.h"
#include "BattleFieldVolume.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "GenericTeamAgentInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Interface/NavmodifiedActorInterface.h"
#include "Engine/OverlapResult.h"
#include "Character/Base/MyCharacterBase.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AIPerceptionSystem.h"
#include <limits>
#include "Interface/BattleSubsystemProviderInterface.h"
#include "GameFramework/GameModeBase.h"

bool UBattleSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer)) { return false; }

	const UWorld* World{ Cast<UWorld>(Outer) };
	const AWorldSettings* WorldSettings{ World && World->IsGameWorld() ? World->GetWorldSettings() : nullptr };
	const TSubclassOf<AGameModeBase> DefaultGameMode{ WorldSettings ? WorldSettings->DefaultGameMode : nullptr };
	if (const UObject* GameMode{ DefaultGameMode ? DefaultGameMode->GetDefaultObject() : nullptr }; GameMode && GameMode->Implements<UBattleSubsystemProviderInterface>())
	{
		return IBattleSubsystemProviderInterface::Execute_ShouldCreateBattleSubsystem(GameMode);
	}
	return false;
}

void UBattleSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InitializeDelegates();

	InitializeBattleFieldVolumes();
	InitializeNavModifiedActors();
	InitializeCharacters();

}

void UBattleSubsystem::Deinitialize()
{
	DeinitializeDelegates();

	Super::Deinitialize();
}

FNavigationModifyHandle UBattleSubsystem::ActivateNavModify(const FVector& StartLocation, const FVector& EndLocation)
{
	FNavigationModifyHandle Handle;

	TArray<TWeakObjectPtr<AActor>> NavModifiedActors = CollectModifiedActors(StartLocation, EndLocation);
	for (const TWeakObjectPtr<AActor>& Actor : NavModifiedActors)
	{
		if (INavModifiedActorInterface * NavModifiedActorInterface{ Cast<INavModifiedActorInterface>(Actor) })
		{
			NavModifiedActorInterface->ActivateModify();

			TArray<FNavigationModifyHandle>& Handles{ ActivatedModifyActorsMap.FindOrAdd(Actor) };

			Handles.Add(Handle);
		}
	}

	return Handle;
}

void UBattleSubsystem::DeactivateNavModify(const FNavigationModifyHandle& Handle)
{
	if (!ensure(Handle.IsValid())) { return; }

	for (auto It{ ActivatedModifyActorsMap.CreateIterator() }; It; ++It)
	{
		if (!It->Key.IsValid()) { continue; }

		It->Value.RemoveAllSwap([Handle](const FNavigationModifyHandle& Element) {return Element == Handle; });

		if (It->Value.IsEmpty())
		{
			if (INavModifiedActorInterface * NavModifiedActorInterface{ Cast<INavModifiedActorInterface>(It->Key.Get()) })
			{
				NavModifiedActorInterface->DeactivateModify();
			}

			It.RemoveCurrent();
		}
	}
}

void UBattleSubsystem::OnBattleFieldVolumeBeginPlay(const ABattleFieldVolume* Volume)
{
	if (!Volume) { return; }

	//TArray<AActor> NavModifiedActors;
	TArray<TWeakObjectPtr<AActor>>& NavModifiedActors = BattleFieldVolumes.Add(Volume);

	{
		TArray<FOverlapResult> OverlapResults;
		GetWorld()->OverlapMultiByObjectType(
			OverlapResults,
			Volume->GetActorLocation(),
			Volume->GetActorRotation().Quaternion(),
			FCollisionObjectQueryParams::AllObjects,
			FCollisionShape::MakeBox(Volume->GetBounds().BoxExtent));

		for (const FOverlapResult& Result : OverlapResults)
		{
			if (AActor* Actor = Result.GetActor(); Actor && Actor->Implements<UNavModifiedActorInterface>())
			{
				NavModifiedActors.Add(Actor);
			}
		}
	}
}

void UBattleSubsystem::OnBattleFieldVolumeEndPlay(const ABattleFieldVolume* Volume, const EEndPlayReason::Type EndPlayReason)
{
	if (!Volume) { return; }

	BattleFieldVolumes.Remove(Volume);
}

void UBattleSubsystem::OnNavModifiedActorBeginPlay(AActor* NavModifiedActor)
{
	INavModifiedActorInterface* NavModifiedInterface{ Cast<INavModifiedActorInterface>(NavModifiedActor) };
	if (!NavModifiedActor) { return; }

	ECollisionChannel BattleFieldVolumeObjectType;
	{
		const ABattleFieldVolume* BattleFieldVolumeCDO{ Cast<ABattleFieldVolume>(ABattleFieldVolume::StaticClass()->GetDefaultObject()) };
		const USceneComponent* BattleFieldVolumeRootComponent{ BattleFieldVolumeCDO ? BattleFieldVolumeCDO->GetRootComponent() : nullptr };
		check(BattleFieldVolumeRootComponent);

		BattleFieldVolumeObjectType = BattleFieldVolumeRootComponent->GetCollisionObjectType();
	}

	{
		TArray<FOverlapResult> OverlapResults;
		GetWorld()->OverlapMultiByChannel(
			OverlapResults,
			NavModifiedActor->GetActorLocation(),
			NavModifiedActor->GetActorRotation().Quaternion(),
			BattleFieldVolumeObjectType,
			FCollisionShape::MakeBox(NavModifiedInterface->GetNavModifiedBounds().GetExtent()));

		for (const FOverlapResult& Result : OverlapResults)
		{
			if (TArray<TWeakObjectPtr<AActor>>* NavModifiedActors = BattleFieldVolumes.Find(Cast<ABattleFieldVolume>(Result.GetActor())))
			{
				NavModifiedActors->Add(NavModifiedActor);
				NavModifiedActor->OnEndPlay.AddDynamic(this, &UBattleSubsystem::OnNavModifiedActorEndPlay);
			}
		}
	}
}

void UBattleSubsystem::OnNavModifiedActorEndPlay(AActor* NavModifiedActor, const EEndPlayReason::Type EndPlayReason)
{
	for (auto It{ BattleFieldVolumes.CreateIterator() }; It; ++It)
	{
		if (It->Key.IsValid())
		{
			It->Value.RemoveAllSwap([NavModifiedActor](const TWeakObjectPtr<AActor>& Actor) {return Actor == NavModifiedActor; });
		}
		else
		{
			//for (auto ValueIt{ It->Value.CreateIterator() }; ValueIt; ++ValueIt)
			//{
			//	
			//}
			It.RemoveCurrent();
		}
	}
}

void UBattleSubsystem::RegisterToBattleSubsystem(AMyCharacterBase* Character)
{
	if (!Character) { return; }

	const BattleSubsystemTypes::TeamIDType TeamID{ Character->GetGenericTeamId().GetId() };
	TeamMembersMap.FindOrAdd(TeamID).Add(Character);

	if (!Character->OnSenseUpdatedDelegate.IsBoundToObject(this)) { Character->OnSenseUpdatedDelegate.AddUObject(this, &UBattleSubsystem::OnSenseUpdated); }
	if (!Character->OnEndPlay.IsAlreadyBound(this, &UBattleSubsystem::OnTeamMemberEndPlay)) { Character->OnEndPlay.AddUniqueDynamic(this, &UBattleSubsystem::OnTeamMemberEndPlay); }

	UpdateAllSensesActor(true, *Character);
}

void UBattleSubsystem::UnregisterToBattleSubsystem(AMyCharacterBase* Character)
{
	if (!Character) { return; }

	// if (FTeamSensesContainer* Container = TeamSensesMap.Find(TeamID)) 
	// { 
	// 	Container->OnActorEndPlayed(*Character);
	// }
	for (auto It{ TeamSensesMap.CreateIterator() }; It; ++It)
	{
		It.Value().OnActorEndPlayed(*Character);
	}
		
	const BattleSubsystemTypes::TeamIDType TeamID{ Character->GetGenericTeamId().GetId() };
	if (TSet<TWeakObjectPtr<AMyCharacterBase>>*Members{ TeamMembersMap.Find(TeamID) })
	{
		Members->Remove(Character);
		if (Members->IsEmpty()) { TeamMembersMap.Remove(TeamID); }
	}

	Character->OnSenseUpdatedDelegate.RemoveAll(this);
	Character->OnEndPlay.RemoveAll(this);

	// UpdateAllSensesActor(false, *Character);
	// OnNoLongerSensedByAnyTeamMember(Character->GetGenericTeamId(), *Character);
}

bool UBattleSubsystem::K2_IsSensedByTeam(int32 TeamID, const AMyCharacterBase* Enemy) const
{
	check(TeamID <= std::numeric_limits<BattleSubsystemTypes::TeamIDType>::max());

	if (Enemy) { return IsSensedByTeam(TeamID, *Enemy); }

	return false;
}

bool UBattleSubsystem::IsSensedByTeam(BattleSubsystemTypes::TeamIDType TeamID, const AMyCharacterBase& Enemy) const
{
	//if (const TSet<TWeakObjectPtr<AMyCharacterBase>>*TeamMembers{ TeamMap.Find(TeamID) })
	//{
	//	for (const TWeakObjectPtr<AMyCharacterBase>& Member : *TeamMembers)
	//	{
	//		if (Member.IsValid() && SensesContainer.IsObserverSenseToEnemy(*Member.Get(), Enemy)) { return true; }
	//	}
	//}

	//return false;
	return false;
}

AActor* UBattleSubsystem::GetOneTeamSensedActor(const BattleSubsystemTypes::TeamIDType TeamID) const
{
	//TMap<BattleSubsystemTypes::TeamIDType, FTeamSensesContainer> TeamSensesMap;
	if (const FTeamSensesContainer * Container{ TeamSensesMap.Find(TeamID) }) { return Container->GetOneTeamSensedActor(); }

	return nullptr;
}

void UBattleSubsystem::InitializeDelegates()
{
	//SensesContainer.OnNoLongerSensedByAnyOtherDelegate.AddUObject(this, &UBattleSubsystem::OnNoLongerSensedByAnyOther);
}

void UBattleSubsystem::DeinitializeDelegates()
{
	//TMap<BattleSubsystemTypes::TeamIDType, TSet<TWeakObjectPtr<AMyCharacterBase>>> TeamMap;
	for (auto It{ TeamMembersMap.CreateIterator() }; It; ++It)
	{
		for (auto ValueIt{ It->Value.CreateIterator() }; ValueIt; ++ValueIt)
		{
			if (ValueIt->IsValid())
			{
				(*ValueIt)->OnSenseUpdatedDelegate.RemoveAll(this);
				(*ValueIt)->OnEndPlay.RemoveAll(this);
			}
		}
	}
}

TArray<TWeakObjectPtr<AActor>> UBattleSubsystem::CollectModifiedActors(const FVector& StartLocation, const FVector& EndLocation)
{
	TSet<TWeakObjectPtr<AActor>> Actors;

	const FVector2D StartLocationXY{ StartLocation };
	const FVector2D DirectionToDestination{ EndLocation - StartLocation };
	const double DistanceSquareToDestination{ FVector2D::DistSquared(FVector2D{EndLocation} , StartLocationXY) };

	for (auto It{ BattleFieldVolumes.CreateConstIterator() }; It; ++It)
	{
		const ABattleFieldVolume* Volume{ It->Key.Get() };
		if (!ensure(Volume)) { continue; }

		const FBox VolumeAABB{ Volume->GetBounds().GetBox() };
		if (VolumeAABB.IsInsideOrOnXY(StartLocation) || VolumeAABB.IsInsideOrOnXY(EndLocation))
		{
			for (auto ValueIt{ It->Value.CreateConstIterator() }; ValueIt; ++ValueIt)
			{
				if (!ValueIt->IsValid()) { continue; }

				AActor* NavModifiedActor{ ValueIt->Get() };
				const FVector2D NavModifiedActorLocationXY{ NavModifiedActor->GetActorLocation() };

				//�� DirectionToDestination ��ͬ��
				if (FVector2D::DotProduct(NavModifiedActorLocationXY - StartLocationXY, DirectionToDestination) < 0) { continue; }

				//������� DistanceSquareToDestination
				if (FVector2D::DistSquared(NavModifiedActorLocationXY, StartLocationXY) > DistanceSquareToDestination) { continue; }

				INavModifiedActorInterface* NavModifiedActorInterface{ Cast<INavModifiedActorInterface>(NavModifiedActor) };
				if (!ensure(NavModifiedActorInterface)) { continue; }

				Actors.Add(ValueIt->Get());
			}
		}
	}

	return Actors.Array();
}

void UBattleSubsystem::OnSenseUpdated(const bool bSuccessfullySensed, AMyCharacterBase* Observer, AMyCharacterBase* Enemy)
{
	if (!Observer || !Enemy) { return; }
	if (!ensure(!bSuccessfullySensed || CheckCharacterRegistered(Observer)) || !ensure(!bSuccessfullySensed || CheckCharacterRegistered(Enemy))) { return; }

	const BattleSubsystemTypes::TeamIDType TeamID{ Observer->GetGenericTeamId().GetId() };

	if (FTeamSensesContainer* Container{ TeamSensesMap.Find(TeamID) }; bSuccessfullySensed)
	{
		if (!Container)
		{
			Container = &TeamSensesMap.Add(TeamID);
			Container->OnTeamSenseAddedDelegate.AddUObject(this, &UBattleSubsystem::OnTeamSenseAdded);
			Container->OnNoLongerSensedByAnyTeamMemberDelegate.AddUObject(this, &UBattleSubsystem::OnNoLongerSensedByAnyTeamMember);
		}
		
		Container->OnSenseUpdated(FSenseUpdateInfo{bSuccessfullySensed, Observer, Enemy});
	}
	else if (Container) { Container->OnSenseUpdated(FSenseUpdateInfo{bSuccessfullySensed, Observer, Enemy}); }
}

void UBattleSubsystem::OnTeamMemberEndPlay(AActor* TeamMember, const EEndPlayReason::Type EndPlayReason)
{
	UnregisterToBattleSubsystem(Cast<AMyCharacterBase>(TeamMember));
}

void UBattleSubsystem::OnNoLongerSensedByAnyOther(const AActor* SensedActor)
{
	//for (auto It{ TeamEnemiesMap.CreateIterator() }; It; ++It)
	//{
	//	for (auto EnemiesIt{ It->Value.CreateIterator() }; EnemiesIt; ++EnemiesIt)
	//	{
	//		if (!EnemiesIt->IsValid() || EnemiesIt->Get() == SensedActor) { EnemiesIt.RemoveCurrent(); }
	//	}

	//	if (It->Value.IsEmpty()) { It.RemoveCurrent(); }
	//}
}

void UBattleSubsystem::OnTeamSenseAdded(const BattleSubsystemTypes::TeamIDType TeamID, AActor& SensedActor)
{
	AMyCharacterBase* Enemy{ Cast<AMyCharacterBase>(&SensedActor) };
	if (!Enemy) { return; }

	if (TSet<TWeakObjectPtr<AMyCharacterBase>>*TeamMembers{ TeamMembersMap.Find(TeamID) })
	{
		for (auto It{ TeamMembers->CreateIterator() }; It; ++It)
		{
			if (It->IsValid()) { (*It)->OnEnemySensed(Enemy); }
		}
	}
}

void UBattleSubsystem::OnNoLongerSensedByAnyTeamMember(const BattleSubsystemTypes::TeamIDType TeamID, const AActor& SensedActor)
{
	const AMyCharacterBase* Enemy{ Cast<AMyCharacterBase>(&SensedActor) };
	if (!Enemy) { return; }

	if (TSet<TWeakObjectPtr<AMyCharacterBase>>*TeamMembers{ TeamMembersMap.Find(TeamID) })
	{
		for (auto It{ TeamMembers->CreateIterator() }; It; ++It)
		{
			if (It->IsValid()) { (*It)->OnEnemyDisappear(Enemy); }
		}
	}
}

bool UBattleSubsystem::CheckCharacterRegistered(const AMyCharacterBase* Character) const
{
	const BattleSubsystemTypes::TeamIDType TeamID{ Character->GetGenericTeamId().GetId() };
	if (const TSet<TWeakObjectPtr<AMyCharacterBase>>*Members{ TeamMembersMap.Find(TeamID) })
	{
		return Members->Contains(Character);
	}

	return false;
}

void UBattleSubsystem::UpdateAllSensesActor(const bool bIsSensed, AMyCharacterBase& Observer)
{
	if (UAIPerceptionComponent * Component{ Observer.GetController() ? Observer.GetController()->FindComponentByClass<UAIPerceptionComponent>() : nullptr })
	{
		TArray<AActor*> Actors;
		Component->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), Actors);
		for (AActor* Actor : Actors)
		{
			if (AMyCharacterBase* EnemyCharacter{ Cast<AMyCharacterBase>(Actor) })
			{
				OnSenseUpdated(bIsSensed, &Observer, EnemyCharacter);
			}
		}
	}
}

void UBattleSubsystem::InitializeBattleFieldVolumes()
{
	TArray<AActor*> Volumes;
	UGameplayStatics::GetAllActorsOfClass(this, ABattleFieldVolume::StaticClass(), Volumes);

	for (const AActor* Actor : Volumes)
	{
		if (const ABattleFieldVolume * Volume{ Cast<ABattleFieldVolume>(Actor) })
		{
			OnBattleFieldVolumeBeginPlay(Volume);
		}
	}
}

void UBattleSubsystem::InitializeNavModifiedActors()
{
	TArray<AActor*> NavModifiedActors;
	UGameplayStatics::GetAllActorsWithInterface(this, UNavModifiedActorInterface::StaticClass(), NavModifiedActors);
	for (AActor* Actor : NavModifiedActors)
	{
		OnNavModifiedActorBeginPlay(Actor);
	}
}

void UBattleSubsystem::InitializeCharacters()
{
	TArray<AActor*> OutActors;
	TArray<AActor*> OutActors2;
	UGameplayStatics::GetAllActorsOfClass(this, AMyCharacterBase::StaticClass(), OutActors);

	for (AActor* Actor : OutActors)
	{
		if (AMyCharacterBase * Ch{ Cast<AMyCharacterBase>(Actor) }) { RegisterToBattleSubsystem(Ch); }
	}
}