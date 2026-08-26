#include "FogOfWarProxyActor.h"
#include "FogOfWarComponent.h"
#include "Components/SceneComponent.h"

AFogOfWarProxyActor::AFogOfWarProxyActor()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	FogOfWarComponent = CreateDefaultSubobject<UFogOfWarComponent>(TEXT("FogOfWarComponent"));
}