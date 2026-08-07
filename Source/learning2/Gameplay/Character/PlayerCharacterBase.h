#pragma once

#include "Base/MyCharacterBase.h"
#include "Targeting/TargetingInstigatorInterface.h"
#include "Targeting/TargetingInstigatorTypes.h"
#include "PlayerCharacterBase.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UFogOfWarComponent;
class UInputRecordComponent;
struct FOnAttributeChangeData;
struct IRecordedDataObjectInterface;
class URecordedLocationVisualizationComponent;

UCLASS(Blueprintable, Blueprinttype)
class LEARNING2_API APlayerCharacterBase : public AMyCharacterBase/*, public ITargetingInstigatorInterface*/
{
	GENERATED_BODY()

	APlayerCharacterBase();

	virtual void PossessedBy(AController* NewController) override;

public:
	inline UInputRecordComponent* GetInputRecordComponent() const { return InputRecordComponent; }

	virtual void UpdateCharacterWidget() override;

	virtual void SetTargetingState(ETargetingState TargetingState);

	//virtual FORCEINLINE FOnTargetingStateChangedDelegate& GetOnTargetingStateChangedDelegate() override { return OnTargetingStateChangedDelegate; }

protected:
	virtual void InitializeDelegates() override;
	virtual void DeinitializeDelegates() override;

	virtual void OnCharacterHealthChanged(const FOnAttributeChangeData& OnAttributeChangeData);

	virtual void OnInputRecordOperationPreview(const bool bIsPreview, const IRecordedDataObjectInterface* Data);

private:
	void CreateAndSetupComponents();

public:
	FOnOperationPreviewDelegate OnOperationPreviewDelegate;

	//FOnTargetingStateChangedDelegate OnTargetingStateChangedDelegate;

protected:
	//UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components")
	//TObjectPtr<USpringArmComponent> SpringArmComponent;

	//UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components")
	//TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UFogOfWarComponent> FogOfWarComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UInputRecordComponent> InputRecordComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<URecordedLocationVisualizationComponent> RecordedLocationVisualizationComponent;
};