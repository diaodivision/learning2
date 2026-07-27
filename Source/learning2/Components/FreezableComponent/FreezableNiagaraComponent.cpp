// Fill out your copyright notice in the Description page of Project Settings.


#include "FreezableNiagaraComponent.h"
#include "WorldPauseSubsystem.h"

void UFreezableNiagaraComponent::Freeze_Implementation()
{
	SetCustomTimeDilation(0.f);
	bIsFreezing = true;
}

void UFreezableNiagaraComponent::Unfreeze_Implementation()
{
	SetCustomTimeDilation(1.f);
	bIsFreezing = false;
}

void UFreezableNiagaraComponent::OnRegister()
{
	Super::OnRegister();

	UWorldPauseSubsystem* WorldPauseSubsystem{ GetWorld()->GetSubsystem<UWorldPauseSubsystem>() };
	if (!WorldPauseSubsystem) { return; }

	WorldPauseSubsystem->OnFreezableObjectRegistered(this);
}

void UFreezableNiagaraComponent::OnUnregister()
{
	Super::OnUnregister();

	UWorldPauseSubsystem* WorldPauseSubsystem{ GetWorld()->GetSubsystem<UWorldPauseSubsystem>() };
	if (!WorldPauseSubsystem) { return; }

	WorldPauseSubsystem->OnFreezableObjectUnregistered(this);
}