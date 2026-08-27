#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Delegates/DelegateCombinations.h"
#include "InteractionOption.generated.h"

class UTexture2D;
class UGameplayAbility;
class ACharacter;

namespace InteractionOptionTypes
{
	using OptionGroupIDType = int32;
}

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnInteractiveOptionStateChangedDelegate, const bool, bIsActivating);

//class FInteractionOptionToken
//{
//	friend struct FInteractionOption;
//
//	TWeakObjectPtr<UMyGameplayAbilityBase> Token;
//
//public:
//	explicit FInteractionOptionToken(UMyGameplayAbilityBase& GA) : Token(&GA)
//	{
//		if (GA.HasAnyFlags(RF_ClassDefaultObject))
//		{
//			Token = nullptr;
//		}
//	}
//
//	~FInteractionOptionToken()
//	{
//		if (UMyGameplayAbilityBase* GA = Token.Get())
//		{
//			GA->Unbind();
//			Token.Reset();
//		}
//	}
//};

UCLASS(BlueprintType, Abstract)
class UInteractionOptionBase : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE bool Activate() { return false; }

	UFUNCTION(BlueprintCallable)
	virtual FORCEINLINE void SetWillBeActivate(const bool bWillActivate)
	{
		const bool OldState{ bWillBeActivate };
		bWillBeActivate = bWillActivate;

		if (OldState != bWillBeActivate) { OnInteractiveOptionStateChangedDelegate.ExecuteIfBound(bWillBeActivate); }
	}

	UFUNCTION(BlueprintPure)
	virtual FORCEINLINE bool IsActivating() const { return bWillBeActivate; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetGroupID() const { return GroupID; }

	UFUNCTION(BlueprintPure)
	virtual TSoftObjectPtr<UTexture2D> GetIcon() const { return nullptr; }

	UFUNCTION(BlueprintPure)
	virtual FORCEINLINE bool IsValid() const { return bIsAlive; }

	virtual FORCEINLINE bool CanDestroy() { return true; }

	virtual FORCEINLINE void Destroy()
	{
		if (!bIsAlive) { return; }

		bWillBeActivate = false;
		bIsAlive = false;

		if (!HasAnyFlags(RF_BeginDestroyed)) { MarkAsGarbage(); }
	}

	virtual FORCEINLINE void BeginDestroy() override
	{
		Super::BeginDestroy();
	}
	//virtual FORCEINLINE void Destroy()
	//{
	//	if (!bIsAlive) { return; }

	//	bWillBeActivate = false;
	//	bIsAlive = false;

	//	if (!HasAnyFlags(RF_BeginDestroyed)) { MarkAsGarbage(); }
	//}

	//virtual FORCEINLINE void BeginDestroy() override
	//{
	//	Super::BeginDestroy();
	//}

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Equal (InteractionOptionBase)", CompactNodeTitle = "==", Keywords = "== equal"))
	static FORCEINLINE bool EqualEqual_InteractionOptionBaseInteractionOptionBase(const UInteractionOptionBase* A, const UInteractionOptionBase* B)
	{
		return A == B;
	}

protected:
	FORCEINLINE bool IsAlive() const { return bIsAlive; }

public:
	UPROPERTY(BlueprintReadWrite, Category = "Delegate")
	FOnInteractiveOptionStateChangedDelegate OnInteractiveOptionStateChangedDelegate;

	TWeakObjectPtr<ACharacter> Instigator;

protected:
	int32 GroupID{ INDEX_NONE };
	static_assert(std::is_same_v<InteractionOptionTypes::OptionGroupIDType, decltype(GroupID)>);

private:
	bool bWillBeActivate{ false };
	bool bIsAlive{ true };
};

USTRUCT(BlueprintType)
struct FOptionInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<UGameplayAbility> AbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 GroupID{ INDEX_NONE };

	FORCEINLINE bool IsValid() const { return operator bool(); }

	bool operator==(const FOptionInfo& Other) const;

	operator bool() const;

	friend uint32 GetTypeHash(const FOptionInfo& Request)
	{
		return GetTypeHash(Request.AbilityClass);
	}

	static_assert(std::is_same_v<InteractionOptionTypes::OptionGroupIDType, decltype(GroupID)>);
};

//USTRUCT(BlueprintType)
//struct FInteractionOption
//{
//	GENERATED_BODY()
//
//	friend struct FInteractionOptionsBuilder;
//
//	FInteractionOption()
//	{
////#if !WITH_EDITOR
////		ensureMsgf(false, TEXT("Default ctor should not be used directly"));
////#endif
//	}
//
//private:
//	explicit FInteractionOption(const FCombinedAbilityHandle& Handle) :
//		TargetAbilitySystem(Handle.AbilitySystemComponent),
//		TargetInteractionAbilityHandle(Handle.Handle)
//	{
//		if (TargetAbilitySystem.IsValid())
//		{
//			FGameplayAbilitySpec* Spec = TargetAbilitySystem->FindAbilitySpecFromHandle(Handle.Handle);
//			if (!ensureAlwaysMsgf(Spec, TEXT("Spec not found"))) { return; }
//
//			AbilityInstance = Spec->GetPrimaryInstance();
//
//			if (UMyGameplayAbilityBase* MyGA = Cast<UMyGameplayAbilityBase>(AbilityInstance))
//			{
//				Token = MakeShared<FInteractionOptionToken, ESPMode::NotThreadSafe>(*MyGA);
//
//				GameplayEventData = MakeShared<FGameplayEventData, ESPMode::NotThreadSafe>(MyGA->MakeGameplayEventData());
//			}
//		}
//	}
//
//public:
//	bool Activate()
//	{
//		UE_LOG(LogTemp, Warning, TEXT("Activate 111"));
//		if (!IsValid()) { return false; }
//		UE_LOG(LogTemp, Warning, TEXT("Activate 222"));
//
//		if (GameplayEventData && GameplayEventData->EventTag.IsValid())
//		{
//			return TargetAbilitySystem->HandleGameplayEvent(GameplayEventData->EventTag, GameplayEventData.Get()) > 0;
//		}
//		else { return TargetAbilitySystem->TryActivateAbility(TargetInteractionAbilityHandle); }
//	}
//
//public:
//	FInteractionOption(FInteractionOption&& Other) = default;
//	FInteractionOption(const FInteractionOption& Other) = default;
//	FInteractionOption& operator=(const FInteractionOption&) = default;
//	FInteractionOption& operator=(FInteractionOption&&) = default;
//	~FInteractionOption() = default;
//
//	inline bool IsValid() const { return AbilityInstance.IsValid() && TargetAbilitySystem.IsValid() && TargetInteractionAbilityHandle.IsValid(); }
//
//	/** �ڻ��������ϼ������� */
//	UPROPERTY(BlueprintReadWrite)
//	TWeakObjectPtr<UAbilitySystemComponent> TargetAbilitySystem{ nullptr };
//
//	UPROPERTY(BlueprintReadWrite)
//	TWeakObjectPtr<UGameplayAbility> AbilityInstance;
//
//	UPROPERTY(BlueprintReadWrite)
//	FGameplayAbilitySpecHandle TargetInteractionAbilityHandle;
//
//	TSharedPtr<FGameplayEventData, ESPMode::NotThreadSafe> GameplayEventData;
//
//private:
//	TSharedPtr<FInteractionOptionToken, ESPMode::NotThreadSafe> Token;
//
//public:
//	inline bool operator==(const FInteractionOption& Other) const
//	{
//		return TargetAbilitySystem == Other.TargetAbilitySystem &&
//			TargetInteractionAbilityHandle == Other.TargetInteractionAbilityHandle;
//	}
//};