#pragma once
#include "MyGameplayAbilityType.h"
#include "Abilities/GameplayAbility.h"
#include "MyGameplayAbilityBase.h"
#include "AbilitySystemComponent.h"

FCombinedAbilityHandle::FCombinedAbilityHandle(UMyGameplayAbilityBase* InAbilityInstance) : AbilityInstance(InAbilityInstance)
{
	if (InAbilityInstance) { GameplayEventData = MakeUnique<FGameplayEventData>(InAbilityInstance->MakeGameplayEventData()); }
}

FCombinedAbilityHandle::FCombinedAbilityHandle(const FBindAbilityParameter& BindAbilityParameter) : AbilityInstance(BindAbilityParameter.AbilityInstance)
{
	if (BindAbilityParameter.AbilityInstance.IsValid()) { GameplayEventData = MakeUnique<FGameplayEventData>(BindAbilityParameter.AbilityInstance->MakeGameplayEventData()); }
}

FBindAbilityParameter::FBindAbilityParameter(UMyGameplayAbilityBase& Ability) :AbilityInstance(&Ability)
{
}

FBoundAbilityInfo::FBoundAbilityInfo(const FBindAbilityParameter& BindAbilityParameter, const FBindAbilityParameter& InstigatorAbilityParameter, const FBindAbilityParameter& TargetAbilityParameter, TFunction<void()> RedoBindCallback)
	: Ability(BindAbilityParameter.AbilityInstance),
	BindInstigatorAbility(InstigatorAbilityParameter.AbilityInstance),
	BindTargetAbility(TargetAbilityParameter.AbilityInstance),
	RedoBindCallback(RedoBindCallback)
{
	if (!BindAbilityParameter.IsValid())
	{
		Reset();
		return;
	}

	Handle = BindAbilityParameter.AbilityInstance->GetCurrentAbilitySpecHandle();
	AbilitySystemComponent = BindAbilityParameter.AbilityInstance->GetAbilitySystemComponentFromActorInfo();
	BindInstigator = InstigatorAbilityParameter.AbilityInstance->GetAvatarActorFromActorInfo();
	BindTarget = TargetAbilityParameter.AbilityInstance->GetAvatarActorFromActorInfo();
}

bool FBoundAbilityInfo::CanActivateAbility() const
{
	if (!IsValid()) { return false; }

	FGameplayAbilityActorInfo* ActorInfo = AbilitySystemComponent->AbilityActorInfo.Get();
	if (!ActorInfo) { return false; }

	return Ability->CanActivateAbility(Handle, ActorInfo);
}

void FBoundAbilityInfo::CancelAbility()
{
	if (IsValid()) { AbilitySystemComponent->CancelAbilityHandle(Handle); }
}

EBindType FBoundAbilityInfo::GetBindType() const
{
	if (IsValid()) { return Ability->BindType; }

	return EBindType::NoBind;
}

void FBoundAbilityInfo::Reset()
{
	Ability.Reset();
	Handle = FGameplayAbilitySpecHandle{};
	AbilitySystemComponent.Reset();
	BindInstigator.Reset();
	BindTarget.Reset();
	BindInstigatorAbility.Reset();
	BindTargetAbility.Reset();
	RedoBindCallback.Reset();
}

UInteractionAbilityOption* UInteractionAbilityOption::CreateInteractionAbilityOption(UGameplayAbility* InAbilityInstance, const int32 GroupID)
{
	if (!InAbilityInstance) { return nullptr; }

	UInteractionAbilityOption* Option{ NewObject<UInteractionAbilityOption>() };
	CreateInteractionAbilityOption(*Option, InAbilityInstance, GroupID);

	return Option;
}

UInteractionAbilityOption* UInteractionAbilityOption::CreateInteractionAbilityOption(FCombinedAbilityHandle&& Handle, const int32 GroupID, TSoftObjectPtr<UTexture2D> InIcon)
{
	if (!Handle.IsValid()) { return nullptr; }

	UInteractionAbilityOption* Option{ NewObject<UInteractionAbilityOption>() };
	CreateInteractionAbilityOption(*Option, MoveTemp(Handle), GroupID,InIcon);

	return Option;
}

