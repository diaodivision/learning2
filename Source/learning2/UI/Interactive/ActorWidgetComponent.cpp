// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorWidgetComponent.h"
//#include "Controller/MyPlayerController.h"
#include "ActorWidgetControllableInterface.h"
#include "ActorWidgetTypes.h"
#include "ActorWidget.h"

void UActorWidgetComponent::BeginPlay()
{
	Super::BeginPlay();

	UUserWidget* ViewInstance = GetWidget();
	AActor* OwnerActor = GetOwner();

	if (ViewInstance && OwnerActor && OwnerActor->Implements<UActorWidgetControllableInterface>())
	{
		IActorWidgetControllableInterface::Execute_OnUIInitialized(OwnerActor, ViewInstance);
	}
}

void UActorWidgetComponent::UpdateInteractionOptions(TArray<UInteractionOptionBase*> RawOptions)
{
	UActorWidget* ActorWidget{ Cast<UActorWidget>(GetWidget()) };
	if (!ActorWidget) { return; }

	//TArray<UInteractionOptionBase*> Options;
	//for (const UInteractionOptionBase* Option : RawOptions)
	//{
	//	if (!Option->IsValid()) { continue; }

	//	//TSoftObjectPtr<UTexture2D> FoundIcon = nullptr;

	//	//if (Option.AbilityInstance->Implements<UItemIconProviderInterface>())
	//	//{
	//	//	FoundIcon = IItemIconProviderInterface::Execute_GetItemIcon(Option.AbilityInstance.Get());
	//	//}
	//	//else if (Option.AbilityInstance->Implements<UAbilityIconProviderInterface>())
	//	//{
	//	//	FoundIcon = IAbilityIconProviderInterface::Execute_GetItemIcon(Option.AbilityInstance.Get(), ASC);
	//	//}

	//	Options.Add(Option);
	//}

	ActorWidget->UpdateInteractionOptions(RawOptions);
}