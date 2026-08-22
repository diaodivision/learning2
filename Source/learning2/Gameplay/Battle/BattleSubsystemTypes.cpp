#include "BattleSubsystemTypes.h"
#include "GameFramework/Actor.h"
#include "Misc/CoreDelegates.h"
#include "Tickable.h"
#include "GenericTeamAgentInterface.h"

FSenseHandle::FSenseHandle(AActor& Actor) : Actor(Cast<IGenericTeamAgentInterface>(&Actor) ? &Actor : nullptr)
{
	GenerateNewHandle();
}

void FSenseHandle::GenerateNewHandle()
{
	static int32 GHandle = 1;
	Handle = GHandle++;
}

class FSensesContainer::Impl : public FTickableGameObject
{
public:
	using FEnemyHandle = FSenseHandle;
	using FObserverHandle = FSenseHandle;
	using ActorPtrType = TWeakObjectPtr<AActor>;

public:
	[[nodiscard]] FObserverHandle RegisterSense(AActor& Observer, AActor& Enemy);

	void UnregisterSense(AActor& Observer, AActor& Enemy);

	bool IsObserverSenseToEnemy(const AActor& Observer, const AActor& Enemy) const;

	inline void OnActorDestoryed(const FObserverHandle Handle) { OnDataInvalidated(Handle); }
	inline void OnActorDestoryed(const AActor& Observer) { if (const FObserverHandle* Handle = ActorToHandleMap.Find(&Observer)) { OnActorDestoryed(*Handle); } }

	TArray<AActor*> GetObserverSensesEnemies(const AActor& Observer);
	TArray<AActor*> GetEnemySensedObservers(const AActor& Enemy);

private:
	//��� Actor ����Щ���˿�����
	TMap<FEnemyHandle, TSet<FObserverHandle>> EnemyToObserversMap;

	//��� Actor ��������Щ����
	TMap<FObserverHandle, TSet<FEnemyHandle>> ObserverToEnemiesMap;

public:
	inline int32 GetClearMaxStep() const { return ClearMaxStep; }
	inline void SetClearMaxStep(int32 NewMaxStep) { ClearMaxStep = FMath::Max(0, NewMaxStep); }

private:
	void ClearInvalidData();

	void OnDataInvalidated(const FObserverHandle Handle);

	void RegisterSense_Internal(const FObserverHandle ObserverHandle, const FEnemyHandle EnemyHandle);
	void UnregisterSense_Internal(const FObserverHandle ObserverHandle, const FEnemyHandle EnemyHandle);

	void OnNoLongerSensedByAnyOther(const AActor* SensedActor) const;

public:
	//virtual void Tick(float DeltaTime) override;
	//virtual inline  bool IsTickable() const override { return true; }
	//virtual inline TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UBattleSubsystem, STATGROUP_Tickables); }

	bool CanAddActor(const AActor& Actor);
	bool CanAddEnemy(const IGenericTeamAgentInterface& Observer, const AActor& Enemy);

	static IGenericTeamAgentInterface* AsGenericTeamAgentInterface(AActor* Actor);
	static const IGenericTeamAgentInterface* AsGenericTeamAgentInterface(const AActor* Actor);

public:
	//FOnTeamSenseUpdatedDelegate OnTeamSenseUpdatedDelegate;
	FOnNoLongerSensedByAnyOtherDelegate OnNoLongerSensedByAnyOtherDelegate;

private:
	TMap<ActorPtrType, FObserverHandle> ActorToHandleMap;

	int32 LastIndex{ 0 };
	int32 ClearMaxStep{ 50 };
};

FSensesContainer::FSensesContainer()
{
	pImpl->OnNoLongerSensedByAnyOtherDelegate.AddRaw(this, &FSensesContainer::OnNoLongerSensedByAnyOther);
}

FSensesContainer::~FSensesContainer()
{
	pImpl->OnNoLongerSensedByAnyOtherDelegate.RemoveAll(this);
}

void FSensesContainer::RegisterSense(AActor& Observer, AActor& Enemy)
{
	FSensesContainer::Impl::FObserverHandle Handle = pImpl->RegisterSense(Observer, Enemy);
}

