#include "FogOfWarProxyActor.h"
#include "FogOfWarComponent.h"
#include "Components/SceneComponent.h"

AFogOfWarProxyActor::AFogOfWarProxyActor() : Super()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	FogOfWarComponent = CreateDefaultSubobject<UFogOfWarComponent>(TEXT("FogOfWarComponent"));
}

void AFogOfWarProxyActor::SetActorHiddenInGame(bool bNewHidden)
{
	Super::SetActorHiddenInGame(bNewHidden);

	if (bNewHidden) { FogOfWarComponent->Deactivate(); }
	else { FogOfWarComponent->Activate(); }
}

void AFogOfWarProxyActor::BeginPlay()
{
	Super::BeginPlay();

	SetActorEnableCollision(false);
}