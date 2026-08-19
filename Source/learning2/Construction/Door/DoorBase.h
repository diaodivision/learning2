#pragma once

#include "CoreMinimal.h"
#include "InteractableActor/Base/InteractableActorBase.h"
#include "UObject/ObjectPtr.h"
#include "WorldHeightEffectiveActorInterface.h"
#include "Battle/Interface/NavModifiedActorInterface.h"
#include "AITypes.h"
#include "Delegates/DelegateCombinations.h"
#include "PredictionLineProvider/PredictionLineProviderInterface.h"

#include "DoorBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDoorOpenedDelegate);

class UNavLinkCustomComponent;
class AGrenadeTargetActor;
class AGrenadeBulletBase;
class UMeshComponent;

UCLASS(BlueprintType, Blueprintable, Abstract)
class ADoorBase :
	public AInteractableActorBase,
	public IWorldHeightEffectiveActorInterface,
	public INavModifiedActorInterface,
	public IPredictionLineProviderInterface
{
	GENERATED_BODY()

	ADoorBase();

public:
	virtual void BeginPlay() override;

	virtual FOrientedBox GetBounds_Implementation() const override;

	virtual void NotifySmartLinkReached(UNavLinkCustomComponent* LinkComp, UObject* PathingAgent, const FVector& DestPoint);

	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveSmartLinkReached(AActor* Agent, const FVector& InDestination);

	UFUNCTION(BlueprintCallable, Category = "AI|Navigation")
	void ResumePathFollowing(AActor* Agent);

	UFUNCTION(BlueprintCallable)
	virtual void ActivateModify() override;

	UFUNCTION(BlueprintCallable)
	virtual void DeactivateModify() override;

	virtual FBox GetNavModifiedBounds() override;

	UFUNCTION(BlueprintPure)
	inline bool IsDoorOpened() const { return bIsDoorOpened; }

	virtual void ShowPredictionLine(const FPredictionLineParams& Params) override;
	void ShowPredictionLineStatic(const FPredictionLineParams& Params, const FVector& TargetLocation);
	virtual void HidePredictionLine() override;
	virtual bool IsPredictionLineVisible() const override;

	virtual void NotifyOptionActivate()
	{
		//todo
	}

protected:
	UFUNCTION(BlueprintCallable)
	void NotifyDoorOpened();

	void CreateGrenadeTargetActor();

	UFUNCTION(BlueprintImplementableEvent)
	UMeshComponent* GetDoorMesh() const;

	UFUNCTION(BlueprintImplementableEvent)
	UMeshComponent* GetDoorFrameMesh() const;
	
	// UFUNCTION()
	virtual void OnDoorRotated(USceneComponent* SceneComponent, EUpdateTransformFlags Flags, ETeleportType TeleportType);

public:
	UPROPERTY(BlueprintAssignable)
	FOnDoorOpenedDelegate OnDoorOpenedDelegate;

private:
	UPROPERTY(VisibleAnywhere, Category = SmartLink, meta = (AllowPrivateAccess = true))
	TObjectPtr<UNavLinkCustomComponent> NavLinkCustomComponent;

	//FAIRequestID CacheAgentAIRequestID;
	TOptional<FVector> CachedDestination;

	bool bIsTriggerOpenDoor{ false };
	bool bIsDoorOpened{ false };

	UPROPERTY(BlueprintReadOnly, Category = "Attribute|PredictionLine", meta = (AllowPrivateAccess = true))
	TObjectPtr<AGrenadeTargetActor> GrenadeTargetActor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute|PredictionLine|Mesh", meta = (AllowPrivateAccess = true))
	TSoftObjectPtr<UStaticMesh> PredictionLineMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute|PredictionLine", meta = (AllowPrivateAccess = true))
	int32 IterationNum{ 1 };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute|PredictionLine", meta = (AllowPrivateAccess = true))
	TSubclassOf<AGrenadeTargetActor> GrenadeTargetActorClass;

	FRotator DoorLastRotation{ FRotator::ZeroRotator };
};