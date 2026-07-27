// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableActorBase.h"
#include "Components/StaticMeshComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Ability/MyGameplayAbilityBase.h"
#include "AbilitySystemGlobals.h"
//#include "AbilitySystemComponent.h"
#include "Ability/AbilitySystemComponent/MyAbilitySystemComponent.h"
//#include "Components/WidgetComponent.h"
#include "Interactive/ActorWidgetComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/BoxComponent.h"
#include "Interactive/ActorWidget.h"
#include "Interactive/ItemIconProviderInterface.h"
#include "Interactable/InteractableTargetUIInterface.h"
#include "WorldPauseSubsystem.h"
#include "InputRecordComponent.h"

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
	OptionsBuilder.Empty();

	if (!InteractionQuery.IsValid()) { return; }

	UAbilitySystemComponent* OtherASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(InteractionQuery.RequestingAvatar.Get());
	if (!OtherASC) { return; }

	for (const FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
	{
		UGameplayAbility* Instance{ Spec.GetPrimaryInstance() };
		if (!Instance) { continue; }

		if (UMyGameplayAbilityBase* GA = Cast<UMyGameplayAbilityBase>(Instance))
		{
			for (const FGameplayAbilitySpec& OtherSpec : OtherASC->GetActivatableAbilities())
			{
				if (UMyGameplayAbilityBase* OtherGA = Cast<UMyGameplayAbilityBase>(OtherSpec.GetPrimaryInstance()))
				{
					//UE_LOG(LogTemp, Error, TEXT("AWeaponActorBase::OnControl GA %s"), *GetNameSafe(GA));
					//UE_LOG(LogTemp, Error, TEXT("AWeaponActorBase::OnControl OtherGA %s"), *GetNameSafe(OtherGA));
					//UE_LOG(LogTemp, Error, TEXT("AWeaponActorBase::OnControl GA->IsBound() %d"), GA->IsBound());
					//UE_LOG(LogTemp, Error, TEXT("AWeaponActorBase::OnControl GA->CanBindWith(OtherGA) %d"), GA->CanBindWith(OtherGA));
					if (!GA->IsBound() && GA->CanBindWith(OtherGA))
					{
						BindAbility(*GA, *AbilitySystemComponent, *OtherGA, *OtherASC, InteractionQuery);
					}
				}
			}
		}
		else
		{
			OptionsBuilder.AddInteractionOption(UInteractionAbilityOption::CreateInteractionAbilityOption(Instance));
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("OptionsBuilder.Options.Num(): %d"), OptionsBuilder.Options.Num());
	int i = 0;
	for (auto O : OptionsBuilder.Options)
	{
		if (O.IsValid()) { i++; }
	}
	UE_LOG(LogTemp, Warning, TEXT("OptionsBuilder.Options Valid: %d"), i);
}

void AInteractableActorBase::ShowOptions_Implementation(const FInteractionQuery& InteractionQuery)
{
	//RequestingAvatar = InteractionQuery.RequestingAvatar;
	if (!InteractionQuery.IsValid() || !IsOverlappingActorByCollisionComponent(InteractionQuery.RequestingAvatar.Get())) { return; }

	CachedInteractionQuery = InteractionQuery;

	if (RequestingAvatar.IsValid() && InteractionQuery.RequestingAvatar == RequestingAvatar) { return; }

	RequestingAvatar = InteractionQuery.RequestingAvatar;

	IInteractableTargetInterface::Execute_GatherInteractionOptions(this, InteractionQuery);

	WidgetComponent->SetVisibility(true);
	WidgetComponent->UpdateInteractionOptions(OptionsBuilder.GetOptions());

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

	// todo
	//OptionsBuilder.Options.Sort();
}

void AInteractableActorBase::HideOptions_Implementation()
{
	{
		AbilitySystemComponent->OnGiveGameplayAbilityDelegate.RemoveAll(this);
		AbilitySystemComponent->OnRemoveGameplayAbilityDelegate.RemoveAll(this);

		WidgetComponent->UpdateInteractionOptions({});

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

	//OptionsBuilder.Empty();

	RequestingAvatar.Reset();
	CachedInteractionQuery.Reset();

	WidgetComponent->SetVisibility(false);
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
	for (TSubclassOf<UGameplayAbility> AbilityClass : OptionClasses)
	{
		if (AbilityClass)
		{
			FGameplayAbilitySpecHandle Handle = AbilitySystemComponent->K2_GiveAbility(AbilityClass);

			if (AbilityClass.GetDefaultObject() && AbilityClass.GetDefaultObject()->IsA<UMyGameplayAbilityBase>())
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

	UGameplayAbility* Instance = AbilitySpec.GetPrimaryInstance();
	UMyGameplayAbilityBase* GA = Cast<UMyGameplayAbilityBase>(Instance);
	if (!GA)
	{
		if (InAbilitySystemComponent == AbilitySystemComponent)
		{
			OptionsBuilder.AddInteractionOption(UInteractionAbilityOption::CreateInteractionAbilityOption(Instance));
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
		OptionsBuilder.RemoveInteractionOption(Option->Get());
	}


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

	FRecordedDataObjectHandle RecordedDataObjectHandle;

	FCombinedAbilityHandle Handle = UMyGameplayAbilityBase::BindWith(Param1, Param2,
		[WeakThis = MakeWeakObjectPtr<AInteractableActorBase>(this), InteractionQuery]()
		{
			if (!WeakThis.IsValid()) { return; }

			IInteractableTargetInterface::Execute_GatherInteractionOptions(WeakThis.Get(), InteractionQuery);
		},
		FPostRecordCallbackType::CreateWeakLambda(this, [&RecordedDataObjectHandle](FRecordedDataObjectHandle InRecordedDataObjectHandle) {
			RecordedDataObjectHandle = InRecordedDataObjectHandle;
			})
	);

	if (Handle.IsValid())
	{
		Handle.GameplayEventData->Instigator = InteractionQuery.RequestingAvatar.Get();
		const TSoftObjectPtr<UTexture2D> Icon{ IAbilityIconProviderInterface::Execute_GetItemIcon(&GA1, &ASC1) };

		UInteractionAbilityOption* Option{ UInteractionAbilityOption::CreateInteractionAbilityOption(MoveTemp(Handle), Icon) };
		OptionsBuilder.AddInteractionOption(Option);

		if (RecordedDataObjectHandle.IsValid()) { PostAbilityOptionRecorded(Option, RecordedDataObjectHandle); }
	}
}

void AInteractableActorBase::OnAbilityStateChanged(EActionState OldState, EActionState NewState)
{
	WidgetComponent->UpdateInteractionOptions(OptionsBuilder.GetOptions());
}

void AInteractableActorBase::PostAbilityOptionRecorded(UInteractionOptionBase* Option, const FRecordedDataObjectHandle Handle)
{
	if (!Handle.IsValid()) { return; }


	Handle.InputRecordComponent->OnOperationPreviewDelegate.AddWeakLambda(Option, [Option, Handle](const bool bIsPreview, const IRecordedDataObjectInterface* Data)
		{
			if (Data->Handle != Handle) { return; }

			Option->SetWillBeActivate(bIsPreview);
		});
}
//void AInteractableActorBase::OnUIInitialized_Implementation(UUserWidget* UserWidget)
//{
//	ActorWidget
//}

//void AInteractableActorBase::CreateUI()
//{
//	checkf(UIClass, TEXT("Need to specify UserWidget"));
//	UI = CreateWidget<UUserWidget>(GetWorld(), UIClass);
//
//	if (UI->GetClass()->ImplementsInterface(UInteractableTargetUIInterface::StaticClass()))
//	{
//		IInteractableTargetUIInterface::Execute_SetUIOwner(UI, this);
//	}
//}