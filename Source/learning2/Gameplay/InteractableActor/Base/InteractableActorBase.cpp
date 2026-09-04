// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableActorBase.h"
#include "Ability/MyGameplayAbilityType.h"
#include "Components/StaticMeshComponent.h"
#include "Ability/MyGameplayAbilityBase.h"
#include "AbilitySystemGlobals.h"
//#include "AbilitySystemComponent.h"
#include "Ability/AbilitySystemComponent/MyAbilitySystemComponent.h"
//#include "Components/WidgetComponent.h"
#include "Interactable/InteractionOption.h"
#include "Interactive/ActorWidgetComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/BoxComponent.h"
#include "Interactive/ActorWidget.h"
#include "Interactive/ItemIconProviderInterface.h"
#include "Interactable/InteractableTargetUIInterface.h"
#include "TimerManager.h"
#include "WorldPauseSubsystem.h"
#include "InputRecordComponent.h"
#include "RewindSystemStatics.h"

// Sets default values
AInteractableActorBase::AInteractableActorBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	AbilitySystemComponent = CreateDefaultSubobject<UMyAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	RootComponent = BoxComponent;

	WidgetComponent = CreateDefaultSubobject<UActorWidgetComponent>(TEXT("WidgetComponent"));
	WidgetComponent->SetupAttachment(RootComponent);
	//WidgetComponent->SetRelativeLocation(RootComponent->GetRelativeLocation() + FVector{ 0.f, 0.f, 650.f });
}

void AInteractableActorBase::GatherInteractionOptions_Implementation(const FInteractionQuery& InteractionQuery)
{
	OptionsBuilder.ClearAllInvalidOption();

	if (!InteractionQuery.IsValid()) { return; }

	UAbilitySystemComponent* OtherASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(InteractionQuery.RequestingAvatar.Get());
	if (!OtherASC) { return; }

	const ERecordState RecordState{ URewindSystemStatics::GetRewindSubsystemState(this) };

	for (int32 Index{ 0 }; Index < AbilitySystemComponent->GetActivatableAbilities().Num(); Index++)
	{
		const FGameplayAbilitySpec& Spec{ AbilitySystemComponent->GetActivatableAbilities()[Index] };
		UGameplayAbility* Instance{ Spec.GetPrimaryInstance() };
		if (!Instance) { continue; }

		if (UMyGameplayAbilityBase* GA = Cast<UMyGameplayAbilityBase>(Instance))
		{
			if (GA->NeedBound() || GA->NeedBindTo())
			{
				for (const FGameplayAbilitySpec& OtherSpec : OtherASC->GetActivatableAbilities())
				{
					if (UMyGameplayAbilityBase* OtherGA = Cast<UMyGameplayAbilityBase>(OtherSpec.GetPrimaryInstance()))
					{
						if (!GA->IsBound() && GA->CanBindWith(OtherGA))
						{
							BindAbility(*GA, *AbilitySystemComponent, *OtherGA, *OtherASC, InteractionQuery);
						}
					}
				}
			}
			else
			{
				ConstructAndAddOption(*Instance, InteractionQuery, Index);
			}
		}
		else if (RecordState == ERecordState::Idle)
		{
			ConstructAndAddOption(*Instance, InteractionQuery, Index);
		}
	}

	OnInteractionOptionsUpdated();
}

