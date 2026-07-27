#pragma once

#include "CoreMinimal.h"
#include "InteractionOption.h"
#include "Ability/MyGameplayAbilityType.h"
//#include "InteractableTargetInterface.h"
#include "UObject/StrongObjectPtr.h"
#include "InteractionOptionsBuilder.generated.h"

class IInteractableTargetInterface;

USTRUCT(BlueprintType)
struct FInteractionOptionsBuilder
{
	GENERATED_BODY()

public:
	FInteractionOptionsBuilder() = default;

	/**
	 * 添加一个互动选项
	 * @param Option 要添加的选项
	 */
	void AddInteractionOption(UInteractionOptionBase* Option)
	{
		if (!Option || !Option->IsValid()) { return; }

		Options.Add(TStrongObjectPtr<UInteractionOptionBase>{Option});
	}

	//void AddInteractionOption(FCombinedAbilityHandle Handle)
	//{
	//	Handle.
	//	FInteractionOption Option{ Handle };
	//	if (!Option.IsValid()) { return; }

	//	AddInteractionOption(MoveTemp(Option));
	//}

	inline int32 GetOptionsNum() { return Options.Num(); }

	inline UInteractionOptionBase* GetOption(int32 Index)
	{
		if (Index < GetOptionsNum()) { return Options[Index].Get(); }
		return nullptr;
	}

	/** Returns the list of all activatable abilities. */
	/*inline*/FORCENOINLINE TArray<UInteractionOptionBase*> GetOptions() const
	{
		TArray<UInteractionOptionBase*> Result;
		for (const TStrongObjectPtr<UInteractionOptionBase>& Option : Options)
		{
			if (Option) { Result.Add(Option.Get()); }
		}
		return Result;
	}

	inline bool ActivateOption(const int32 Index)
	{
		UInteractionOptionBase* Option{ GetOption(Index) };
		return Option && Option->Activate();
	}

	FORCEINLINE void Empty()
	{
		for (TStrongObjectPtr<UInteractionOptionBase>& Option : Options)
		{
			if (Option) { Option->Destroy(); }
		}
		Options.Empty();
	}

	FORCEINLINE void ClearAllInactiveOption()
	{
		for (auto It{ Options.CreateIterator() }; It; ++It)
		{
			if (const TStrongObjectPtr<UInteractionOptionBase>& Option{ *It }; !Option.IsValid() || !Option->IsActivating())
			{
				if (Option.IsValid()) { Option->Destroy(); }
				It.RemoveCurrent();
			}
		}
	}

	inline void RemoveInteractionOption(UInteractionOptionBase* InOption)
	{
		const int32 Index{ Options.IndexOfByPredicate([InOption](const TStrongObjectPtr<UInteractionOptionBase>& Option) {return Option && Option.Get() == InOption; }) };
		if (Options.IsValidIndex(Index))
		{
			if (Options[Index]) { Options[Index]->Destroy(); }
			Options.RemoveAt(Index);
		}
	}

	//inline void RemoveOptionByAbility(UGameplayAbility* GA)
	//{
	//	if (!GA) { return; }

	//	int32 Index = GetOptions().IndexOfByPredicate([GA](const TObjectPtr<UInteractionOptionBase>& Option) {
	//		if (!Option || !Option->IsValid()) { return false; }
	//		FGameplayAbilitySpec* Spec = Option.TargetAbilitySystem->FindAbilitySpecFromHandle(Option.TargetInteractionAbilityHandle);
	//		return Spec && Spec->GetPrimaryInstance() == GA;
	//		});

	//	if (Index != INDEX_NONE) { Options.RemoveAt(Index); }
	//}

	//private:
		/** 这个构建器所属的可互动对象 */
	TScriptInterface<IInteractableTargetInterface> InteractableTarget;

	/** 指向正在构建的选项数组的指针 */
	//UPROPERTY(BlueprintReadOnly)
	TArray<TStrongObjectPtr<UInteractionOptionBase>> Options;
};