void UInteractionAbilityOption::CreateInteractionAbilityOption(UInteractionAbilityOption& Option, UGameplayAbility* InAbilityInstance, const int32 GroupID)
{
	if (!InAbilityInstance) { return; }

	Option.AbilityInstance = InAbilityInstance;
	Option.GroupID = GroupID;

	if (InAbilityInstance && InAbilityInstance->Implements<UAbilityIconProviderInterface>())
	{
		Option.Icon = IAbilityIconProviderInterface::Execute_GetItemIcon(InAbilityInstance, InAbilityInstance->GetAbilitySystemComponentFromActorInfo());
	}
}

void UInteractionAbilityOption::CreateInteractionAbilityOption(UInteractionAbilityOption& Option, FCombinedAbilityHandle&& Handle, const int32 GroupID, TSoftObjectPtr<UTexture2D> InIcon)
{
	if (!Handle.IsValid()) { return; }

	Option.AbilityInstance = Handle.AbilityInstance;
	Option.GroupID = GroupID;
	Option.GameplayEventData = MoveTemp(Handle.GameplayEventData);

	if (!InIcon.IsNull()) { Option.Icon = InIcon; }
	else if (Option.AbilityInstance.IsValid() && Option.AbilityInstance->Implements<UAbilityIconProviderInterface>())
	{
		Option.Icon = IAbilityIconProviderInterface::Execute_GetItemIcon(Option.AbilityInstance.Get(), Option.AbilityInstance->GetAbilitySystemComponentFromActorInfo());
	}
}

bool UInteractionAbilityOption::Activate()
{
	if (!IsValid()) { return false; }

	UAbilitySystemComponent* AbilitySystemComponent{ AbilityInstance->GetAbilitySystemComponentFromActorInfo() };
	if (!ensure(AbilitySystemComponent)) { return false; }

	if (UMyGameplayAbilityBase* MyGA{ Cast<UMyGameplayAbilityBase>(AbilityInstance) }) { MyGA->PreActivateInteractiveOption(); }

	//return AbilitySystemComponent->HandleGameplayEvent(GameplayEventData->EventTag, GameplayEventData.Get()) > 0;
	if (GameplayEventData && GameplayEventData->EventTag.IsValid())
	{
		return AbilitySystemComponent->HandleGameplayEvent(GameplayEventData->EventTag, GameplayEventData.Get()) > 0;
	}
	else { return AbilitySystemComponent->TryActivateAbility(AbilityInstance->GetCurrentAbilitySpecHandle()); }
}

bool UInteractionAbilityOption::CanDestroy()
{
	if (!RecordedDataObjectHandle.IsSet() || !InputRecordComponent.IsValid()) { return true; }

	return InputRecordComponent->IsRecordDataObjectExist(RecordedDataObjectHandle.GetValue()) == false;
}

void UInteractionAbilityOption::Destroy()
{
	if (!IsAlive()) { return; }

	Super::Destroy();

	if (UMyGameplayAbilityBase * MyGA{ Cast<UMyGameplayAbilityBase>(AbilityInstance) }) { MyGA->Unbind(); }
}

FGameplayEventWeakData::FGameplayEventWeakData(const FGameplayEventData& Data) :
	EventTag(Data.EventTag),
	Instigator(Data.Instigator),
	Target(Data.Target),
	OptionalObject(Data.OptionalObject),
	OptionalObject2(Data.OptionalObject2),
	ContextHandle(Data.ContextHandle),
	InstigatorTags(Data.InstigatorTags),
	TargetTags(Data.TargetTags),
	EventMagnitude(Data.EventMagnitude),
	TargetData(Data.TargetData)
{
}

FGameplayEventData FGameplayEventWeakData::Pin() const
{
	FGameplayEventData Result;
	Result.EventTag = EventTag;
	Result.Instigator = Instigator.Get();
	Result.Target = Target.Get();
	Result.OptionalObject = OptionalObject.Get();
	Result.OptionalObject2 = OptionalObject2.Get();
	Result.ContextHandle = ContextHandle;
	Result.InstigatorTags = InstigatorTags;
	Result.TargetTags = TargetTags;
	Result.EventMagnitude = EventMagnitude;
	Result.TargetData = TargetData;

	return Result;
}