void AInteractableActorBase::ShowOptions_Implementation(const FInteractionQuery& InteractionQuery)
{
	//RequestingAvatar = InteractionQuery.RequestingAvatar;
	if (!InteractionQuery.IsValid() || !IsOverlappingActorByCollisionComponent(InteractionQuery.RequestingAvatar.Get())) { return; }

	CachedInteractionQuery = InteractionQuery;

	if (RequestingAvatar.IsValid() && InteractionQuery.RequestingAvatar == RequestingAvatar) { return; }

	RequestingAvatar = InteractionQuery.RequestingAvatar;

	WidgetComponent->SetVisibility(true);

	ClearAllInactiveOptionDelay(EClearAllInactiveOptionDelay::Immediate);

	IInteractableTargetInterface::Execute_GatherInteractionOptions(this, InteractionQuery);

	{
		AbilitySystemComponent->OnGiveGameplayAbilityDelegate.AddDynamic(this, &AInteractableActorBase::OnGiveAbility);
		AbilitySystemComponent->OnRemoveGameplayAbilityDelegate.AddDynamic(this, &AInteractableActorBase::OnRemoveAbility);

		UAbilitySystemComponent* OtherASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(InteractionQuery.RequestingAvatar.Get());
		if (UMyAbilitySystemComponent* OtherAbilitySystemComponent = Cast<UMyAbilitySystemComponent>(OtherASC))
		{
			OtherAbilitySystemComponent->OnGiveGameplayAbilityDelegate.AddDynamic(this, &AInteractableActorBase::OnGiveAbility);
			OtherAbilitySystemComponent->OnRemoveGameplayAbilityDelegate.AddDynamic(this, &AInteractableActorBase::OnRemoveAbility);
		}
	}

	if (OptionsBuilder.FindByPredicate([](const UInteractionOptionBase* Option) { return Option && Option->IsActivating(); }) != nullptr)
	{
		ClearAllInactiveOptionDelay(EClearAllInactiveOptionDelay::Immediate);
	}
}

void AInteractableActorBase::HideOptions_Implementation()
{
	{
		AbilitySystemComponent->OnGiveGameplayAbilityDelegate.RemoveAll(this);
		AbilitySystemComponent->OnRemoveGameplayAbilityDelegate.RemoveAll(this);

		if (CachedInteractionQuery.IsSet())
		{
			UAbilitySystemComponent* OtherASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(CachedInteractionQuery.GetValue().RequestingAvatar.Get());
			if (UMyAbilitySystemComponent* OtherAbilitySystemComponent = Cast<UMyAbilitySystemComponent>(OtherASC))
			{
				OtherAbilitySystemComponent->OnGiveGameplayAbilityDelegate.RemoveAll(this);
				OtherAbilitySystemComponent->OnRemoveGameplayAbilityDelegate.RemoveAll(this);
			}
		}
	}

	WidgetComponent->UpdateInteractionOptions(TArray<UInteractionOptionBase*>{});
	RequestingAvatar.Reset();
	CachedInteractionQuery.Reset();

	if (OptionsBuilder.IsEmpty())
	{
		WidgetComponent->SetVisibility(false);
	}
}

bool AInteractableActorBase::IsShow_Implementation() const
{
	return WidgetComponent->IsVisible();
}

bool AInteractableActorBase::K2_ActivateOption(int32 Index, FGameplayTag TriggerTag, FGameplayEventData Payload)
{
	return ActivateOption(Index);
}

bool AInteractableActorBase::ActivateOption(int32 Index)
{
	return OptionsBuilder.ActivateOption(Index);
}

bool AInteractableActorBase::CanActivateAbility(const FGameplayAbilitySpec& Spec) const
{
	if (!Spec.Ability) { return false; }

	return Spec.Ability->CanActivateAbility(Spec.Handle, AbilitySystemComponent->AbilityActorInfo.Get());
}

// Called when the game starts or when spawned
void AInteractableActorBase::BeginPlay()
{
	Super::BeginPlay();

	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	InitAbilities();

	InitWidget();
}

void AInteractableActorBase::Freeze_Implementation()
{
	CustomTimeDilation = 0.f;
	bIsFreezing = true;
}

void AInteractableActorBase::Unfreeze_Implementation()
{
	CustomTimeDilation = 1.f;
	bIsFreezing = false;
}
UAbilitySystemComponent* AInteractableActorBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AInteractableActorBase::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();

	UWorldPauseSubsystem* WorldPauseSubsystem{ GetWorld()->GetSubsystem<UWorldPauseSubsystem>() };
	if (!WorldPauseSubsystem) { return; }

	WorldPauseSubsystem->OnFreezableObjectRegistered(this);
}

void AInteractableActorBase::PostUnregisterAllComponents()
{
	Super::PostUnregisterAllComponents();

	UWorldPauseSubsystem* WorldPauseSubsystem{ GetWorld()->GetSubsystem<UWorldPauseSubsystem>() };
	if (!WorldPauseSubsystem) { return; }

	WorldPauseSubsystem->OnFreezableObjectUnregistered(this);
}

