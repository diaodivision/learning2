// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameplayAbilityBase.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"
#include "BlueprintGameplayTagLibrary.h"
#include "InputRecordedDataTypes/RecordableInterface.h"
#include "InputRecordedDataTypes/RecordedDataTypes.h"
#include "RewindSystemStatics.h"
#include "RewindSubsystem.h"

UMyGameplayAbilityBase::UMyGameplayAbilityBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UMyGameplayAbilityBase::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	bool bCanActivate = Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);

	if (!NeedBound()) { return bCanActivate; }

	return bCanActivate && IsBound() && CanActivateBoundAbility();
}

void UMyGameplayAbilityBase::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if (TagToBind.IsValid())
	{
		int32 Index = AbilityTriggers.IndexOfByPredicate([this](const FAbilityTriggerData& TriggerData) {
			return TriggerData.TriggerTag == TagToBind && TriggerData.TriggerSource == EGameplayAbilityTriggerSource::GameplayEvent;
			});

		ensureMsgf(Index != INDEX_NONE, TEXT("Does not exist TriggerData that same as TagToBind or TriggerData's TriggerSource is not GameplayEvent. Log from Class: %s"), *GetClass()->GetName());
	}

	if (!AbilityID.IsSet())
	{
		GameplayAbilityIDType ID;
		//GetGameplayAbilityID(ID, *ActorInfo->AbilitySystemComponent.Get(), this);
		GetGameplayAbilityID(ID, *GetAbilitySystemComponentFromActorInfo(), this);

		AbilityID = ID;
	}
}

void UMyGameplayAbilityBase::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);

	if (!IsBound()) { return; }

	BoundAbilityInfo.CancelAbility();
}

bool UMyGameplayAbilityBase::CanActivateBoundAbility() const
{
	return BoundAbilityInfo.CanActivateAbility();
}

bool UMyGameplayAbilityBase::IsBound() const
{
	return BoundAbilityInfo.IsValid();
}

bool UMyGameplayAbilityBase::CanBindWith(const UMyGameplayAbilityBase* OtherGA) const
{
	if (!ensureAlwaysMsgf(OtherGA, TEXT("Input GameplayAbility is nullptr"))) { return false; }

	return CanBindWith(this, OtherGA) || CanBindWith(OtherGA, this);

	//if (OtherGA->BindTag.GetTagName() != NAME_None)

	//return !(OtherGA->TagToBind.GetTagName() != NAME_None) && OtherGA->TagToBind == this->TagToBind;
}

bool UMyGameplayAbilityBase::CanBindWith(const UMyGameplayAbilityBase* Instigator, const UMyGameplayAbilityBase* Target)
{
	if (!Target->TagToBind.IsValid() || !ensureMsgf(Target->BindType != EBindType::NoBind, TEXT("TagToBind is specified but BindType is None"))) { return false; }

	if (Instigator->BindTag.IsValid() && Instigator->BindTag == Target->TagToBind) { return true; }

	return false;
}

FCombinedAbilityHandle UMyGameplayAbilityBase::BindWith(const FBindAbilityParameter& InInstigator, const FBindAbilityParameter& InTarget, TFunction<void()> RedoCallback, FPostRecordCallbackType InPostRecordCallback)
{
	/*FBindAbilityParameter* InstigatorPtr = &Instigator;
	FBindAbilityParameter* TargetPtr = &Target;*/
	const FBindAbilityParameter* Instigator{ &InInstigator }, * Target{ &InTarget };

	UE_LOG(LogTemp, Warning, TEXT("BindWith 111"));
	if (!ensureMsgf(Instigator->IsValid() && Target->IsValid(), TEXT("Input GameplayAbility is nullptr"))) { return FCombinedAbilityHandle{ nullptr }; };
	UE_LOG(LogTemp, Warning, TEXT("BindWith 222"));

	if (!CanBindWith(Instigator->AbilityInstance.Get(), Target->AbilityInstance.Get())) { Swap(Instigator, Target); }
	UE_LOG(LogTemp, Warning, TEXT("BindWith 333"));

	UMyGameplayAbilityBase* InstigatorGA = Instigator->AbilityInstance.Get();
	UMyGameplayAbilityBase* TargetGA = Target->AbilityInstance.Get();
	if (!ensureMsgf(InstigatorGA, TEXT("%s: %s is not a UMyGameplayAbilityBase type"), *GetNameSafe(Instigator->AbilityInstance.Get()), *Instigator->AbilityInstance.Get()->GetClass()->GetName())) { return FCombinedAbilityHandle{ nullptr }; }
	UE_LOG(LogTemp, Warning, TEXT("BindWith 444"));
	if (!ensureMsgf(TargetGA, TEXT("%s: %s is not a UMyGameplayAbilityBase type"), *GetNameSafe(Target->AbilityInstance.Get()), *Target->AbilityInstance.Get()->GetClass()->GetName())) { return FCombinedAbilityHandle{ nullptr }; }
	UE_LOG(LogTemp, Warning, TEXT("BindWith 555"));

	if (!ensureMsgf(CanBindWith(Instigator->AbilityInstance.Get(), Target->AbilityInstance.Get()), TEXT("%s can't bind with %s"), *GetNameSafe(InstigatorGA), *GetNameSafe(TargetGA))) { return FCombinedAbilityHandle{ nullptr }; }
	UE_LOG(LogTemp, Warning, TEXT("BindWith 666"));

	if (InstigatorGA->IsBound()) { return FCombinedAbilityHandle{ nullptr }; }
	UE_LOG(LogTemp, Warning, TEXT("BindWith 777"));

	InstigatorGA->BoundAbilityInfo = FBoundAbilityInfo{ *Target, InInstigator, InTarget, RedoCallback };

	UE_LOG(LogTemp, Display, TEXT("Ability %s bind with %s"), *GetNameSafe(InstigatorGA), *GetNameSafe(TargetGA));

	Instigator->AbilityInstance->CombinableAbilityData.PostRecordCallback = InPostRecordCallback;

	return FCombinedAbilityHandle{ *Instigator };
}

