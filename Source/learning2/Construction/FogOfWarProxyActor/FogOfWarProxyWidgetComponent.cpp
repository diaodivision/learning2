// Fill out your copyright notice in the Description page of Project Settings.


#include "FogOfWarProxyWidgetComponent.h"
#include "FogOfWarProxyWidget.h"
#include "FogOfWarProxyActor.h"

UFogOfWarProxyWidgetComponent::UFogOfWarProxyWidgetComponent() : Super()
{
	bAutoActivate = true;
}

void UFogOfWarProxyWidgetComponent::ShowWidget(const bool bIsShow)
{
    if (UFogOfWarProxyWidget* ViewInstance{ Cast<UFogOfWarProxyWidget>(GetWidget()) }) 
    { 
        ViewInstance->ShowWidget(bIsShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden, Cast<AFogOfWarProxyActor>(GetOwner()));
    }
}

void UFogOfWarProxyWidgetComponent::BeginPlay()
{
    Super::BeginPlay();

    if (GetWidget()) 
    { 
        GetWidget()->OnNativeVisibilityChanged.AddUObject(this, &UFogOfWarProxyWidgetComponent::OnWidgetVisibilityChanged);
    }
}

void UFogOfWarProxyWidgetComponent::OnWidgetVisibilityChanged(const ESlateVisibility InVisibility)
{
    OnWidgetVisibilityChangedDelegate.Broadcast(InVisibility);
}