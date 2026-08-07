// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraBoundsVolume.h"

ACameraBoundsVolume::ACameraBoundsVolume()
{
	if (UPrimitiveComponent * PrimitiveComponent{ Cast<UPrimitiveComponent>(GetRootComponent()) })
	{
		PrimitiveComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
		PrimitiveComponent->SetCollisionObjectType(ECC_WorldStatic);
		PrimitiveComponent->SetNotifyRigidBodyCollision(true);
		PrimitiveComponent->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	}
}