void UMyGameplayAbilityBase::Unbind()
{
	UE_LOG(LogTemp, Warning, TEXT("BindWith unbind"));

	if (IsBound())
	{
		BoundAbilityInfo.Ability->Unbind();
		BoundAbilityInfo.Reset();
		CombinableAbilityData.Reset();
	}
}

bool UMyGameplayAbilityBase::NeedBound() const
{
	return BindTag.IsValid();
}

bool UMyGameplayAbilityBase::NeedBindTo() const
{
	return TagToBind.IsValid();
}

ERecordableActionType UMyGameplayAbilityBase::GetActionType_Implementation() const
{
	return ERecordableActionType::HasDuration;
}

void UMyGameplayAbilityBase::Record(const FGameplayEventData* TriggerEventData)
{
	//UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	//if (!AbilitySystemComponent) { return; }

	//TUniquePtr<FRecordedCombinableAbilityData> Data = MakeUnique<FRecordedCombinableAbilityData>();
	//FRecordedCombinableAbilityDataPayloadBase& Payload = Data->Payload;
	//Payload.AbilityComponent = AbilitySystemComponent;
	////Payload.AbilityTriggerTag = AbilityTriggerTag;
	//Payload.AbilityTriggerTag = EventDataToBoundAbility->EventTag;
	//Payload.AbilityClass = GetClass();
	////Payload.EventDataToBoundAbility = MakeUnique<FGameplayEventWeakData>(*EventDataToBoundAbility.Get());
	//Payload.EventDataToBoundAbility = MoveTemp(EventDataToBoundAbility);
	//Payload.AbilityIDList = GetCombinedAbilityIDList(*this);
	////Payload.AbilityPreviewDelegateList = GetPreviewAbilityDelegateList(*this);
	//if (BoundAbilityInfo.IsValid())
	//{
	//	Payload.RedoBindCallback = BoundAbilityInfo.RedoBindCallback;
	//}
	//Payload.PreviewCallback = [WeakThis = MakeWeakObjectPtr<UMyGameplayAbilityBase>(this)](bool bIsPreview, const IRecordedDataObjectInterface* Data) {
	//	if (!WeakThis.IsValid()) { return; }

	//	WeakThis->OnPreview(bIsPreview, Data);
	//	};
	TUniquePtr<FRecordedCombinableAbilityData> Data{ MakeUnique<FRecordedCombinableAbilityData>(MakeRecordedCombinableAbilityData()) };
	if (!ensureAlways(Data->IsPayloadValid())) { return; }

	Record_Internal(MoveTemp(Data), TriggerEventData);
}

