#include "TopDownCameraSystemStatics.h"
#include "TopDownCameraSubsystem.h"

UTopDownCameraSubsystem* UTopDownCameraSystemStatics::GetTopDownCameraSubsystem(const UObject* WorldContextObject)
{
    if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		return World->GetSubsystem<UTopDownCameraSubsystem>();
	}
	return nullptr;
}