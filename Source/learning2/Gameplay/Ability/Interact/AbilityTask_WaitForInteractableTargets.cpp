//#include "AbilityTask_WaitForInteractableTargets.h"
//#include "GameFramework/PlayerController.h"
//#include "Engine/World.h"
//#include "AbilitySystemComponent.h"
//
//UAbilityTask_WaitForInteractableTargets::UAbilityTask_WaitForInteractableTargets()
//{
//	bTickingTask = false;
//	bSimulatedTask = false;
//}
//
//UAbilityTask_WaitForInteractableTargets* UAbilityTask_WaitForInteractableTargets::WaitForInteractableTargets_SingleLineTrace(UGameplayAbility* OwningAbility, FName TaskInstanceName, float InScanRange, float InScanRate)
//{
//	UAbilityTask_WaitForInteractableTargets* MyTask = NewAbilityTask<UAbilityTask_WaitForInteractableTargets>(OwningAbility, TaskInstanceName);
//	MyTask->ScanRange = InScanRange;
//	MyTask->ScanRate = InScanRate;
//
//	return MyTask;
//}
//
//void UAbilityTask_WaitForInteractableTargets::Activate()
//{
//	UWorld* World = GetWorld();
//	if (!World || !Ability || !AbilitySystemComponent.IsValid())
//	{
//		EndTask();
//		return;
//	}
//
//	// 设置碰撞参数
//	CollisionQueryParams = FCollisionQueryParams::DefaultQueryParam;
//	CollisionQueryParams.bTraceComplex = false; // 使用简单碰撞
//	CollisionQueryParams.bReturnPhysicalMaterial = false;
//	CollisionQueryParams.AddIgnoredActor(GetAvatarActor());
//
//	// 立即执行一次扫描
//	PerformTrace();
//
//	// 设置定时器进行周期性扫描
//	World->GetTimerManager().SetTimer(TraceTimerHandle, this, &ThisClass::PerformTrace, ScanRate, true);
//}
//
//void UAbilityTask_WaitForInteractableTargets::OnDestroy(bool bInOwnerFinished)
//{
//	// 清理定时器
//	if (UWorld* World = GetWorld())
//	{
//		World->GetTimerManager().ClearTimer(TraceTimerHandle);
//	}
//
//	Super::OnDestroy(bInOwnerFinished);
//}
//
//void UAbilityTask_WaitForInteractableTargets::PerformTrace()
//{
//	if (!AbilitySystemComponent.IsValid()) { return; }
//
//	FVector TraceStart, TraceEnd;
//	GetTraceStartAndEnd(TraceStart, TraceEnd);
//
//	FHitResult HitResult;
//	LineTrace(HitResult, TraceStart, TraceEnd);
//
//	TArray<TScriptInterface<IInteractableTargetInterface>> InteractableTargets;
//
//	// 检查击中的Actor是否实现了IInteractableTarget接口
//	if (AActor* HitActor = HitResult.GetActor())
//	{
//		if (TScriptInterface<IInteractableTargetInterface> InteractableTarget = HitActor)
//		{
//			InteractableTargets.Add(InteractableTarget);
//		}
//	}
//
//	// 更新互动选项
//	UpdateInteractableOptions(InteractableTargets);
//}
//void UAbilityTask_WaitForInteractableTargets::UpdateInteractableOptions(const TArray<TScriptInterface<IInteractableTargetInterface>>& InteractableTargets)
//{
//	if (!AbilitySystemComponent.IsValid()) { return; }
//
//	TArray<FInteractionOption> NewOptions;
//
//	// 构建查询上下文
//	FInteractionQuery InteractQuery;
//	AActor* AvatarActor = GetAvatarActor();
//	InteractQuery.RequestingAvatar = AvatarActor;
//	if (APawn* Pawn = Cast<APawn>(AvatarActor))
//	{
//		InteractQuery.RequestingController = Pawn->GetController();
//	}
//
//	// 遍历所有击中的可互动对象
//	for (const TScriptInterface<IInteractableTargetInterface>& InteractableTarget : InteractableTargets)
//	{
//		TArray<FInteractionOption> TempOptions;
//		FInteractionOptionBuilder OptionBuilder(InteractableTarget, TempOptions);
//
//		// 调用接口方法获取选项
//		InteractableTarget->GatherInteractionOptions(InteractQuery);
//	}
//}
//
//void UAbilityTask_WaitForInteractableTargets::LineTrace(FHitResult& OutHitResult, const FVector& Start, const FVector& End) const
//{
//}
//
//void UAbilityTask_WaitForInteractableTargets::GetTraceStartAndEnd(FVector& OutStart, FVector& OutEnd) const
//{
//}
//
//
