#pragma once

#include "CoreMinimal.h"
#include "Ability/MyGameplayAbilityBase.h"
#include "DoorDeploymentAbilityBase.generated.h"

struct IRecordedDataObjectInterface;

UCLASS(BlueprintType, Blueprintable, Abstract)
class UDoorDeploymentAbilityBase : public UMyGameplayAbilityBase
{
	GENERATED_BODY()

	virtual void Record() override;

	virtual bool TryHandleRecordedData(TSharedPtr<IRecordedDataObjectInterface, ESPMode::NotThreadSafe> InRecordedData) override;

	virtual void OnPreview(const bool bIsPreview, const IRecordedDataObjectInterface* InRecordedData) override;
};