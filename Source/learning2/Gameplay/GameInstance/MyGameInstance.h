#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MyGameInstance.generated.h"

class ULoadingScreenWidget;

UCLASS()
class LEARNING2_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

protected:
	virtual void PostLoadMapWithWorld(UWorld* InLoadedWorld);
	
	// 轮询检查 PSO 编译状态
	void CheckPSOPrecacheCompletion();

    void CancelPSOCheck();

private:
	UPROPERTY()
	TObjectPtr<ULoadingScreenWidget> CachedLoadingWidget;

	FTimerHandle PSOCheckTimerHandle;
    FTimerHandle CancelPSOCheckTimerHandle;

    uint32 TotalPSOs{ 0 };
};