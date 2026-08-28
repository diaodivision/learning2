#pragma once

#include "CoreMinimal.h"
#include "Ability/MyGameplayAbilityBase.h"
#include "DoorDeploymentAbilityBase.generated.h"

struct IRecordedDataObjectInterface;
class AWeaponActorBase;

UCLASS(BlueprintType, Blueprintable, Abstract)
class UDoorDeploymentAbilityBase : public UMyGameplayAbilityBase
{
	GENERATED_BODY()

	virtual void Record(const FGameplayEventData* TriggerEventData) override;

	virtual bool TryHandleRecordedData(TSharedPtr<IRecordedDataObjectInterface, ESPMode::NotThreadSafe> InRecordedData) override;

	virtual void OnPreview(const bool bIsPreview, const IRecordedDataObjectInterface* InRecordedData) override;

	virtual void PreActivateInteractiveOption() override;

	virtual void PostExecuteBindAbility() override;

private:
	TWeakObjectPtr<AWeaponActorBase> LastControlWeapon;
};