void FSensesContainer::UnregisterSense(AActor& Observer, AActor& Enemy)
{
	pImpl->UnregisterSense(Observer, Enemy);
}

TArray<AActor*> FSensesContainer::GetObserverSensesEnemies(const AActor& Observer)
{
	return pImpl->GetObserverSensesEnemies(Observer);
}

TArray<AActor*> FSensesContainer::GetEnemySensedObservers(const AActor& Enemy)
{
	return pImpl->GetEnemySensedObservers(Enemy);
}

bool FSensesContainer::IsObserverSenseToEnemy(const AActor& Observer, const AActor& Enemy) const
{
	return pImpl->IsObserverSenseToEnemy(Observer, Enemy);
}

void FSensesContainer::OnActorDestoryed(const AActor& Observer)
{
	pImpl->OnActorDestoryed(Observer);
}

void FSensesContainer::OnTeamSenseUpdated(const bool bSuccessfullySensed, const AActor* Observer, const AActor* Enemy)
{
	//OnTeamSenseUpdatedDelegate.Broadcast(bSuccessfullySensed, Observer, Enemy);
}

void FSensesContainer::OnNoLongerSensedByAnyOther(const AActor* SensedActor)
{
	if (!SensedActor) { return; }

	OnNoLongerSensedByAnyOtherDelegate.Broadcast(SensedActor);
}

FSensesContainer::Impl::FObserverHandle FSensesContainer::Impl::RegisterSense(AActor& Observer, AActor& Enemy)
{
	if (!CanAddActor(Observer)) { return FObserverHandle{}; }

	const FObserverHandle* ObserverHandlePtr = ActorToHandleMap.Find(&Observer);
	if (!ObserverHandlePtr || !ObserverHandlePtr->IsValid())
	{
		const FObserverHandle Handle{ Observer };
		//EnemyToObserversMap.Add(Handle);
		//ObserverToEnemiesMap.Add(Handle);
		ObserverHandlePtr = &ActorToHandleMap.Add(&Observer, Handle);
	}

	RegisterSense_Internal(FObserverHandle{ Observer }, FEnemyHandle{ Enemy });

	return *ObserverHandlePtr;
}

void FSensesContainer::Impl::UnregisterSense(AActor& Observer, AActor& Enemy)
{
	//const FObserverHandle* ObserverHandlePtr = ActorToHandleMap.Find(&Observer);
	//if (!ObserverHandlePtr || !ObserverHandlePtr->IsValid()) { return; }

	UnregisterSense_Internal(FObserverHandle{ Observer }, FEnemyHandle{ Enemy });
}

bool FSensesContainer::Impl::IsObserverSenseToEnemy(const AActor& Observer, const AActor& Enemy) const
{
	const FObserverHandle* ObserverHandlePtr = ActorToHandleMap.Find(&Observer);
	const FObserverHandle* EnemyHandlePtr = ActorToHandleMap.Find(&Enemy);
	if (!ObserverHandlePtr || !ObserverHandlePtr->IsValid() || !EnemyHandlePtr || !EnemyHandlePtr->IsValid()) { return false; }

	const TSet<FEnemyHandle>* Enemies = ObserverToEnemiesMap.Find(*ObserverHandlePtr);
	return Enemies && Enemies->Contains(*EnemyHandlePtr);
}

TArray<AActor*> FSensesContainer::Impl::GetObserverSensesEnemies(const AActor& Observer)
{
	////��� Actor ����Щ���˿�����
	//TMap<FEnemyHandle, TSet<FObserverHandle>> EnemyToObserversMap;

	////��� Actor ��������Щ����
	//TMap<FObserverHandle, TSet<FEnemyHandle>> ObserverToEnemiesMap;

	TArray<AActor*> Enemies;

	if (const FObserverHandle * Handle{ ActorToHandleMap.Find(&Observer) })
	{
		if (TSet<FEnemyHandle>*EnemyHandles{ ObserverToEnemiesMap.Find(*Handle) })
		{
			for (FEnemyHandle& EnemyHandle : *EnemyHandles)
			{
				if (EnemyHandle.Actor.IsValid()) { Enemies.Add(EnemyHandle.Actor.Get()); }

			}
		}
	}

	return Enemies;
}

