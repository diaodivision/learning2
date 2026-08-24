#pragma once

#include "Base/BattleGameMode.h"
#include "GameModeTypes.h"
#include "Delegates/DelegateCombinations.h"
#include "GeneralBattleGameMode.generated.h"

class AMyCharacterBase;
class APlayerCharacterBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameEndedDelegate, EGameEndResult, GameEndResult);

UCLASS(Blueprintable, BlueprintType)
class LEARNING2_API AGeneralBattleGameMode : public ABattleGameMode
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure)
    virtual FORCEINLINE EGameEndResult IsGameEnd() const 
    { 
        if (PlayerCharacters.IsEmpty()) { return EGameEndResult::Failed; }
        if (Enemies.IsEmpty()) { return EGameEndResult::Won; }

        return EGameEndResult::NotEnd;
    }

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnCharacterDead(AMyCharacterBase* Character); 
    virtual FORCEINLINE void OnCharacterDead_Internal(AMyCharacterBase* Character) {}

    UFUNCTION()
    void OnEndPlay(AActor* TeamMember, const EEndPlayReason::Type EndPlayReason);
    virtual FORCEINLINE void OnEndPlay_Internal(AActor* Actor, const EEndPlayReason::Type EndPlayReason) {}

    TArray<TObjectPtr<const AMyCharacterBase>>* GetContainer(const AMyCharacterBase* Character);
    const TArray<TObjectPtr<const AMyCharacterBase>>* GetContainer(const AMyCharacterBase* Character) const;

    virtual void OnGameEnded(const EGameEndResult GameEndResultResult);
    virtual FORCEINLINE void OnGameEnded_Internal(const EGameEndResult GameEndResultResult) {}
    UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Game Ended"))
    void K2_OnGameEnded(const EGameEndResult GameEndResultResult);

public:
    UPROPERTY(BlueprintAssignable)
    FOnGameEndedDelegate OnGameEndedDelegate;

private:
    TArray<TObjectPtr<const AMyCharacterBase>> Enemies;
    TArray<TObjectPtr<const AMyCharacterBase>> PlayerCharacters;

    TArray<TObjectPtr<const AMyCharacterBase>> DeadCharacters;
};