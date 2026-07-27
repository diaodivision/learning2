#pragma once

#include "CoreMinimal.h"
#include "InteractionQuery.generated.h"

class IInteractableTarget;

/**
 * 互动查询上下文
 * 包含谁在请求互动等信息
 */
USTRUCT(BlueprintType)
struct FInteractionQuery
{
	GENERATED_BODY()

	inline bool IsValid() const { return RequestingAvatar.IsValid() /*&& RequestingController.IsValid()*/; }

	/** 发起互动的Pawn（通常是玩家角色） */
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AActor> RequestingAvatar = nullptr;

	/** 发起互动的Controller */
	UE_DEPRECATED(5.7, "Unnecessary Member")
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AController> RequestingController = nullptr;
};