TArray<AActor*> FSensesContainer::Impl::GetEnemySensedObservers(const AActor& Enemy)
{
	TArray<AActor*> Observers;

	if (const FObserverHandle * Handle{ ActorToHandleMap.Find(&Enemy) })
	{
		if (TSet<FObserverHandle>*ObserverHandles{ EnemyToObserversMap.Find(*Handle) })
		{
			for (FEnemyHandle& EnemyHandle : *ObserverHandles)
			{
				if (EnemyHandle.Actor.IsValid()) { Observers.Add(EnemyHandle.Actor.Get()); }

			}
		}
	}

	return Observers;
}

void FSensesContainer::Impl::ClearInvalidData()
{

	if (ClearMaxStep == 0 || ActorToHandleMap.IsEmpty()) { return; }

	const int32 MaxIndex{ ActorToHandleMap.GetMaxIndex() };
	if (MaxIndex <= LastIndex) { LastIndex = 0; }

	const int32 StartIndex{ LastIndex };
	const int32 EndIndex{ FMath::Min(StartIndex + ClearMaxStep, MaxIndex) };
	{
		for (int32 i{ StartIndex }; i < EndIndex; i++)
		{
			if (FSetElementId ID{ FSetElementId::FromInteger(i) }; ActorToHandleMap.IsValidId(ID))
			{
				if (const TPair<const ActorPtrType, const FObserverHandle>& Element{ ActorToHandleMap.Get(ID) }; !Element.Key.IsValid())
				{
					OnDataInvalidated(Element.Value);
					ActorToHandleMap.Remove(ID);
				}
			}
		}
	}

	LastIndex = EndIndex;
	if (EndIndex >= ActorToHandleMap.GetMaxIndex())
	{
		if (ActorToHandleMap.Num() < ActorToHandleMap.GetMaxIndex() / 2)
		{
			ActorToHandleMap.Compact();
			ActorToHandleMap.Shrink(); // ����ɶԳ���
		}

		LastIndex = 0;
	}
}

void FSensesContainer::Impl::OnDataInvalidated(const FObserverHandle InvalidatedHandle)
{
	//TSet<const AActor*> UnregisterSense;

	if (TSet<FEnemyHandle> OutEnemies; ObserverToEnemiesMap.RemoveAndCopyValue(InvalidatedHandle, OutEnemies))
	{
		for (const FEnemyHandle& Enemy : OutEnemies)
		{
			if (TSet<FObserverHandle>* Observers = EnemyToObserversMap.Find(Enemy))
			{
				Observers->Remove(InvalidatedHandle);
				if (Observers->IsEmpty())
				{
					OnNoLongerSensedByAnyOther(Enemy.Actor.Get());
					EnemyToObserversMap.Remove(Enemy);
				}
			}

			//if (InvalidatedHandle.Actor.IsValid() && Enemy.Actor.IsValid())
			//{
			//	OnTeamSenseUpdatedDelegate.Broadcast(false, InvalidatedHandle.Actor.Get(), Enemy.Actor.Get());
			//}
		}
	}

	if (TSet<FObserverHandle> OutObservers; EnemyToObserversMap.RemoveAndCopyValue(static_cast<FEnemyHandle>(InvalidatedHandle), OutObservers))
	{
		for (const FObserverHandle& Observer : OutObservers)
		{
			if (TSet<FEnemyHandle>* Enemies = ObserverToEnemiesMap.Find(Observer))
			{
				Enemies->Remove(static_cast<FEnemyHandle>(InvalidatedHandle));
				if (Enemies->IsEmpty()) { ObserverToEnemiesMap.Remove(Observer); }

				//if (Observer.Actor.IsValid() && InvalidatedHandle.Actor.IsValid())
				//{
				//	OnTeamSenseUpdatedDelegate.Broadcast(false, Observer.Actor.Get(), InvalidatedHandle.Actor.Get());
				//}
			}
		}
	}

	//if (const TSet<const FObserverHandle>* ObserverHandles = EnemyToObserversMap.Find(Handle))
	//{
	//	for (const FObserverHandle& ObserverHandle : *ObserverHandles)
	//	{
	//		if (!ObserverHandle.IsValid()) { continue; }

	//		if (TSet<const FEnemyHandle>* FEnemyHandles = ObserverToEnemiesMap.Find(ObserverHandle))
	//		{
	//			FEnemyHandles->Remove(ObserverHandle);
	//		}
	//	}

	//	EnemyToObserversMap.Remove(Handle);
	//}

	//if (const TSet<const FEnemyHandle>* EnemyHandles = ObserverToEnemiesMap.Find(Handle))
	//{
	//	for (const FEnemyHandle& EnemyHandle : *EnemyHandles)
	//	{
	//		if (!EnemyHandle.IsValid()) { continue; }

	//		if (TSet<const FObserverHandle>* ObserverHandles = EnemyToObserversMap.Find(EnemyHandle))
	//		{
	//			ObserverHandles->Remove(EnemyHandle);
	//		}
	//	}

	//	ObserverToEnemiesMap.Remove(Handle);
	//}
}