void UMyGameplayAbilityBase::Record_Internal(TUniquePtr<IRecordedDataObjectInterface>&& Data, const FGameplayEventData* TriggerEventData)
{
	URewindSubsystem* System = URewindSystemStatics::GetRewindSubsystem(this);
	if (!System) { return; }

	AActor* AvatarActor{ GetRecordAvatar() };
	if (!AvatarActor || !AvatarActor->FindComponentByClass<UInputRecordComponent>()) 
	{ 
		AvatarActor = TriggerEventData ? const_cast<AActor*>(TriggerEventData->Instigator.Get()) : nullptr; 
	}
	if (!AvatarActor || !AvatarActor->FindComponentByClass<UInputRecordComponent>()) { return; }


	FRecordedDataObjectHandle RecordedDataObjectHandle = System->Record(MoveTemp(Data), *AvatarActor,
		[WeakThis = MakeWeakObjectPtr(this)](const FRecordedDataObjectHandle& Handle)
		{
			if (!WeakThis.IsValid()) { return; }

			for (FCombinedAbilityIterator It{ *WeakThis }; It; ++It) { (*It)->PostRecord(Handle); }
		});
}

void UMyGameplayAbilityBase::PostRecord(const FRecordedDataObjectHandle& Handle)
{
	Super::PostRecord(Handle);

	//Unbind();
	if (CombinableAbilityData.PostRecordCallback.IsSet()) { CombinableAbilityData.PostRecordCallback.GetValue().ExecuteIfBound(Handle); }
	//CombinableAbilityData.Reset();
}

bool UMyGameplayAbilityBase::TryHandleRecordedData(TSharedPtr<IRecordedDataObjectInterface, ESPMode::NotThreadSafe> InRecordedData)
{
	FRecordedCombinableAbilityData* Data = CustomCast<FRecordedCombinableAbilityData>(InRecordedData.Get());
	if (!Data || !Data->IsPayloadValid()) { return false; }

	const FRecordedCombinableAbilityDataPayloadBase& Payload = Data->Payload;

	if (GetCombinedAbilityIDList(*this) != Payload.AbilityIDList) { return false; }
	
	if (const FGameplayAbilitySpec* Spec{ Payload.AbilityComponent->FindAbilitySpecFromClass(Payload.AbilityClass) })
	{
		TOptional<FGameplayEventData> EventData;
		if (Payload.EventDataToBoundAbility) { EventData = Payload.EventDataToBoundAbility->Pin(); }

		NotifyStartDurativeAction(InRecordedData);
		const bool bActivateAbilitySuccessful{ Payload.AbilityComponent->TriggerAbilityFromGameplayEvent(
			Spec->Handle, 
			Payload.AbilityComponent->AbilityActorInfo.Get(), 
			Payload.AbilityTriggerTag.Get(FGameplayTag::EmptyTag), 
			EventData.GetPtrOrNull(),
			*Payload.AbilityComponent
		) };

		if (!bActivateAbilitySuccessful)
		{
			NotifyEndDurativeAction();
		}

		return bActivateAbilitySuccessful;
	}

	return false;
}

void UMyGameplayAbilityBase::PreRecord()
{
	Super::PreRecord();
}

void UMyGameplayAbilityBase::OnPreview(const bool bIsPreview, const IRecordedDataObjectInterface* InRecordedData)
{
	const FRecordedCombinableAbilityData* Data = CustomCast<FRecordedCombinableAbilityData>(InRecordedData);
	if (!Data || !Data->IsPayloadValid() || Data->Payload.AbilityIDList != GetCombinedAbilityIDList(*this)) { return; }

	OnPreview_Internal(bIsPreview, *Data);

}

void UMyGameplayAbilityBase::OnPreview_Internal(const bool bIsPreview, const FRecordedCombinableAbilityData& InRecordedData)
{
	if (!BoundAbilityInfo.IsValid()) { return; }
	K2_OnPreview(bIsPreview);

	for (FCombinedAbilityIterator It{ *BoundAbilityInfo.Ability }; It; ++It) { (*It)->OnPreview_Internal(bIsPreview, InRecordedData); }
}

FRecordedCombinableAbilityData UMyGameplayAbilityBase::MakeRecordedCombinableAbilityData()
{
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent) { return {}; }

	FRecordedCombinableAbilityData Data;
	FRecordedCombinableAbilityDataPayloadBase& Payload = Data.Payload;
	Payload.AbilityComponent = AbilitySystemComponent;
	Payload.AbilityTriggerTag = CombinableAbilityData.EventDataToBoundAbility->EventTag;
	Payload.AbilityClass = GetClass();
	Payload.EventDataToBoundAbility = MoveTemp(CombinableAbilityData.EventDataToBoundAbility);
	Payload.AbilityIDList = GetCombinedAbilityIDList(*this);
	if (BoundAbilityInfo.IsValid())
	{
		Payload.RedoBindCallback = BoundAbilityInfo.RedoBindCallback;
	}
	Payload.PreviewCallback = [WeakThis = MakeWeakObjectPtr(this)](bool bIsPreview, const IRecordedDataObjectInterface* Data) {
		if (!WeakThis.IsValid()) { return; }

		WeakThis->OnPreview(bIsPreview, Data);
		};

	return Data;
}

