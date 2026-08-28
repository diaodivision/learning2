#pragma once
#include "GameplayAbilitySpecHandle.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Misc/Optional.h"
#include "Templates/Function.h"
#include "InputRecordedDataTypes/InputRecordedDataTemplates.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "Delegates/DelegateCombinations.h"
#include "Interactable/InteractionOption.h"
#include "InputRecordComponent.h"
#include "MyGameplayAbilityType.generated.h"

class UMyGameplayAbilityBase;
class UAbilitySystemComponent;
class AActor;
class UTexture2D;
class UInputRecordComponent;

// using FPostRecordCallbackType = TDelegate<void(FRecordedDataObjectHandle Handle), FNotThreadSafeNotCheckedDelegateUserPolicy>;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityStateChangedDelegate, EActionState, OldState, EActionState, NewState);

UENUM(BlueprintType)
enum class EBindType : uint8
{
	NoBind,
	Bind
};

using GameplayAbilityIDType = uint8;

class FCombinedAbilityIterator
{
public:
	explicit FCombinedAbilityIterator(UMyGameplayAbilityBase& FirstAbility);

	void operator++();
	UMyGameplayAbilityBase* operator*() const;
	explicit operator bool() const;

private:
	UMyGameplayAbilityBase* CurrentAbility;
};

class FCombinedAbilityConstIterator
{
public:
	explicit FCombinedAbilityConstIterator(const UMyGameplayAbilityBase& FirstAbility);

	void operator++();
	const UMyGameplayAbilityBase* operator*() const;
	explicit operator bool() const;

private:
	const UMyGameplayAbilityBase* CurrentAbility;
};

//USTRUCT(BlueprintType)
struct FCombinedAbilityHandle
{
	//GENERATED_BODY()

	FCombinedAbilityHandle() = default;
	explicit FCombinedAbilityHandle(UMyGameplayAbilityBase* InAbilityInstance);
	explicit FCombinedAbilityHandle(const FBindAbilityParameter& BindAbilityParameter);

	inline bool IsValid() const { return AbilityInstance.IsValid(); }

	TWeakObjectPtr<UMyGameplayAbilityBase> AbilityInstance{ nullptr };

	TUniquePtr<FGameplayEventData> GameplayEventData{ nullptr };
};

USTRUCT(BlueprintType)
struct FBindAbilityParameter
{
	GENERATED_BODY()

	FBindAbilityParameter() = default;
	explicit FBindAbilityParameter(UMyGameplayAbilityBase& Ability);

	inline bool IsValid() const { return AbilityInstance.IsValid(); }

	UPROPERTY()
	TWeakObjectPtr<UMyGameplayAbilityBase> AbilityInstance{ nullptr };
};

USTRUCT()
struct FBoundAbilityInfo
{
	GENERATED_BODY()

	FBoundAbilityInfo() = default;
	FBoundAbilityInfo(const FBindAbilityParameter& BindAbilityParameter, const FBindAbilityParameter& InstigatorAbilityParameter, const FBindAbilityParameter& TargetAbilityParameter, TFunction<void()> RedoBindCallback);

	bool CanActivateAbility() const;

	void CancelAbility();

	inline bool IsValid() const
	{
		return Ability.IsValid() &&
			Handle.IsValid() &&
			AbilitySystemComponent.IsValid() &&
			BindInstigator.IsValid() &&
			BindTarget.IsValid() &&
			BindInstigatorAbility.IsValid() &&
			BindTargetAbility.IsValid() &&
			RedoBindCallback.IsSet();
	}

	EBindType GetBindType() const;

	void Reset();

	/*UPROPERTY()
	TWeakObjectPtr<UMyGameplayAbilityBase> Ability{ nullptr };*/

	UPROPERTY()
	TWeakObjectPtr<UMyGameplayAbilityBase> Ability{ nullptr };

	FGameplayAbilitySpecHandle Handle;

	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent{ nullptr };

	UPROPERTY()
	TWeakObjectPtr<AActor> BindInstigator{ nullptr };

	UPROPERTY()
	TWeakObjectPtr<AActor> BindTarget{ nullptr };

	UPROPERTY()
	TWeakObjectPtr<UMyGameplayAbilityBase> BindInstigatorAbility{ nullptr };

	UPROPERTY()
	TWeakObjectPtr<UMyGameplayAbilityBase> BindTargetAbility{ nullptr };

	TFunction<void()> RedoBindCallback;
};

struct FCombinableAbilityData
{
	FORCEINLINE bool IsValid() const { return EventDataToBoundAbility.IsValid() /*&& PostRecordCallback.IsSet() */; }

	FORCEINLINE bool IsNull() const { return  !EventDataToBoundAbility.IsValid() /*&& !PostRecordCallback.IsSet()*/; }

	FORCEINLINE void Reset()
	{
		EventDataToBoundAbility.Reset();
		// PostRecordCallback.Reset();
	}

	TUniquePtr<FGameplayEventWeakData> EventDataToBoundAbility;

	// TOptional<FPostRecordCallbackType> PostRecordCallback;
};

UCLASS(BlueprintType)
class UInteractionAbilityOption : public UInteractionOptionBase
{
	GENERATED_BODY()

public:
	//void InitializeObject(UGameplayAbility* InAbilityInstance);
	//void InitializeObject(FCombinedAbilityHandle&& Handle, TSoftObjectPtr<UTexture2D> InIcon);

