// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "Interface/FreezableInterface.h"
#include "FreezableNiagaraComponent.generated.h"

/**
 *
 */
UCLASS()
class LEARNING2_API UFreezableNiagaraComponent : public UNiagaraComponent, public IFreezableInterface
{
	GENERATED_BODY()

public:
	UFreezableNiagaraComponent();

	virtual void Freeze_Implementation() override;
	virtual void Unfreeze_Implementation() override;
	virtual FORCEINLINE bool IsFreezing_Implementation() override { return bIsFreezing; };

protected:
	virtual void OnRegister() override;
	virtual void OnUnregister() override;

private:
	bool bIsFreezing{ false };
};
