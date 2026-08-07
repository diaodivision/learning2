// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractiveTraceComponent.h"
#include "Interactable\InteractableTargetInterface.h"
#include "Interactable\HighLightInterface.h"
#include "Controller/MyPlayerController.h"
#include "GameFramework/Actor.h"
#include "HoverReactive/HoverReactiveInterface.h"

// Sets default values for this component's properties
UInteractiveTraceComponent::UInteractiveTraceComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UInteractiveTraceComponent::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* ControllerBase = Cast<APlayerController>(GetOwner());
	checkf(ControllerBase, TEXT("Controller was destroy but its component name: { UInteractiveTraceComponent } still alive!!!"));

	if (AMyPlayerController* Controller = Cast<AMyPlayerController>(ControllerBase))
	{
		Controller->OnHoveredActorChangedDelegate.AddUObject(this, &UInteractiveTraceComponent::OnHoveredActorChanged);
		Controller->OnSelectedActorChangedDelegate.AddUObject(this, &UInteractiveTraceComponent::OnSelectedActorChanged);
	}
}

void UInteractiveTraceComponent::OnHoveredActorChanged(AActor* OldActor, AActor* NewActor)
{
	if (OldActor != NewActor)
	{
		HoveredStartSeconds = GetWorld()->GetTimeSeconds();
		HoveredActor = NewActor;

		if (OldActor)
		{
			if (OldActor->Implements<UHighLightInterface>()) { IHighLightInterface::Execute_UnhighLightActor(OldActor); }
			if (OldActor->Implements<UHoverReactiveInterface>()) { IHoverReactiveInterface::Execute_OnHoverReleased(OldActor); }
		}

		if (NewActor)
		{
			if (NewActor->Implements<UHighLightInterface>()) { IHighLightInterface::Execute_HighLightActor(NewActor); }
			if (NewActor->Implements<UHoverReactiveInterface>()) { IHoverReactiveInterface::Execute_OnHovered(NewActor, 0.f); }
		}
	}
	//else if()

	K2_OnHoveredActorChanged(OldActor, NewActor);
}

void UInteractiveTraceComponent::OnSelectedActorChanged(AActor* OldActor, AActor* NewActor)
{
	K2_OnSelectedActorChanged(OldActor, NewActor);
}

//void UInteractiveTraceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
//{
//	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
//
//	if (HoveredActor.IsValid() && HoveredActor->Implements<UHoverReactiveInterface>())
//	{
//		IHoverReactiveInterface::Execute_OnHovered(HoveredActor.Get(), GetWorld()->GetTimeSeconds() - HoveredStartSeconds);
//	}
//}

// Called every frame
void UInteractiveTraceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//UE_LOG(LogTemp, Warning, TEXT("UInteractiveTraceComponent::TickComponent"));

	if (HoveredActor.IsValid() && HoveredActor->Implements<UHoverReactiveInterface>() && GetWorld())
	{
		IHoverReactiveInterface::Execute_OnHovered(HoveredActor.Get(), GetWorld()->GetTimeSeconds() - HoveredStartSeconds);
	}

	APlayerController* Controller = Cast<APlayerController>(GetOwner());

	FHitResult HitResult;
	Controller->GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

	if (AActor* HitActor = HitResult.GetActor(); HoveredActor.Get() != HitActor)
	{
		//if (LastHoveredActor) { IHighLightInterface::Execute_UnhighLightActor(LastHoveredActor); }
		//if (HitActor) { IHighLightInterface::Execute_HighLightActor(HitActor); }
		OnHoveredActorChanged(HoveredActor.Get(), HitActor);
		HoveredActor = HitActor;


		////处理选项的显示和隐藏
		//if (HitActor->Implements<UInteractableTargetInterface>())
		//{
		//	FInteractionQuery InteractQuery;
		//	InteractQuery.RequestingAvatar = Controller->GetPawn();
		//	InteractQuery.RequestingController = Controller;

		//	if (!Controller->GetPawn()) { UE_LOG(LogTemp, Warning, TEXT("Failed to Controller->GetPawn()")); }

		//	IInteractableTargetInterface::Execute_ShowOptions(HitActor, InteractQuery);

		//	if (LastActor && HitActor != LastActor)
		//	{
		//		if (LastActor->Implements<UInteractableTargetInterface>())
		//		{
		//			IInteractableTargetInterface::Execute_HideOptions(LastActor);
		//		}
		//	}
		//}
	}

	////处理勾边的显示和隐藏
	//if (HitActor->Implements<UHighLightInterface>())
	//{
	//	IHighLightInterface::Execute_HighLightActor(HitActor);

	//	if (LastActor && HitActor != LastActor)
	//	{
	//		if (LastActor->Implements<UHighLightInterface>())
	//		{
	//			IHighLightInterface::Execute_UnhighLightActor(LastActor);
	//		}
	//	}
	//}


}