bool UMyGameplayAbilityBase::ShouldRecord_Implementation() const
{
	URewindSubsystem* Subsystem = URewindSystemStatics::GetRewindSubsystem(this);
	if (!Subsystem) { return false; }

	return Subsystem->GetCurrentState() == ERecordState::Recording;
}

bool UMyGameplayAbilityBase::ShouldStopWhenFailToHandleRecordedData_Implementation() const
{
	return false;
}

const UGameplayEffect* UMyGameplayAbilityBase::GetReverseCostGameplayEffect_Implementation() const
{
	if (ReverseCostGameplayEffectClass) { return ReverseCostGameplayEffectClass->GetDefaultObject<UGameplayEffect>(); }
	else { return nullptr; }
}

TSoftObjectPtr<UTexture2D> UMyGameplayAbilityBase::GetItemIcon_Implementation(const UAbilitySystemComponent* ASC) const
{
	if (ASC == GetAbilitySystemComponentFromActorInfo()) { return Icon; }

	if (!BoundAbilityInfo.IsValid()) { return { nullptr }; }

	return IAbilityIconProviderInterface::Execute_GetItemIcon(BoundAbilityInfo.Ability.Get(), ASC);
}

TArray<AActor*> UMyGameplayAbilityBase::GetCombinedAbilityAvatarActorList() const
{
	TArray<AActor*> AvatarActors;
	for (FCombinedAbilityConstIterator It{ *this }; It; ++It)
	{
		AvatarActors.Add((*It)->GetAvatarActorFromActorInfo());
	}

	return AvatarActors;
}

void UMyGameplayAbilityBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitCheck(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	ApplyCost(Handle, ActorInfo, ActivationInfo);

	//if (!ensureAlwaysMsgf(CombinableAbilityData.IsNull(), TEXT("Memory does not reset!")))
	//{
	//	UE_LOG(LogTemp, Error, TEXT("Memory does not reset %s"), *GetNameSafe(this));
	//	CombinableAbilityData.Reset();
	//}
	if (TriggerEventData)
	{
		CombinableAbilityData.EventDataToBoundAbility = MakeUnique<FGameplayEventWeakData>(FGameplayEventWeakData{ *TriggerEventData });
	}

	if (!IRecordableInterface::Execute_ShouldRecord(this))
	{
		Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

		UE_LOG(LogTemp, Display, TEXT("Activate ability %s TriggerEventData"), TriggerEventData ? TEXT("With") : TEXT("WithOut"));
	}
	else
	{
		if (!ensureAlways(TriggerEventData)) { return; }
		PreRecord();
		K2_PreRecord(*TriggerEventData);
		if (bIsAutoRecordWhenShouldRecord)
		{
			Record(TriggerEventData);
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
			return;
		}
	}

	//EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UMyGameplayAbilityBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	ApplyCooldown(Handle, ActorInfo, ActivationInfo);

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	//CombinableAbilityData.Reset();

	//ReleaseRecordedData();
}

void UMyGameplayAbilityBase::ExecuteBindAbility()
{
	if (!IsBound())
	{
#if WITH_EDITOR
		HandleFailToExecuteBindAbility(TEXT("Ability not bound"));
#endif

		return;
	}

	//if (!BoundAbilityInfo.IsValid())
	if (!IsBound())
	{
#if WITH_EDITOR
		HandleFailToExecuteBindAbility(TEXT("BoundAbilityInfo is invalid"));
#endif

		return;
	}

	UAbilitySystemComponent* ASC = BoundAbilityInfo.AbilitySystemComponent.Get();
	if (!ASC)
	{
#if WITH_EDITOR
		HandleFailToExecuteBindAbility(TEXT("fail to get ASC of bound ability"));
#endif
		return;
	}

	PassRecordedToBoundAbility();

	{
		TUniquePtr<FGameplayEventWeakData> EventData{ MoveTemp(CombinableAbilityData.EventDataToBoundAbility) };

		FGameplayEventData TriggerEventData{ EventData ? EventData->Pin() : FGameplayEventData{} };
		K2_ExecuteBindAbility(TriggerEventData);

		ASC->HandleGameplayEvent(BindTag, &TriggerEventData);
	}

	PostExecuteBindAbility();
}

#if WITH_EDITOR
void UMyGameplayAbilityBase::HandleFailToExecuteBindAbility(const FString& InReason)
{
	UE_LOG(LogTemp, Error, TEXT("Fail to execute bound ability: %s"), *InReason);

	K2_EndAbility();
}
#endif