void FSensesContainer::Impl::RegisterSense_Internal(const FObserverHandle ObserverHandle, const FEnemyHandle EnemyHandle)
{
	if (!ObserverHandle.IsValid() || !EnemyHandle.IsValid()) { return; }

	//if (!CanAddActor(*ObserverHandle.Actor)) { return; }

	if (CanAddEnemy(*AsGenericTeamAgentInterface(ObserverHandle.Actor.Get()), *EnemyHandle.Actor))
	{
		//if (TSet<FEnemyHandle>* Enemies = ObserverToEnemiesMap.Find(ObserverHandle); Enemies && Enemies->Contains(EnemyHandle))
		//{
		//	OnTeamSenseUpdatedDelegate.Broadcast(true, ObserverHandle.Actor.Get(), EnemyHandle.Actor.Get());
		//}

		ObserverToEnemiesMap.FindOrAdd(ObserverHandle).Add(EnemyHandle);
		EnemyToObserversMap.FindOrAdd(EnemyHandle).Add(ObserverHandle);

	}
}

void FSensesContainer::Impl::UnregisterSense_Internal(const FObserverHandle ObserverHandle, const FEnemyHandle EnemyHandle)
{
	if (TSet<FEnemyHandle>* Enemies = ObserverToEnemiesMap.Find(ObserverHandle))
	{
		//if (ObserverHandle.IsValid() && EnemyHandle.IsValid() && Enemies->Contains(EnemyHandle))
		//{
		//	OnTeamSenseUpdatedDelegate.Broadcast(false, ObserverHandle.Actor.Get(), EnemyHandle.Actor.Get());
		//}

		Enemies->Remove(EnemyHandle);
		if (Enemies->IsEmpty()) { ObserverToEnemiesMap.Remove(ObserverHandle); }
	}

	//if (TSet<FObserverHandle>* Observers = EnemyToObserversMap.Find())
	//{
	//	Observers->Remove(ObserverHandle);
	//	if (Observers->IsEmpty())
	//	{
	//		OnNoLongerSensedByAnyOther(EnemyHandle.Actor.Get());
	//		EnemyToObserversMap.Remove(EnemyHandle);
	//	}
	//}
}

void FSensesContainer::Impl::OnNoLongerSensedByAnyOther(const AActor* SensedActor) const
{
	if (!SensedActor) { return; }

	OnNoLongerSensedByAnyOtherDelegate.Broadcast(SensedActor);
}

//void FSensesContainer::Impl::Tick(float DeltaTime)
//{
//	ClearInvalidData();
//}

bool FSensesContainer::Impl::CanAddActor(const AActor& Actor)
{
	return !!AsGenericTeamAgentInterface(&Actor);
}