void AInteractableActorBase::InitAbilities()
{
	for (const FOptionInfo& AbilityInfo : OptionClasses)
	{
		if (AbilityInfo.IsValid())
		{
			FGameplayAbilitySpecHandle Handle = AbilitySystemComponent->K2_GiveAbility(AbilityInfo.AbilityClass);

			if (AbilityInfo.AbilityClass.GetDefaultObject()->IsA<UMyGameplayAbilityBase>())
			{
				FGameplayAbilitySpec* Spec{ AbilitySystemComponent->FindAbilitySpecFromHandle(Handle) };
				if (UMyGameplayAbilityBase * AbilityInstance{ Spec ? Cast<UMyGameplayAbilityBase>(Spec->GetPrimaryInstance()) : nullptr })
				{
					AbilityInstance->OnAbilityStateChangedDelegate.AddDynamic(this, &AInteractableActorBase::OnAbilityStateChanged);
				}
			}
		}
	}
}

void AInteractableActorBase::OnGiveAbility_Implementation(const FGameplayAbilitySpec& AbilitySpec, const UMyAbilitySystemComponent* InAbilitySystemComponent)
{
	if (!CachedInteractionQuery.IsSet() || !CachedInteractionQuery.GetValue().IsValid()) { return; }
	const int32 OptionIndex{ OptionClasses.AddUnique({ AbilitySpec.Ability->StaticClass() }) };

	UGameplayAbility* Instance = AbilitySpec.GetPrimaryInstance();
	UMyGameplayAbilityBase* GA = Cast<UMyGameplayAbilityBase>(Instance);
	if (!GA)
	{
		if (InAbilitySystemComponent == AbilitySystemComponent)
		{
			ConstructAndAddOption(*Instance, CachedInteractionQuery.GetValue(), OptionIndex);
			OnInteractionOptionsUpdated();
		}
		return;
	}

	UAbilitySystemComponent* OtherASC = AbilitySystemComponent;
	if (InAbilitySystemComponent == AbilitySystemComponent)
	{
		OtherASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(RequestingAvatar.Get());

		GA->OnAbilityStateChangedDelegate.AddDynamic(this, &AInteractableActorBase::OnAbilityStateChanged);
	}

	if (OtherASC)
	{
		for (const FGameplayAbilitySpec& Spec : OtherASC->GetActivatableAbilities())
		{
			UMyGameplayAbilityBase* OtherGA = Cast<UMyGameplayAbilityBase>(Spec.GetPrimaryInstance());
			if (OtherGA && !OtherGA->IsActive() && !OtherGA->IsBound() && GA->CanBindWith(OtherGA))
			{
				BindAbility(*GA, *AbilitySystemComponent, *OtherGA, *OtherASC, CachedInteractionQuery.GetValue());

				break;
			}
		}
	}
}

void AInteractableActorBase::OnRemoveAbility_Implementation(const FGameplayAbilitySpec& AbilitySpec, const UMyAbilitySystemComponent* InAbilitySystemComponent)
{
	UGameplayAbility* Instance = AbilitySpec.GetPrimaryInstance();
	if (const TWeakObjectPtr<UInteractionOptionBase>*Option{ AbilityToOptionMap.Find(Instance) })
	{
		OptionToAbilityIndexMap.Remove(Option->Get());
		OptionsBuilder.RemoveInteractionOption(Option->Get());
		OnInteractionOptionsUpdated();
	}

	AbilityToOptionMap.Remove(Instance);

	if (UMyGameplayAbilityBase * GA{ Cast<UMyGameplayAbilityBase>(Instance) })
	{
		if (GA->IsBound()) { GA->Unbind(); }

		if (InAbilitySystemComponent == AbilitySystemComponent) { GA->OnAbilityStateChangedDelegate.RemoveAll(this); }
	}

}

