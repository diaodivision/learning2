// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "MyGameplayAbilityType.h"
#include "Interface/RecordableAbilityInterface.h"
#include "InputRecordedDataTypes/RecordedDataDefines.h"
#include "Interactable/InteractableTargetInterface.h"
#include "InputRecordedDataTypes/RecordedDataDelegates.h"
#include "Interactive/ItemIconProviderInterface.h"
#include "Delegates/DelegateCombinations.h"
#include "MyGameplayAbilityBase.generated.h"

class URewindSubsystem;

DECLARE_MULTICAST_DELEGATE_OneParam(FPostRecordDelegate, const FRecordedDataObjectHandle& Handle);

/**
 *
 */
UCLASS(Blueprintable)
//class LEARNING2_API UMyGameplayAbilityBase : public UGameplayAbility, public FRecordableAbilityBase
class LEARNING2_API UMyGameplayAbilityBase : public URecordableAbilityBase, public IAbilityIconProviderInterface
{
	GENERATED_BODY()

	friend class FCombinedAbilityIterator;
	friend class FCombinedAbilityConstIterator;

public:
	UMyGameplayAbilityBase();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;

	virtual bool CanActivateBoundAbility() const;

	bool IsBound() const;

	FORCEINLINE virtual void PreActivateInteractiveOption() { K2_PreActivateInteractiveOption(); }
	UFUNCTION(BlueprintImplementableEvent)
	void K2_PreActivateInteractiveOption();

	UFUNCTION(BlueprintCallable, Category = "Combinable Ability")
	bool CanBindWith(const UMyGameplayAbilityBase* OtherGA) const;
	static bool CanBindWith(const UMyGameplayAbilityBase* Instigator, const UMyGameplayAbilityBase* Target);

	//UFUNCTION(BlueprintCallable, Category = "Combinable Ability")
	static FCombinedAbilityHandle BindWith(const FBindAbilityParameter& InInstigator, const FBindAbilityParameter& InTarget, TFunction<void()> RedoCallback);
	void Unbind();

	bool NeedBound() const;
	bool NeedBindTo() const;

	virtual ERecordableActionType GetActionType_Implementation() const override;
	virtual bool ShouldRecord_Implementation() const override;
	virtual bool ShouldStopWhenFailToHandleRecordedData_Implementation() const override;

	virtual bool TryHandleRecordedData(TSharedPtr<IRecordedDataObjectInterface, ESPMode::NotThreadSafe> InRecordedData) override;

protected:
	virtual void PreRecord() override;

	UFUNCTION(BlueprintImplementableEvent)
	void K2_PreRecord(const FGameplayEventData& TriggerEventData);

	virtual void Record(const FGameplayEventData* TriggerEventData) override;
	UFUNCTION(BlueprintCallable, Category = "Recordable", meta = (DisplayName = "Record"))
	FORCEINLINE void K2_Record() { Record(nullptr); }
	UFUNCTION(BlueprintCallable, Category = "Recordable", meta = (DisplayName = "Record From Event"))
	FORCEINLINE void K2_RecordFromEvent(const FGameplayEventData& TriggerEventData) { Record(&TriggerEventData); }
	void Record_Internal(TUniquePtr<IRecordedDataObjectInterface>&& Data, const FGameplayEventData* TriggerEventData);
	virtual void PostRecord(const FRecordedDataObjectHandle& Handle) override;
	FORCEINLINE static void InvokePostRecord(UMyGameplayAbilityBase* Target, const FRecordedDataObjectHandle& Handle)
	{
		if (Target) { Target->PostRecord(Handle); }
	}

	virtual void OnPreview(const bool bIsPreview, const IRecordedDataObjectInterface* InRecordedData) override;
	void OnPreview_Internal(const bool bIsPreview, const FRecordedCombinableAbilityData& InRecordedData);
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Preview"))
	void K2_OnPreview(const bool bIsPreview);

	[[nodiscard]] FRecordedCombinableAbilityData MakeRecordedCombinableAbilityData();

