// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Delegates/DelegateCombinations.h"
#include "Subsystems/WorldSubsystem.h"
// #include "Tickable.h"
#include "GenericQuadTree.h"
#include "BattleSubsystemTypes.h"
#include "Perception/AIPerceptionTypes.h"
#include "BattleSubsystem.generated.h"

class AMyCharacterBase;
class ABattleFieldVolume;

//OnCharacterMovementUpdated

USTRUCT(BlueprintType)
struct FBattleFieldCharacterHandle
{
	GENERATED_BODY()

private:
	TWeakObjectPtr<ABattleFieldVolume> BattleField;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	int32 Handle{ INDEX_NONE };
};

/**
 *
 */
UCLASS()
class LEARNING2_API UBattleSubsystem : public UWorldSubsystem/*, public FTickableGameObject*/
{
	GENERATED_BODY()

public:
		using EnemyType = BattleSubsystemTypes::EnemyType;
	
		virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
		// virtual FORCEINLINE bool IsTickable() const override { return !IsTemplate(); }//����CDO��Tick
		// virtual FORCEINLINE TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UBattleSubsystem, STATGROUP_Tickables); }
	
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// virtual void Tick(float DeltaTime) override {};

public:
	[[nodiscard]] virtual FNavigationModifyHandle ActivateNavModify(const FVector& StartLocation, const FVector& EndLocation);
	virtual void DeactivateNavModify(const FNavigationModifyHandle& Handle);

	virtual void OnBattleFieldVolumeBeginPlay(const ABattleFieldVolume* Volume);
	virtual void OnBattleFieldVolumeEndPlay(const ABattleFieldVolume* Volume, const EEndPlayReason::Type EndPlayReason);
	virtual void OnNavModifiedActorBeginPlay(AActor* NavModifiedActor);
	virtual void OnNavModifiedActorEndPlay(AActor* NavModifiedActor, const EEndPlayReason::Type EndPlayReason);

	virtual void RegisterToBattleSubsystem(AMyCharacterBase* Character);
	virtual void UnregisterToBattleSubsystem(AMyCharacterBase* Character);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Is Sensed By Team"))
	bool K2_IsSensedByTeam(int32 TeamID, const AMyCharacterBase* Enemy) const;
	bool IsSensedByTeam(BattleSubsystemTypes::TeamIDType TeamID, const AMyCharacterBase& Enemy) const;

	AActor* GetOneTeamSensedActor(const BattleSubsystemTypes::TeamIDType TeamID) const;

protected:
	virtual void InitializeDelegates();
	virtual void DeinitializeDelegates();

	virtual TArray<TWeakObjectPtr<AActor>> CollectModifiedActors(const FVector& StartLocation, const FVector& EndLocation);

	virtual void OnSenseUpdated(const bool bSuccessfullySensed, AMyCharacterBase* Observer, AMyCharacterBase* Enemy);

	UFUNCTION()
	virtual void OnTeamMemberEndPlay(AActor* TeamMember, const EEndPlayReason::Type EndPlayReason);

	virtual void OnNoLongerSensedByAnyOther(const AActor* SensedActor);

	virtual void OnTeamSenseAdded(const BattleSubsystemTypes::TeamIDType TeamID, AActor& SensedActor);
	virtual void OnNoLongerSensedByAnyTeamMember(const BattleSubsystemTypes::TeamIDType TeamID, const AActor& SensedActor);

	bool CheckCharacterRegistered(const AMyCharacterBase* Character) const;

	void UpdateAllSensesActor(const bool bIsSensed, AMyCharacterBase& Observer);

private:
	void InitializeBattleFieldVolumes();
	void InitializeNavModifiedActors();
	void InitializeCharacters();

public:
	//FOnTeamSenseUpdatedDelegate OnTeamSenseUpdatedDelegate;

private:
	//TQuadTree<>
	//UPROPERTY()
	//FSensesContainer SensesContainer;
	TMap<BattleSubsystemTypes::TeamIDType, FTeamSensesContainer> TeamSensesMap;

	TMap<TWeakObjectPtr<const ABattleFieldVolume>, TArray<TWeakObjectPtr<AActor>>> BattleFieldVolumes;

	TMap<TWeakObjectPtr<AActor>, TArray<FNavigationModifyHandle>> ActivatedModifyActorsMap;

	TMap<BattleSubsystemTypes::TeamIDType, TSet<TWeakObjectPtr<AMyCharacterBase>>> TeamMembersMap;
	//TMap<BattleSubsystemTypes::TeamIDType, TSet<TWeakObjectPtr<EnemyType>>> TeamEnemiesMap;
};