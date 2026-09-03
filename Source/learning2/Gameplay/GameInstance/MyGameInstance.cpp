#include "MyGameInstance.h"
#include "UI/LoadingScreen/LoadingScreenWidget.h"
#include "MyGameInstanceSettings.h"
#include "ShaderPipelineCache.h"
#include "TimerManager.h"

void UMyGameInstance::Init()
{
	Super::Init();
	
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UMyGameInstance::PostLoadMapWithWorld);
}

void UMyGameInstance::PostLoadMapWithWorld(UWorld* InLoadedWorld)
{
    UWorld* World{ GetWorld() };
    if (!World) { return; }
    
    const UMyGameInstanceSettings* Settings = GetDefault<UMyGameInstanceSettings>();
    if (!Settings) { return; }
    const FSoftObjectPath CurrentWorldPath{ World };

    TSubclassOf<ULoadingScreenWidget> LoadingWidgetClass;
    if (Settings->NoCoveredLevelList.ContainsByPredicate([&CurrentWorldPath](const TSoftObjectPtr<UWorld>& Level) { return !Level.IsNull() && Level.ToSoftObjectPath() == CurrentWorldPath; }))
    {
        LoadingWidgetClass = !Settings->LoadingScreen.IsNull() ? Settings->LoadingScreen.LoadSynchronous() : nullptr;
    }
    else { LoadingWidgetClass = !Settings->LoadingScreen_Covered.IsNull() ? Settings->LoadingScreen_Covered.LoadSynchronous() : nullptr; }
	if (!LoadingWidgetClass) { return; }
    
    TotalPSOs = FShaderPipelineCache::NumPrecompilesRemaining();
    
    if (CachedLoadingWidget) { CancelPSOCheck(); }

    CachedLoadingWidget = CreateWidget<ULoadingScreenWidget>(World, LoadingWidgetClass);
    if (CachedLoadingWidget) { CachedLoadingWidget->AddToViewport(9999); }

    if (World->GetTimerManager().IsTimerActive(PSOCheckTimerHandle)) { World->GetTimerManager().ClearTimer(PSOCheckTimerHandle); }
    World->GetTimerManager().SetTimer(PSOCheckTimerHandle, FTimerDelegate::CreateUObject(this, &UMyGameInstance::CheckPSOPrecacheCompletion), 0.1f, true);
    
    if (World->GetTimerManager().IsTimerActive(CancelPSOCheckTimerHandle)) { World->GetTimerManager().ClearTimer(CancelPSOCheckTimerHandle); }
    World->GetTimerManager().SetTimer(CancelPSOCheckTimerHandle, FTimerDelegate::CreateUObject(this, &UMyGameInstance::CancelPSOCheck), 1.f, false);

    FShaderPipelineCache::SetBatchMode(FShaderPipelineCache::BatchMode::Fast);

    CheckPSOPrecacheCompletion();
}

void UMyGameInstance::CheckPSOPrecacheCompletion()
{
	// 检查当前剩余的 PSO / Pipeline 编译数量
	const uint32 RemainingPSOs{ FShaderPipelineCache::NumPrecompilesRemaining() };

    if (RemainingPSOs > TotalPSOs)
    {
        // 记录总的 PSO 数量
        TotalPSOs = RemainingPSOs;
    }

    if (CachedLoadingWidget && TotalPSOs > 0)
    {
        CachedLoadingWidget->UpdateLoadingInfo(FLoadingInfo{TotalPSOs - RemainingPSOs, TotalPSOs});
    }

	// 当所有的 PSO 都编译完毕
	if (RemainingPSOs == 0)
	{
        CancelPSOCheck();

        if (UWorld* World{ GetWorld() }; World&& World->GetTimerManager().IsTimerActive(PSOCheckTimerHandle)) 
        { 
            World->GetTimerManager().ClearTimer(PSOCheckTimerHandle);
        }
	}
}

void UMyGameInstance::CancelPSOCheck()
{
    if (FShaderPipelineCache::NumPrecompilesRemaining() > 0) { return; }

    UWorld* World{ GetWorld() };
    if (!World) { return; }
    
    if (World->GetTimerManager().IsTimerActive(CancelPSOCheckTimerHandle)) { World->GetTimerManager().ClearTimer(CancelPSOCheckTimerHandle); }

    if (CachedLoadingWidget)
    {
        CachedLoadingWidget->RemoveFromParent();
        CachedLoadingWidget->RemoveFromRoot();
        CachedLoadingWidget = nullptr;
    }

    FShaderPipelineCache::SetBatchMode(FShaderPipelineCache::BatchMode::Background);
    TotalPSOs = 0;
}