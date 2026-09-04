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

	TWeakObjectPtr<AActor> Instigator;

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