//bool UMyGameplayAbilityBase::PrepareToHandleRecordedData(IRecordedDataObjectInterface* InRecordedData)
//{
//	FRecordedCombinableAbilityData* Data = CustomCast<FRecordedCombinableAbilityData>(InRecordedData);
//	if (!Data || !Data->Payload.RedoBindCallback.IsSet()) { return false; }
//
//	Data->Payload.RedoBindCallback();
//	return true;
//}

TArray<GameplayAbilityIDType> UMyGameplayAbilityBase::GetCombinedAbilityIDList(const UMyGameplayAbilityBase& FirstAbility)
{
	//if (!FirstAbility.IsFirstAbilityOfBindAbility()) { return {}; }

	//TArray<GameplayAbilityIDType> IDList;
	//for (const UMyGameplayAbilityBase* CurrentAbility{ &FirstAbility }; CurrentAbility; CurrentAbility = CurrentAbility->BoundAbilityInfo.Ability.Get())
	//{
	//	ensureAlwaysMsgf(CurrentAbility->AbilityID.IsSet(), TEXT("Ability %s is not set"), *GetNameSafe(CurrentAbility));

	//	IDList.Add(CurrentAbility->AbilityID.GetValue());
	//}


	TArray<GameplayAbilityIDType> IDList;
	for (FCombinedAbilityConstIterator It{ FirstAbility }; It; ++It)
	{
		const UMyGameplayAbilityBase* CurrentAbility{ (*It) };

		if (!ensureAlwaysMsgf(CurrentAbility->AbilityID.IsSet(), TEXT("Ability %s is not set"), *GetNameSafe(CurrentAbility))) { return {}; }
		IDList.Add(CurrentAbility->AbilityID.GetValue());
	}

	return IDList;
}

//TArray<FShowWillExecuteOperationDelegate> UMyGameplayAbilityBase::GetPreviewAbilityDelegateList(const UMyGameplayAbilityBase& FirstAbility)
//{
//	TArray<FShowWillExecuteOperationDelegate> DelegateList;
//
//	for (const UMyGameplayAbilityBase* CurrentAbility{ &FirstAbility }; CurrentAbility; CurrentAbility = CurrentAbility->BoundAbilityInfo.Ability.Get())
//	{
//		DelegateList.Add(CurrentAbility->ShowWillExecuteOperationDelegate);
//	}
//
//	return DelegateList;
//}

void UMyGameplayAbilityBase::PassRecordedToBoundAbility()
{
	if (IsBound())
	{
		BoundAbilityInfo.Ability->RecordedData = RecordedData;
	}
}

void UMyGameplayAbilityBase::K2_NotifyEndDurativeAction()
{
	NotifyEndDurativeAction();
}

//void UMyGameplayAbilityBase::K2_SetShowWillExecuteOperationDelegate(const FShowWillExecuteOperationDelegate& InDelegate)
//{
//	ShowWillExecuteOperationDelegate = InDelegate;
//}

//void UMyGameplayAbilityBase::EndAbilityWithCancelState(const bool bWasCancelled)
//{
//	constexpr bool bReplicateEndAbility{ true };
//	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicateEndAbility, bWasCancelled);
//}

bool UMyGameplayAbilityBase::GetGameplayAbilityID(GameplayAbilityIDType& ID, const UAbilitySystemComponent& ASC, const UMyGameplayAbilityBase* GA)
{
	if (!GA) { return false; }

	uint32 ASCHash{ GetTypeHash(&ASC) };
	uint32 GAHash{ GetTypeHash(GA) };
	using ASCHashType = decltype(ASCHash);

#if !UE_BUILD_SHIPPING
#include <type_traits>
	using GAHashType = decltype(GAHash);
	static_assert(std::is_same_v<uint32, ASCHashType> && std::is_same_v<ASCHashType, GAHashType>);
#endif

	uint64 Key = (static_cast<uint64>(ASCHash) << 32) & static_cast<uint64>(GAHash);

	static TMap<uint64, GameplayAbilityIDType> UsedIDMap;
	if (GameplayAbilityIDType* IDPtr = UsedIDMap.Find(Key))
	{
		ID = *IDPtr;
	}
	else
	{
		static GameplayAbilityIDType NextID{ 1 };
		ID = NextID++;
	}
	UE_LOG(LogTemp, Error, TEXT("ASCHash: %u"), ASCHash);
	UE_LOG(LogTemp, Error, TEXT("GAHash: %u"), GAHash);
	UE_LOG(LogTemp, Error, TEXT("Key: %u"), Key);
	UE_LOG(LogTemp, Error, TEXT("ID: %u"), ID);

	return true;
}