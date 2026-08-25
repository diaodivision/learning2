// Fill out your copyright notice in the Description page of Project Settings.


#include "MyNavModifierComponent.h"
#include "Battle/Interface/NavModifiedActorInterface.h"
#include "AI/Navigation/NavigationRelevantData.h"

UMyNavModifierComponent::UMyNavModifierComponent() : Super()
{
	bAutoActivate = true;
}

void UMyNavModifierComponent::GetNavigationData(FNavigationRelevantData& Data) const
{
	if (INavModifiedActorInterface* NavModifiedActorInterface = Cast<INavModifiedActorInterface>(GetOwner()))
	{
		FAreaNavModifier NavModifier(NavModifiedActorInterface->GetNavModifiedBounds().GetExtent(), GetOwner()->GetActorTransform(), AreaClass);
		if (AreaClassToReplace)
		{
			NavModifier.SetAreaClassToReplace(AreaClassToReplace);
		}
		Data.Modifiers.Add(NavModifier);

		Data.Modifiers.SetNavMeshResolution(NavMeshResolution);
	}
	else
	{
		Super::GetNavigationData(Data);
	}
}