// Fill out your copyright notice in the Description page of Project Settings.


#include "MyHUD.h"
#include "MyUserWidget.h"
#include "WidgetController.h"
#include "Controller/MyPlayerController.h"

void AMyHUD::SetUpHUD(FPossessedCharacterWidgetControllerContext InContext)
{
	if (!InContext.IsValid()) { return; }

	if (!WidgetController)
	{
		check(WidgetControllerClass);
		WidgetController = NewObject<UWidgetController>(this, WidgetControllerClass);
	}

	if (!Widget)
	{
		check(WidgetClass);
		Widget = CreateWidget<UMyUserWidget>(GetWorld(), WidgetClass);
	}

	if (WidgetController)
	{
		InContext.PlayerController->OnPossessedPawnChanged.AddUniqueDynamic(this, &AMyHUD::OnPossessedPawnChanged);

		WidgetController->SetUpWidgetController(InContext);

		if (Widget)
		{
			Widget->SetWidgetController(WidgetController);
			WidgetController->BroadcastInitialData();

			if (!Widget->IsInViewport()) { Widget->AddToViewport(); }
		}

		Context = InContext;
	}
}

void AMyHUD::BeginDestroy()
{
	Super::BeginDestroy();

	if (Context.PlayerController.IsValid()) { Context.PlayerController->OnPossessedPawnChanged.RemoveAll(this); }
}

void AMyHUD::OnPossessedPawnChanged(APawn* InOldPawn, APawn* InNewPawn)
{
	if (!Context.IsValid()) { return; }

	SetUpHUD({ Cast<APlayerCharacterBase>(InNewPawn) });
}