	UFUNCTION(BlueprintNativeEvent, Category = "Recordable")
	AActor* GetRecordAvatar() const;
	virtual FORCEINLINE AActor* GetRecordAvatar_Implementation() const { return GetAvatarActorFromActorInfo(); }

public:
	UFUNCTION(BlueprintCallable, Category = "Recordable")
	virtual inline EActionState GetActionState() const override { return AbilityState; };
	UFUNCTION(BlueprintCallable, Category = "Recordable")
	inline void SetActionState(const EActionState State)
	{
		const EActionState OldState{ AbilityState };
		AbilityState = State;

		if (BoundAbilityInfo.Ability.IsValid()) { BoundAbilityInfo.Ability->SetActionState(AbilityState); }

		OnAbilityStateChangedDelegate.Broadcast(OldState, AbilityState);
	};
	virtual const UGameplayEffect* GetReverseCostGameplayEffect_Implementation() const;

	inline bool IsFirstAbilityOfBindAbility() const { return !TagToBind.IsValid() && BindTag.IsValid(); }

	virtual TSoftObjectPtr<UTexture2D> GetItemIcon_Implementation(const UAbilitySystemComponent* ASC) const override;

	UFUNCTION(BlueprintImplementableEvent)
	[[nodiscard]] FGameplayEventData MakeGameplayEventData() const;

	UFUNCTION(BlueprintPure)
	[[nodiscard]] TArray<AActor*> GetCombinedAbilityAvatarActorList() const;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION(BlueprintCallable)
	void ExecuteBindAbility();
	UFUNCTION(BlueprintImplementableEvent, Category = "Combinable Ability", meta = (DisplayName = "Execute Bind Ability"))
	void K2_ExecuteBindAbility(UPARAM(ref)FGameplayEventData& GameplayEventData);

	virtual FORCEINLINE void PostExecuteBindAbility() {}

#if WITH_EDITOR
	virtual void HandleFailToExecuteBindAbility(const FString& InReason);
#endif

	//virtual bool PrepareToHandleRecordedData(IRecordedDataObjectInterface* InRecordedData) override;

	static TArray<GameplayAbilityIDType> GetCombinedAbilityIDList(const UMyGameplayAbilityBase& FirstAbility);

	//static TArray<FShowWillExecuteOperationDelegate> GetPreviewAbilityDelegateList(const UMyGameplayAbilityBase& FirstAbility);

	void PassRecordedToBoundAbility();

	UFUNCTION(BlueprintCallable, Category = "Combinable Ability")
	void K2_NotifyEndDurativeAction();

	//UFUNCTION(BlueprintCallable, Category = "Recordable")
	//void K2_SetShowWillExecuteOperationDelegate(FShowWillExecuteOperationDelegate InDelegate);

	UFUNCTION(BlueprintCallable)
	inline void EndAbilityWithCancelState(const bool bWasCancelled)
	{
		constexpr bool bReplicateEndAbility{ true };
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicateEndAbility, bWasCancelled);
	}

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetGameplayEventData(const FGameplayEventData& Data) { CombinableAbilityData.EventDataToBoundAbility = MakeUnique<FGameplayEventWeakData>(Data); }

private:
	static bool GetGameplayAbilityID(GameplayAbilityIDType& ID, const UAbilitySystemComponent& ASC, const UMyGameplayAbilityBase* GA);

public:
	FPostRecordDelegate PostRecordDelegate;

	//��Ҫ������ Tag ���ܰ󶨵��ü���
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Tags)
	FGameplayTag BindTag;

	//�ü��ܽ��ᱻ�󶨵�ӵ������ tag �ļ���
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Tags)
	FGameplayTag TagToBind;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bind Type")
	EBindType BindType{ EBindType::NoBind };

	TOptional<GameplayAbilityIDType> AbilityID;

	UPROPERTY(BlueprintAssignable)
	FOnAbilityStateChangedDelegate OnAbilityStateChangedDelegate;

protected:
	UPROPERTY(EditDefaultsOnly, Category = Costs)
	TSubclassOf<UGameplayEffect> ReverseCostGameplayEffectClass;

	FCombinableAbilityData CombinableAbilityData;

	//TOptional<FBoundAbilityInfo> BoundAbilityInfo;
	FBoundAbilityInfo BoundAbilityInfo;

	//UPROPERTY(BlueprintReadWrite, Category = "Abilities")
	EActionState AbilityState;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	bool bIsAutoRecordWhenShouldRecord{ true };
};