	static UInteractionAbilityOption* CreateInteractionAbilityOption(UGameplayAbility* InAbilityInstance, const int32 GroupID, AActor* InInstigator);
	static UInteractionAbilityOption* CreateInteractionAbilityOption(FCombinedAbilityHandle&& Handle, const int32 GroupID, AActor* InInstigator, TSoftObjectPtr<UTexture2D> InIcon = nullptr);
	static void CreateInteractionAbilityOption(UInteractionAbilityOption& Option, UGameplayAbility* InAbilityInstance, const int32 GroupID, AActor* InInstigator);
	static void CreateInteractionAbilityOption(UInteractionAbilityOption& Option, FCombinedAbilityHandle&& Handle, const int32 GroupID, AActor* InInstigator, TSoftObjectPtr<UTexture2D> InIcon = nullptr);

	virtual bool Activate() override;

	virtual FORCEINLINE TSoftObjectPtr<UTexture2D> GetIcon() const override { return Icon; }

	virtual FORCEINLINE bool IsValid() const override { return Super::IsValid() && AbilityInstance.IsValid(); }

	virtual bool CanDestroy() override;

	virtual void Destroy() override;

	virtual FORCEINLINE void BeginDestroy() override
	{
		Super::BeginDestroy();

		Destroy();
	}

	FORCEINLINE void SetRecordedDataObjectHandle(const UInputRecordComponent* InInputRecordComponent, const FRecordedDataObjectHandle& InHandle)
	{
		InputRecordComponent = InInputRecordComponent;
		RecordedDataObjectHandle = InHandle;
	}
	FORCEINLINE const FRecordedDataObjectHandle* GetRecordedDataObjectHandle() const { return RecordedDataObjectHandle.GetPtrOrNull(); }
	FORCEINLINE const UInputRecordComponent* GetInputRecordComponent() const { return InputRecordComponent.Get(); }

	FORCEINLINE UGameplayAbility* GetAbilityInstance() const { return AbilityInstance.Get(); }

private:
	/** 在互动对象上激活能力 */
	//UPROPERTY(BlueprintReadWrite)
	//TWeakObjectPtr<UAbilitySystemComponent> TargetAbilitySystem{ nullptr };

	UPROPERTY()
	TWeakObjectPtr<UGameplayAbility> AbilityInstance{ nullptr };

	TUniquePtr<FGameplayEventData> GameplayEventData{ nullptr };

	TWeakObjectPtr<const UInputRecordComponent> InputRecordComponent;
	TOptional<FRecordedDataObjectHandle> RecordedDataObjectHandle;

	UPROPERTY()
	TSoftObjectPtr<UTexture2D> Icon;
};

USTRUCT()
struct FGameplayEventWeakData
{
	GENERATED_BODY()

	FGameplayEventWeakData() = default;
	explicit FGameplayEventWeakData(const FGameplayEventData& Data);

	FGameplayEventData Pin() const;

	/** Tag of the event that triggered this */
	UPROPERTY()
	FGameplayTag EventTag;

	/** The instigator of the event */
	UPROPERTY()
	TWeakObjectPtr<const AActor> Instigator;

	/** The target of the event */
	UPROPERTY()
	TWeakObjectPtr<const AActor> Target;

	/** An optional ability-specific object to be passed though the event */
	UPROPERTY()
	TObjectPtr<const UObject> OptionalObject;

	/** A second optional ability-specific object to be passed though the event */
	UPROPERTY()
	TObjectPtr<const UObject> OptionalObject2;

	/** Polymorphic context information */
	UPROPERTY()
	FGameplayEffectContextHandle ContextHandle;

	/** Tags that the instigator has */
	UPROPERTY()
	FGameplayTagContainer InstigatorTags;

	/** Tags that the target has */
	UPROPERTY()
	FGameplayTagContainer TargetTags;

	/** The magnitude of the triggering event */
	UPROPERTY()
	float EventMagnitude{ 0.f };

	/** The polymorphic target information for the event */
	UPROPERTY()
	FGameplayAbilityTargetDataHandle TargetData;
};

static_assert(sizeof(FGameplayEventData) == sizeof(FGameplayEventWeakData));

struct FGameplayEventDataWithRecordedDataObjectToken : public FGameplayEventData
{
	TSharedPtr<FObjectToken::ObjectTokenType, ESPMode::NotThreadSafe> RecordedDataObjectToken;
};

struct FRecordedCombinableAbilityDataPayloadBase
{
	TWeakObjectPtr<UAbilitySystemComponent> AbilityComponent;

	TOptional<FGameplayTag> AbilityTriggerTag;

	TSubclassOf<UGameplayAbility> AbilityClass;

	TUniquePtr<FGameplayEventWeakData> EventDataToBoundAbility;

	TArray<GameplayAbilityIDType> AbilityIDList;
	//TArray<FShowWillExecuteOperationDelegate> AbilityPreviewDelegateList;

	TFunction<void()> RedoBindCallback;

	TFunction<void(bool, const IRecordedDataObjectInterface*)> PreviewCallback;
};

struct FRecordedCombinableAbilityData :
	public TRecordedDataBase<ERecordableActionType::HasDuration, FRecordedCombinableAbilityData, FRecordedCombinableAbilityDataPayloadBase>
{
	virtual bool IsPayloadValid() const override;

	virtual bool PrepareToHandleRecordedData() override;
	virtual bool ConsumeAndTryHandlePayload(TSharedPtr<IRecordedDataObjectInterface, ESPMode::NotThreadSafe> RecordedData) override;

	virtual bool PrepareToPreview() override;
	virtual void Preview(const bool bIsPreview) override;

	//virtual void PreviewOnHandlePayload(bool bIsPreview) override;
	virtual bool ShouldStopRewindWhenHandleThisUnsuccessful() const override;
};