bool FSensesContainer::Impl::CanAddEnemy(const IGenericTeamAgentInterface& Observer, const AActor& Enemy)
{
	return CanAddActor(Enemy) && Observer.GetTeamAttitudeTowards(Enemy) == ETeamAttitude::Hostile;
}

IGenericTeamAgentInterface* FSensesContainer::Impl::AsGenericTeamAgentInterface(AActor* Actor)
{
	return Cast<IGenericTeamAgentInterface>(Actor);
}

const IGenericTeamAgentInterface* FSensesContainer::Impl::AsGenericTeamAgentInterface(const AActor* Actor)
{
	return Cast<const IGenericTeamAgentInterface>(Actor);
}

FNavigationModifyHandle::FNavigationModifyHandle()
{
	GenerateNewHandle();
}

void FNavigationModifyHandle::GenerateNewHandle()
{
	static int32 GHandle = 1;
	Handle = GHandle++;
}

class FTeamSensesContainer::Impl
{
	using TeamMemberType = AActor;
	using EnemyType = AActor;

public:
	void OnSenseUpdated(const FSenseUpdateInfo& SenseUpdateInfo);

	AActor* GetOneTeamSensedActor();

	void OnActorEndPlayed(const AActor& EndPlayedActor);

public:
	FOnTeamSenseAddedDelegate OnTeamSenseAddedDelegate;
	FOnNoLongerSensedByAnyTeamMemberDelegate OnNoLongerSensedByAnyTeamMemberDelegate;

private:
	TMap<TWeakObjectPtr<EnemyType>, TSet<TWeakObjectPtr<TeamMemberType>>> EnemyToTeamMembersMap;
	TSet<TWeakObjectPtr<EnemyType>> TeamSensesEnemies;
};

void FTeamSensesContainer::Impl::OnSenseUpdated(const FSenseUpdateInfo& SenseUpdateInfo)
{
	if (!SenseUpdateInfo.IsValid()) { return; }

	TSet<TWeakObjectPtr<TeamMemberType>>* TeamMembers{ EnemyToTeamMembersMap.Find(SenseUpdateInfo.Enemy) };

	if (SenseUpdateInfo.bSuccessfullySensed)
	{
		if (!TeamMembers) { TeamMembers = &EnemyToTeamMembersMap.Add(SenseUpdateInfo.Enemy); }

		TeamMembers->Add(SenseUpdateInfo.Observer);
		TeamSensesEnemies.Add(SenseUpdateInfo.Enemy);

		if (IGenericTeamAgentInterface * GenericTeamAgentInterface{ Cast<IGenericTeamAgentInterface>(SenseUpdateInfo.Observer) })
		{
			OnTeamSenseAddedDelegate.Broadcast(GenericTeamAgentInterface->GetGenericTeamId().GetId(), *SenseUpdateInfo.Enemy.Get());
		}
	}
	else
	{
		TeamSensesEnemies.Remove(SenseUpdateInfo.Enemy);
		if (IGenericTeamAgentInterface * GenericTeamAgentInterface{ Cast<IGenericTeamAgentInterface>(SenseUpdateInfo.Observer) })
			{
				OnNoLongerSensedByAnyTeamMemberDelegate.Broadcast(GenericTeamAgentInterface->GetGenericTeamId().GetId(), *SenseUpdateInfo.Enemy.Get());
			}
		if (TeamMembers) { TeamMembers->Remove(SenseUpdateInfo.Observer); }
		// if (!TeamMembers)
		// {
		// 	TeamSensesEnemies.Remove(SenseUpdateInfo.Enemy);

		// 	if (IGenericTeamAgentInterface * GenericTeamAgentInterface{ Cast<IGenericTeamAgentInterface>(SenseUpdateInfo.Observer) })
		// 	{
		// 		OnNoLongerSensedByAnyTeamMemberDelegate.Broadcast(GenericTeamAgentInterface->GetGenericTeamId().GetId(), *SenseUpdateInfo.Enemy.Get());
		// 	}

		// }
		// else { TeamMembers->Remove(SenseUpdateInfo.Observer); }
	}
}

