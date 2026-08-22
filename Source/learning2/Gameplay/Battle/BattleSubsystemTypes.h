#pragma once

#include "CoreMinimal.h"
#include "Delegates/DelegateCombinations.h"
#include "BattleSubsystemTypes.generated.h"

class AActor;
class AMyCharacterBase;

namespace BattleSubsystemTypes
{
	using TeamIDType = uint8;
	using EnemyType = AMyCharacterBase;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnNoLongerSensedByAnyOtherDelegate, const AActor* SensedActor);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnTeamSenseAddedDelegate, BattleSubsystemTypes::TeamIDType TeamID, AActor& SensedActor);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnNoLongerSensedByAnyTeamMemberDelegate, BattleSubsystemTypes::TeamIDType TeamID, const AActor& SensedActor);

namespace BattleSubsystemConst
{
	const FVector NoNavLocation{ 0., 0., -1e5 };
	namespace BlackboardKeyName
	{
		const FName SelfActor{ TEXT("SelfActor") };
		const FName EnemyCharacter{ TEXT("EnemyCharacter") };
		const FName ControlledWeaponAmmo{ TEXT("ControlledWeaponAmmo") };
		const FName UncontrolledWeaponAmmo{ TEXT("UncontrolledWeaponAmmo") };
		const FName TargetLocation{ TEXT("TargetLocation") };
		const FName ControlledWeaponSlot{ TEXT("ControlledWeaponSlot") };
		const FName SensedCharacter{ TEXT("SensedEnemyCharacter") };
		const FName ControlledWeaponMaxAmmo{ TEXT("ControlledWeaponMaxAmmo") };
		const FName UncontrolledWeaponMaxAmmo{ TEXT("UncontrolledWeaponMaxAmmo") };
	}
}

USTRUCT(BlueprintType)
struct FSenseUpdateInfo
{
	GENERATED_BODY()

	bool IsValid() const { return Observer.IsValid() && Enemy.IsValid(); }

	UPROPERTY(BlueprintReadWrite, Category = "Sense Update Info")
	bool bSuccessfullySensed{ false };

	UPROPERTY(BlueprintReadWrite, Category = "Sense Update Info")
	TWeakObjectPtr<AActor> Observer{ nullptr };

	UPROPERTY(BlueprintReadWrite, Category = "Sense Update Info")
	TWeakObjectPtr<AActor> Enemy{ nullptr };
};

USTRUCT(BlueprintType)
struct FSenseHandle
{
	GENERATED_BODY()

	FSenseHandle() = default;
	explicit FSenseHandle(AActor& Actor);

	inline bool IsValid() const { return Handle != INDEX_NONE && Actor.IsValid(); }

private:
	void GenerateNewHandle();

public:
	TWeakObjectPtr<AActor> Actor{ nullptr };

private:
	int32 Handle{ INDEX_NONE };

public:
	bool operator==(const FSenseHandle& Other) const { return Handle == Other.Handle; }

	bool operator!=(const FSenseHandle& Other) const { return !operator==(Other); }

	/** Operator to expose FSenseHandle serialization to custom serialization functions like NetSerialize overrides. */
	friend FArchive& operator<<(FArchive& Ar, FSenseHandle& Value)
	{
		static_assert(sizeof(FSenseHandle) == 12, "If properties of FSenseHandle change, consider updating this operator implementation.");
		Ar << Value.Handle;
		Value.Actor.Serialize(Ar);

		return Ar;
	}

	friend uint32 GetTypeHash(const FSenseHandle& SpecHandle)
	{
		return ::GetTypeHash(SpecHandle.Handle);
	}

	FString ToString() const
	{
		return IsValid() ? FString("Handle: ") + FString::FromInt(Handle) + FString("\tActor: ") + GetNameSafe(Actor.Get()) : TEXT("Invalid");
	}
};

