#include "BattleSubsystemStatics.h"
#include "BattleSubsystem.h"

UBattleSubsystem* UBattleSubsystemStatics::GetBattleSubsystem(const UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		return World->GetSubsystem<UBattleSubsystem>();
	}
    return nullptr;
}