void AInteractableActorBase::InitWidget()
{
	if (!WidgetComponent) { return; }

	WidgetComponent->SetRelativeLocation(FVector{ .5f, 1.f, 0.f });
	WidgetComponent->SetWorldRotation(FVector::UpVector.Rotation());

	UUserWidget* Widget = WidgetComponent->GetWidget();

	if (Widget && Widget->Implements<UInteractableTargetUIInterface>())
	{
		IInteractableTargetUIInterface::Execute_SetUIOwner(Widget, this);
	}
}

void AInteractableActorBase::BindAbility(UMyGameplayAbilityBase& GA1, UAbilitySystemComponent& ASC1, UMyGameplayAbilityBase& GA2, UAbilitySystemComponent& ASC2, const FInteractionQuery& InteractionQuery)
{
	//if (!ensure(GA1 && ASC1 && GA2 && ASC2 && InteractionQuery.IsValid())) { return; }
	if (!InteractionQuery.IsValid()) { return; }

	FBindAbilityParameter Param1{ GA1 };
	FBindAbilityParameter Param2{ GA2 };

	UInteractionAbilityOption* Option{ NewObject<UInteractionAbilityOption>() };

	FCombinedAbilityHandle Handle = UMyGameplayAbilityBase::BindWith(Param1, Param2,
		[WeakThis = MakeWeakObjectPtr<AInteractableActorBase>(this), InteractionQuery]()
		{
			if (!WeakThis.IsValid()) { return; }

			IInteractableTargetInterface::Execute_GatherInteractionOptions(WeakThis.Get(), InteractionQuery);
		}
	);

	if (Handle.IsValid())
	{
		Handle.GameplayEventData->Instigator = InteractionQuery.RequestingAvatar.Get();
		const TSoftObjectPtr<UTexture2D> Icon{ IAbilityIconProviderInterface::Execute_GetItemIcon(&GA1, &ASC1) };

		if (!Handle.AbilityInstance->PostRecordDelegate.IsBoundToObject(this))
		{
			Handle.AbilityInstance->PostRecordDelegate.AddUObject(this, &AInteractableActorBase::PostAbilityOptionRecorded, MakeWeakObjectPtr(Option), InteractionQuery);
		}

		const int32 Index{ OptionClasses.IndexOfByPredicate([&Handle](const FOptionInfo& Option)
			{ return Option.AbilityClass == Handle.AbilityInstance->StaticClass(); }) };
		ConstructAndAddOption(*Option, MoveTemp(Handle), InteractionQuery, Index, Icon);

		OnInteractionOptionsUpdated();
	}
}

void AInteractableActorBase::OnInteractionOptionsUpdated()
{
	ClearInvalidDataInOptionToAbilityMap();

	RemoveOptionIfSameGroupActivating();

	if (WidgetComponent->IsVisible()) { WidgetComponent->UpdateInteractionOptions(OptionsBuilder.GetOptions()); }
}

void AInteractableActorBase::OnAbilityStateChanged(EActionState OldState, EActionState NewState)
{
	OnInteractionOptionsUpdated();
}

void AInteractableActorBase::PostAbilityOptionRecorded(const FRecordedDataObjectHandle& Handle, TWeakObjectPtr<UInteractionAbilityOption> WeakOption, FInteractionQuery InteractionQuery)
{
	if (!Handle.IsValid() || !WeakOption.IsValid()) { return; }
	
	UInputRecordComponent* InputRecordComponent{ InteractionQuery.RequestingAvatar.IsValid() ? InteractionQuery.RequestingAvatar->FindComponentByClass<UInputRecordComponent>() : nullptr };
	WeakOption->SetRecordedDataObjectHandle(InputRecordComponent, Handle);

	Handle.InputRecordComponent->OnOperationPreviewDelegate.AddWeakLambda(WeakOption.Get(), 
	[WeakThis = MakeWeakObjectPtr(this), WeakOption, Handle](const bool bIsPreview, const IRecordedDataObjectInterface* Data)
		{
			if (Data->Handle != Handle) { return; }

			WeakOption->SetWillBeActivate(bIsPreview);

			if (WeakThis.IsValid()) { WeakThis->ClearAllInactiveOptionDelay(EClearAllInactiveOptionDelay::Immediate); }
		});
}

