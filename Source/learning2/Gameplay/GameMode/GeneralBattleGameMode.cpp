#include "GeneralBattleGameMode.h"
#include "EngineUtils.h"
#include "Character/PlayerCharacterBase.h"

void AGeneralBattleGameMode::BeginPlay()
{
    Super::BeginPlay();

    const UWorld* World{ GetWorld() };
    if (!World) return;

    for (TActorIterator<AActor> It{ World }; It; ++It)
    {
        AMyCharacterBase* Character{ Cast<AMyCharacterBase>(*It) };
        if (!Character) { continue; }
        if (Character->IsDead()) { DeadCharacters.Add(Character); }

        if (TArray<TObjectPtr<const AMyCharacterBase>>* Container{ GetContainer(Character) })
        {
            Container->Add(Character);
            
            Character->OnCharacterDeadDelegate.AddUniqueDynamic(this, &AGeneralBattleGameMode::OnCharacterDead);
            Character->OnEndPlay.AddUniqueDynamic(this, &AGeneralBattleGameMode::OnEndPlay);
        }
    }
}

void AGeneralBattleGameMode::OnCharacterDead(AMyCharacterBase* Character)
{
    if (TArray<TObjectPtr<const AMyCharacterBase>>* Container{ GetContainer(Character) })
    {
        if (Container->Contains(Character)) 
        { 
            Container->Remove(Character);
            DeadCharacters.Add(Character);
            OnCharacterDead_Internal(Character);

            const EGameEndResult Result{ IsGameEnd() };
            if (Result == EGameEndResult::Failed || Result == EGameEndResult::Won) { OnGameEnded(Result); }
        }
    }
}

void AGeneralBattleGameMode::OnGameEnded(const EGameEndResult Result)
{
    if (APlayerController* Controller{ GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr }) { Controller->DisableInput(nullptr); }

    OnGameEnded_Internal(Result);
    K2_OnGameEnded(Result);
    OnGameEndedDelegate.Broadcast(Result);
}

void AGeneralBattleGameMode::OnEndPlay(AActor* Actor, const EEndPlayReason::Type EndPlayReason)
{
    const AMyCharacterBase* Character{ Cast<AMyCharacterBase>(Actor) };
    if (!Character) { return; }

    if (TArray<TObjectPtr<const AMyCharacterBase>>* Container{ GetContainer(Character) })
    {
        if (Container->Contains(Character)) { Container->Remove(Character); }
        else if (DeadCharacters.Contains(Character)) { DeadCharacters.Remove(Character); }
        else { return; }

        OnEndPlay_Internal(Actor, EndPlayReason);
    }
}

TArray<TObjectPtr<const AMyCharacterBase>>* AGeneralBattleGameMode::GetContainer(const AMyCharacterBase* Character)
{
    return const_cast<TArray<TObjectPtr<const AMyCharacterBase>>*>(const_cast<const AGeneralBattleGameMode*>(this)->GetContainer(Character));
}

const TArray<TObjectPtr<const AMyCharacterBase>>* AGeneralBattleGameMode::GetContainer(const AMyCharacterBase* Character) const
{
    if (!Character) { return nullptr; }
    
    if (Character->IsA<APlayerCharacterBase>()) { return &PlayerCharacters; }
    else { return &Enemies; }
}