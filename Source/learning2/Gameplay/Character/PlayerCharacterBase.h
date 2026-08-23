#pragma once

#include "Base/MyCharacterBase.h"
#include "Targeting/TargetingInstigatorInterface.h"
#include "Targeting/TargetingInstigatorTypes.h"
#include "Perception/AISightTargetInterface.h"
#include "PlayerCharacterBase.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UFogOfWarComponent;
class UInputRecordComponent;
struct FOnAttributeChangeData;
struct IRecordedDataObjectInterface;
class URecordedLocationVisualizationComponent;

UCLASS(Blueprintable, Blueprinttype)
class LEARNING2_API APlayerCharacterBase : public AMyCharacterBase/*, public ITargetingInstigatorInterface*/, public IAISightTargetInterface
{
	GENERATED_BODY()

public:
	APlayerCharacterBase();
	inline UInputRecordComponent* GetInputRecordComponent() const { return InputRecordComponent; }

	virtual void UpdateCharacterWidget() override;

	virtual void SetTargetingState(ETargetingState TargetingState);

	//virtual FORCEINLINE FOnTargetingStateChangedDelegate& GetOnTargetingStateChangedDelegate() override { return OnTargetingStateChangedDelegate; }

	virtual UAISense_Sight::EVisibilityResult CanBeSeenFrom(const FCanBeSeenFromContext& Context, FVector& OutSeenLocation, int32& OutNumberOfLoSChecksPerformed, int32& OutNumberOfAsyncLosCheckRequested, float& OutSightStrength, int32* UserData, const FOnPendingVisibilityQueryProcessedDelegate* Delegate) override;

	FORCEINLINE static bool IsTraceConsideredVisible(const FHitResult* HitResult, const AActor* TargetActor)
	{
		if (HitResult == nullptr)
		{
			return true;
		}
		const AActor* HitResultActor = HitResult->HitObjectHandle.FetchActor();
		return (HitResultActor ? HitResultActor->IsOwnedBy(TargetActor) : false);
	}

protected:
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;

	virtual void InitializeDelegates() override;
	virtual void DeinitializeDelegates() override;

	virtual void OnCharacterHealthChanged(const FOnAttributeChangeData& OnAttributeChangeData);

	virtual void OnInputRecordOperationPreview(const bool bIsPreview, const IRecordedDataObjectInterface* Data);

	virtual void CancelRewindingState();

	UFUNCTION()
	virtual void OnRewindSubsystemStateChanged(const ERecordState OldState, const ERecordState NewState);

	virtual FORCEINLINE void OnStunTagCountChanged(const ETagCountChangeType TagCountChangeType) override {}
	virtual FORCEINLINE void OnBlindTagCountChanged(const ETagCountChangeType TagCountChangeType) override {}

private:
	void CreateAndSetupComponents();

	virtual void OnCharacterDeath_Internal() override;

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