void AInteractableActorBase::ClearAllInactiveOptionDelay(const EClearAllInactiveOptionDelay ClearAllInactiveOptionDelay)
{
	if (ClearAllInactiveOptionDelay == EClearAllInactiveOptionDelay::Delayed && !GetWorld()->GetTimerManager().IsTimerActive(DelayClearAllInactiveOptionTimerHandle))
	{
		GetWorld()->GetTimerManager().SetTimer(DelayClearAllInactiveOptionTimerHandle, this, &AInteractableActorBase::ClearAllInactiveOptionDelay, 2.f, false);
	}
	else if (ClearAllInactiveOptionDelay == EClearAllInactiveOptionDelay::Immediate)
	{
		if (GetWorld()->GetTimerManager().IsTimerActive(DelayClearAllInactiveOptionTimerHandle))
		{
			GetWorld()->GetTimerManager().ClearTimer(DelayClearAllInactiveOptionTimerHandle);
		}

		for (UInteractionOptionBase* Option : OptionsBuilder.GetOptions())
		{
			if (UInteractionAbilityOption* AbilityOption{ Cast<UInteractionAbilityOption>(Option) }; AbilityOption && !AbilityOption->IsActivating()) 
			{
				if (UMyGameplayAbilityBase* MyGA{ Cast<UMyGameplayAbilityBase>(AbilityOption->GetAbilityInstance()) }) { MyGA->PostRecordDelegate.RemoveAll(this); }
			}
		}

		OptionsBuilder.ClearAllInactiveOption();
		OnInteractionOptionsUpdated();
	}
}

void AInteractableActorBase::ConstructAndAddOption(UGameplayAbility& AbilityInstance, const FInteractionQuery& InteractionQuery, const int32 Index)
{
	UInteractionAbilityOption* Option{ UInteractionAbilityOption::CreateInteractionAbilityOption(&AbilityInstance, GetOptionGroupIDByAbilityInstance(&AbilityInstance), InteractionQuery.RequestingAvatar.Get()) };
	OptionsBuilder.AddInteractionOption(Option);
	OptionToAbilityIndexMap.Add(Option, Index);
	if (UMyGameplayAbilityBase* MyGA{ Cast<UMyGameplayAbilityBase>(&AbilityInstance) }; MyGA && !MyGA->PostRecordDelegate.IsBoundToObject(this))
	{
		MyGA->PostRecordDelegate.AddUObject(this, &AInteractableActorBase::PostAbilityOptionRecorded, MakeWeakObjectPtr(Option), InteractionQuery);
	}
}

void AInteractableActorBase::ConstructAndAddOption(UInteractionAbilityOption& TargetOption, FCombinedAbilityHandle&& Handle, const FInteractionQuery& InteractionQuery, const int32 Index, const TSoftObjectPtr<UTexture2D> InIcon)
{
	UInteractionAbilityOption::CreateInteractionAbilityOption(TargetOption, MoveTemp(Handle), GetOptionGroupIDByAbilityInstance(Handle.AbilityInstance.Get()), InteractionQuery.RequestingAvatar.Get(), InIcon);
	OptionsBuilder.AddInteractionOption(&TargetOption);
	OptionToAbilityIndexMap.Add(&TargetOption, Index);

	if (Handle.IsValid() && !Handle.AbilityInstance->PostRecordDelegate.IsBoundToObject(this))
	{
		Handle.AbilityInstance->PostRecordDelegate.AddUObject(this, &AInteractableActorBase::PostAbilityOptionRecorded, MakeWeakObjectPtr(&TargetOption), InteractionQuery);
	}
}

InteractionOptionTypes::OptionGroupIDType AInteractableActorBase::GetOptionGroupIDByAbilityInstance(const UGameplayAbility* AbilityInstance) const
{
	const TSubclassOf<UGameplayAbility> AbilityClass{ AbilityInstance->StaticClass() };
	const FOptionInfo* TargetOption{ OptionClasses.FindByPredicate([AbilityClass](const FOptionInfo& OptionInfo) {return OptionInfo.AbilityClass == AbilityClass; }) };

	if (TargetOption) { return TargetOption->GroupID; }
	else { return INDEX_NONE; }
}