#include "FogOfWarProxyActor.h"
#include "FogOfWarComponent.h"
#include "Components/SceneComponent.h"
#include "FogOfWarProxyWidgetComponent.h"

AFogOfWarProxyActor::AFogOfWarProxyActor() : Super()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	FogOfWarComponent = CreateDefaultSubobject<UFogOfWarComponent>(TEXT("FogOfWarComponent"));
	FogOfWarProxyWidgetComponent = CreateDefaultSubobject<UFogOfWarProxyWidgetComponent>(TEXT("FogOfWarProxyWidgetComponent"));
	if (FogOfWarProxyWidgetComponent) { FogOfWarProxyWidgetComponent->SetupAttachment(RootComponent); }
}

void AFogOfWarProxyActor::SetActorHiddenInGame(bool bNewHidden)
{
	Super::SetActorHiddenInGame(bNewHidden);

	if (bNewHidden) { FogOfWarComponent->Deactivate(); }
	else { FogOfWarComponent->Activate(); }

	if (FogOfWarProxyWidgetComponent) { FogOfWarProxyWidgetComponent->ShowWidget(!bNewHidden); }
	OnFogOfWarProxyActorVisibilityChangedDelegate.Broadcast(!bNewHidden);
}

void AFogOfWarProxyActor::BeginPlay()
{
	Super::BeginPlay();

	SetActorEnableCollision(false);
	FogOfWarProxyWidgetComponent->OnWidgetVisibilityChangedDelegate.AddWeakLambda(this, [this](const ESlateVisibility InVisibility)
	{
		OnFogOfWarProxyActorVisibilityChangedDelegate.Broadcast(InVisibility == ESlateVisibility::Visible);
	});
}