struct UE_DEPRECATED(5.7, "Use FTeamSensesContainer instead.") FSensesContainer
{
	FSensesContainer();
	~FSensesContainer();

	void RegisterSense(AActor& Observer, AActor& Enemy);
	void UnregisterSense(AActor& Observer, AActor& Enemy);

	TArray<AActor*> GetObserverSensesEnemies(const AActor& Observer);
	TArray<AActor*> GetEnemySensedObservers(const AActor& Enemy);

	bool IsObserverSenseToEnemy(const AActor& Observer, const AActor& Enemy) const;

	void OnActorDestoryed(const AActor& Observer);

private:
	void OnTeamSenseUpdated(const bool bSensingSucceeded, const AActor* Observer, const AActor* Enemy);

	void OnNoLongerSensedByAnyOther(const AActor* SensedActor);

public:
	FOnNoLongerSensedByAnyOtherDelegate OnNoLongerSensedByAnyOtherDelegate;
	//FOnTeamSenseUpdatedDelegate OnTeamSenseUpdatedDelegate;

private:
	class Impl;
	TUniquePtr<Impl> pImpl;
};

USTRUCT(BlueprintType)
struct FNavigationModifyHandle
{
	GENERATED_BODY()

	FNavigationModifyHandle();

	inline bool IsValid() const { return Handle != INDEX_NONE; }

private:
	void GenerateNewHandle();

private:
	int32 Handle{ INDEX_NONE };

public:
	bool operator==(const FNavigationModifyHandle& Other) const { return Handle == Other.Handle; }

	bool operator!=(const FNavigationModifyHandle& Other) const { return !operator==(Other); }

	/** Operator to expose FSenseHandle serialization to custom serialization functions like NetSerialize overrides. */
	friend FArchive& operator<<(FArchive& Ar, FNavigationModifyHandle& Value)
	{
		static_assert(sizeof(FNavigationModifyHandle) == 4, "If properties of FNavigationModifyHandle change, consider updating this operator implementation.");
		Ar << Value.Handle;

		return Ar;
	}

	friend uint32 GetTypeHash(const FNavigationModifyHandle& SpecHandle)
	{
		return ::GetTypeHash(SpecHandle.Handle);
	}

	FString ToString() const
	{
		//return IsValid() ?
		//	FString("Handle: ") + FString::FromInt(Handle) + FString("\tStartLocation: ") + StartLocation.ToString() + FString("\EndLocation: ") + EndLocation.ToString() :
		//	TEXT("Invalid");
		return IsValid() ? FString::FromInt(Handle) : TEXT("Invalid");
	}
};

struct FTeamSensesContainer
{
	FTeamSensesContainer();
	~FTeamSensesContainer();

	FTeamSensesContainer(FTeamSensesContainer&&);
	FTeamSensesContainer& operator=(FTeamSensesContainer&&);

	FTeamSensesContainer(const FTeamSensesContainer&) = delete;
	FTeamSensesContainer& operator=(const FTeamSensesContainer&) = delete;

	void OnSenseUpdated(const FSenseUpdateInfo& SenseUpdateInfo);

	AActor* GetOneTeamSensedActor() const;

	void OnActorEndPlayed(const AActor& EndPlayedActor);

private:
	void OnTeamSenseAdded(const BattleSubsystemTypes::TeamIDType TeamID, AActor& SensedActor);
	void OnNoLongerSensedByAnyTeamMember(const BattleSubsystemTypes::TeamIDType TeamID, const AActor& SensedActor) const;

public:
	FOnTeamSenseAddedDelegate OnTeamSenseAddedDelegate;
	FOnNoLongerSensedByAnyTeamMemberDelegate OnNoLongerSensedByAnyTeamMemberDelegate;

private:
	class Impl;
	TUniquePtr<Impl> pImpl;

	//TMap<TWeakObjectPtr<TeamMemberType>, TSet<TWeakObjectPtr<EnemyType>>> TeamMemberToEnemiesMap;

};