AActor* FTeamSensesContainer::Impl::GetOneTeamSensedActor()
{
	if (!TeamSensesEnemies.IsEmpty()) { return TeamSensesEnemies.begin()->Get(); }

	return nullptr;
}

void FTeamSensesContainer::Impl::OnActorEndPlayed(const AActor& EndPlayedActor)
{
	for (auto It{ EnemyToTeamMembersMap.CreateIterator() }; It; ++It)
	{
		if (It->Key == &EndPlayedActor) { It.RemoveCurrent(); }
		else { It->Value.Remove(&EndPlayedActor); }
	}

	TeamSensesEnemies.Remove(&EndPlayedActor);
}

FTeamSensesContainer::FTeamSensesContainer() : pImpl(MakeUnique<Impl>())
{
	pImpl->OnTeamSenseAddedDelegate.AddRaw(this, &FTeamSensesContainer::OnTeamSenseAdded);
	pImpl->OnNoLongerSensedByAnyTeamMemberDelegate.AddRaw(this, &FTeamSensesContainer::OnNoLongerSensedByAnyTeamMember);
}

FTeamSensesContainer::~FTeamSensesContainer()
{
	if (pImpl)
	{
		pImpl->OnTeamSenseAddedDelegate.RemoveAll(this);
		pImpl->OnNoLongerSensedByAnyTeamMemberDelegate.RemoveAll(this);
	}
}

FTeamSensesContainer::FTeamSensesContainer(FTeamSensesContainer&& Other)
{
	pImpl = MoveTemp(Other.pImpl);

	if (pImpl)
	{
		pImpl->OnTeamSenseAddedDelegate.RemoveAll(&Other);
		pImpl->OnNoLongerSensedByAnyTeamMemberDelegate.RemoveAll(&Other);

		pImpl->OnTeamSenseAddedDelegate.AddRaw(this, &FTeamSensesContainer::OnTeamSenseAdded);
		pImpl->OnNoLongerSensedByAnyTeamMemberDelegate.AddRaw(this, &FTeamSensesContainer::OnNoLongerSensedByAnyTeamMember);
	}
}

FTeamSensesContainer& FTeamSensesContainer::operator=(FTeamSensesContainer&& Other)
{
	if (this == &Other)
	{
		return *this;
	}

	if (pImpl)
	{
		pImpl->OnTeamSenseAddedDelegate.RemoveAll(this);
		pImpl->OnNoLongerSensedByAnyTeamMemberDelegate.RemoveAll(this);
	}

	pImpl = MoveTemp(Other.pImpl);

	if (pImpl)
	{
		pImpl->OnTeamSenseAddedDelegate.RemoveAll(&Other);
		pImpl->OnNoLongerSensedByAnyTeamMemberDelegate.RemoveAll(&Other);

		pImpl->OnTeamSenseAddedDelegate.AddRaw(this, &FTeamSensesContainer::OnTeamSenseAdded);
		pImpl->OnNoLongerSensedByAnyTeamMemberDelegate.AddRaw(this, &FTeamSensesContainer::OnNoLongerSensedByAnyTeamMember);
	}

	return *this;
}

void FTeamSensesContainer::OnSenseUpdated(const FSenseUpdateInfo& SenseUpdateInfo)
{
	pImpl->OnSenseUpdated(SenseUpdateInfo);
}

AActor* FTeamSensesContainer::GetOneTeamSensedActor()
{
	return pImpl->GetOneTeamSensedActor();
}

void FTeamSensesContainer::OnActorEndPlayed(const AActor& EndPlayedActor)
{
	pImpl->OnActorEndPlayed(EndPlayedActor);
}

void FTeamSensesContainer::OnTeamSenseAdded(const BattleSubsystemTypes::TeamIDType TeamID, AActor& SensedActor)
{
	OnTeamSenseAddedDelegate.Broadcast(TeamID, SensedActor);
}

void FTeamSensesContainer::OnNoLongerSensedByAnyTeamMember(const BattleSubsystemTypes::TeamIDType TeamID, const AActor& SensedActor) const
{
	OnNoLongerSensedByAnyTeamMemberDelegate.Broadcast(TeamID, SensedActor);
}