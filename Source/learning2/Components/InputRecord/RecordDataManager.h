#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "RecordDataManager.generated.h"

// 2. 再定义 UMyRecordData
UCLASS(BlueprintType, Blueprintable)
class UMyRecordData : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite)
	const UInputAction* InputAction = nullptr;

	UPROPERTY(BlueprintReadWrite)
	int32 Tick = 0;

	bool operator<(const UMyRecordData& Other) const
	{
		return Tick < Other.Tick;
	}
};

// 委托声明
DECLARE_DYNAMIC_DELEGATE_OneParam(FShowWillExecutingOperation, bool, bWillExecute);
DECLARE_DYNAMIC_DELEGATE_OneParam(FExecuteOperation, UMyRecordData*, Data);

// 1. 先定义 FMyRecordBindData（不依赖于其他结构）
USTRUCT(BlueprintType)
struct FMyRecordBindData
{
	GENERATED_BODY()

	FMyRecordBindData() : InputAction(nullptr) {}
	FMyRecordBindData(const UInputAction* InAction, FShowWillExecutingOperation InShowOp, FExecuteOperation InExecOp)
		: InputAction(InAction)
		, ShowWillExecutingOperation(InShowOp)
		, ExecuteOperation(InExecOp)
	{
	}

	UPROPERTY(BlueprintReadWrite)
	const UInputAction* InputAction;

	UPROPERTY(BlueprintReadWrite)
	FShowWillExecutingOperation ShowWillExecutingOperation;

	UPROPERTY(BlueprintReadWrite)
	FExecuteOperation ExecuteOperation;

	bool operator==(const FMyRecordBindData& Other) const
	{
		return InputAction == Other.InputAction;
	}
};

// 类型特征特化放在最后
template<>
struct TStructOpsTypeTraits<FMyRecordBindData> : public TStructOpsTypeTraitsBase2<FMyRecordBindData>
{
	enum
	{
		WithIdenticalViaEquality = true,
	};
};