bool FRecordedCombinableAbilityData::IsPayloadValid() const
{
	return Payload.AbilityComponent.IsValid() && Payload.EventDataToBoundAbility.IsValid() /*&& Payload.RedoBindCallback.IsSet()*/;
}

//void FRecordedCombinableAbilityData::PreviewOnHandlePayload(bool bIsPreview)
//{
//	for (FShowWillExecuteOperationDelegate& Delegate : Payload.AbilityPreviewDelegateList)
//	{
//		Delegate.ExecuteIfBound(bIsPreview);
//	}
//}


bool FRecordedCombinableAbilityData::PrepareToHandleRecordedData()
{
	if (!IsPayloadValid()) { return false; }

	if (Payload.RedoBindCallback.IsSet()) { Payload.RedoBindCallback(); }

	return true;
}

bool FRecordedCombinableAbilityData::ConsumeAndTryHandlePayload(TSharedPtr<IRecordedDataObjectInterface, ESPMode::NotThreadSafe> RecordedData)
{
	if (!IsPayloadValid()) { return false; }

	const FGameplayAbilitySpec* Spec = Payload.AbilityComponent->FindAbilitySpecFromClass(Payload.AbilityClass);
	if (!Spec) { return false; }

	UMyGameplayAbilityBase* Ability = Cast<UMyGameplayAbilityBase>(Spec->GetPrimaryInstance());

	if (!Ability || Ability == Ability->GetClass()->GetDefaultObject()) { return false; }

	return Ability->TryHandleRecordedData(RecordedData);
}

bool FRecordedCombinableAbilityData::PrepareToPreview()
{
	if (!IsPayloadValid()) { return false; }

	if (Payload.RedoBindCallback.IsSet()) { Payload.RedoBindCallback(); }

	return true;
}

void FRecordedCombinableAbilityData::Preview(const bool bIsPreview)
{
	Payload.PreviewCallback(bIsPreview, this);
}

bool FRecordedCombinableAbilityData::ShouldStopRewindWhenHandleThisUnsuccessful() const
{
	constexpr bool bDefaultValue{ false };

	const FGameplayAbilitySpec* Spec = Payload.AbilityComponent->FindAbilitySpecFromClass(Payload.AbilityClass);
	if (!Spec) { return bDefaultValue; }

	const UMyGameplayAbilityBase* Ability = Cast<UMyGameplayAbilityBase>(Spec->GetPrimaryInstance());
	if (!Ability) { return bDefaultValue; }

	return IRecordableInterfaceBase::Execute_ShouldStopWhenFailToHandleRecordedData(Ability);
}

FCombinedAbilityIterator::FCombinedAbilityIterator(UMyGameplayAbilityBase& FirstAbility)
	: CurrentAbility(FirstAbility.IsFirstAbilityOfBindAbility() ? &FirstAbility : nullptr)
{
}

void FCombinedAbilityIterator::operator++()
{
	CurrentAbility = CurrentAbility ? CurrentAbility->BoundAbilityInfo.Ability.Get() : nullptr;
}

UMyGameplayAbilityBase* FCombinedAbilityIterator::operator*() const
{
	return CurrentAbility;
}

FCombinedAbilityIterator::operator bool() const
{
	return CurrentAbility != nullptr;
}

FCombinedAbilityConstIterator::FCombinedAbilityConstIterator(const UMyGameplayAbilityBase& FirstAbility)
	: CurrentAbility(FirstAbility.IsFirstAbilityOfBindAbility() ? &FirstAbility : nullptr)
{
}

void FCombinedAbilityConstIterator::operator++()
{
	CurrentAbility = CurrentAbility ? CurrentAbility->BoundAbilityInfo.Ability.Get() : nullptr;
}

const UMyGameplayAbilityBase* FCombinedAbilityConstIterator::operator*() const
{
	return CurrentAbility;
}

FCombinedAbilityConstIterator::operator bool() const
{
	return CurrentAbility != nullptr;
}