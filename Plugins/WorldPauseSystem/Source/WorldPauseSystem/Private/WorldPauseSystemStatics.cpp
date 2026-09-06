#include "WorldPauseSystemStatics.h"
#include "WorldPauseSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

bool UWorldPauseSystemStatics::IsWorldFreezing(const UObject* WorldContextObject)
{
    return GetWorldPauseSubsystem(WorldContextObject) ? GetWorldPauseSubsystem(WorldContextObject)->IsFreezing() : false;
}

UWorldPauseSubsystem* UWorldPauseSystemStatics::GetWorldPauseSubsystem(const UObject* WorldContextObject)
{
    if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		return World->GetSubsystem<UWorldPauseSubsystem>();
	}
    return nullptr;
}