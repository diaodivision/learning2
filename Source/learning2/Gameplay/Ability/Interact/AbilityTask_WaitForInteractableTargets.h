//#pragma once
//
//#include "CoreMinimal.h"
//#include "Abilities/Tasks/AbilityTask.h"
//#include "Interactable/InteractionQuery.h"
//#include "Interactable/InteractableTargetInterface.h"
//#include "Interactable/InteractionOptionBuilder.h"
//#include "AbilityTask_WaitForInteractableTargets.generated.h"
//
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractableObjectsChangedEvent, const TArray<FInteractionOption>&, InteractableOptions);
//
//UCLASS(BlueprintType, Blueprintable)
//class UAbilityTask_WaitForInteractableTargets : public UAbilityTask
//{
//	GENERATED_BODY()
//
//public:
//	UAbilityTask_WaitForInteractableTargets();
//
//	UPROPERTY(BlueprintAssignable)
//	FInteractableObjectsChangedEvent InteractableObjectsChangedEvent;
//
//	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = true))
//	static UAbilityTask_WaitForInteractableTargets* WaitForInteractableTargets_SingleLineTrace(UGameplayAbility* OwningAbility, FName TaskInstanceName, float InScanRange = 500.0f, float InScanRate = 0.1f);
//
//protected:
//	virtual void Activate() override;
//	virtual void OnDestroy(bool bInOwnerFinished) override;
//
//	void PerformTrace();
//
//	void UpdateInteractableOptions(const TArray<TScriptInterface<IInteractableTargetInterface>>& InteractableTargets);
//
//	void LineTrace(FHitResult& OutHitResult, const FVector& Start, const FVector& End) const;
//
//	void GetTraceStartAndEnd(FVector& OutStart, FVector& OutEnd) const;
//
//protected:
//	/** 扫描范围 */
//	float ScanRange{ 500.0f };
//
//	/** 扫描频率（秒） */
//	float ScanRate{ 0.1f };
//
//	/** 定时器句柄 */
//	FTimerHandle TraceTimerHandle;
//
//	/** 当前的互动选项 */
//	TArray<FInteractionOption> CurrentOptions;
//
//	/** 碰撞查询参数 */
//	FCollisionQueryParams CollisionQueryParams;
//};