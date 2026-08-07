// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacterBase.h"
//#include "AbilitySystemComponent.h"
#include "Ability/AbilitySystemComponent/MyAbilitySystemComponent.h"
#include "Switchable/SwitchableActorCollection.h"
#include "WeaponBase/WeaponActorBase.h"
#include "Delegates/DelegateCombinations.h"
#include "Ability/AttributeSet/MyAttributeSet.h"
#include "BlueprintFunctionLibrary/WeaponActorBlueprintLibrary.h"
#include "GameplayEffectTypes.h"
#include "Battle/BattleSubsystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/CharacterWidgetTypes.h"
#include "Character/CharacterWidgetComponent.h"
#include "BehaviorTree/BehaviorTreeStatics.h"
#include "WorldPauseSubsystem.h"

// Sets default values
AMyCharacterBase::AMyCharacterBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	CreateAndSetupComponents();

	AttributeSet = CreateDefaultSubobject<UMyAttributeSet>("AttributeSet");
}

// Called when the game starts or when spawned
void AMyCharacterBase::BeginPlay()
{
	InitializeDelegates();
	Weapons->SetOwner(this);
	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	Super::BeginPlay();



	if (UAIPerceptionComponent * Component{ GetController() ? GetController()->FindComponentByClass<UAIPerceptionComponent>() : nullptr })
	{
		Component->OnTargetPerceptionUpdated.AddDynamic(this, &AMyCharacterBase::OnSenseUpdated);

		if (UBattleSubsystem* BattleSubsystem = ULocalPlayer::GetSubsystem<UBattleSubsystem>(GetWorld()->GetFirstLocalPlayerFromController()))
		{
			BattleSubsystem->RegisterToBattleSubsystem(this);
		}
	}
}

void AMyCharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	DeinitializeDelegates();

	if (UAIPerceptionComponent * Component{ GetController() ? GetController()->FindComponentByClass<UAIPerceptionComponent>() : nullptr })
	{
		Component->OnTargetPerceptionUpdated.RemoveAll(this);
	}

	if (UBattleSubsystem* BattleSubsystem = ULocalPlayer::GetSubsystem<UBattleSubsystem>(GetWorld()->GetFirstLocalPlayerFromController()))
	{
		BattleSubsystem->UnregisterToBattleSubsystem(this);
	}
}

void AMyCharacterBase::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();

	UWorldPauseSubsystem* WorldPauseSubsystem{ GetWorld()->GetSubsystem<UWorldPauseSubsystem>() };
	if (!WorldPauseSubsystem) { return; }

	WorldPauseSubsystem->OnFreezableObjectRegistered(this);
}

void AMyCharacterBase::PostUnregisterAllComponents()
{
	Super::PostUnregisterAllComponents();

	UWorldPauseSubsystem* WorldPauseSubsystem{ GetWorld()->GetSubsystem<UWorldPauseSubsystem>() };
	if (!WorldPauseSubsystem) { return; }

	WorldPauseSubsystem->OnFreezableObjectUnregistered(this);
}

UAbilitySystemComponent* AMyCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

// Called to bind functionality to input
void AMyCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

AWeaponActorBase* AMyCharacterBase::GetControlledWeapon() const
{
	return Cast<AWeaponActorBase>(Weapons->GetControlledObject());
}

void AMyCharacterBase::GetActorEyesViewPoint(FVector& Location, FRotator& Rotation) const
{
	Location = GetActorLocation();
	Rotation = GetActorRotation();
}

void AMyCharacterBase::OnEnemySensed_Implementation(AMyCharacterBase* Enemy)
{
	if (!Enemy || GetTeamAttitudeTowards(*Enemy) != ETeamAttitude::Type::Hostile) { return; }

	UBlackboardComponent* BlackboardComponent = GetController() ? GetController()->FindComponentByClass<UBlackboardComponent>() : nullptr;
	if (!BlackboardComponent) { BlackboardComponent = FindComponentByClass<UBlackboardComponent>(); }
	if (!BlackboardComponent) { return; }

	const FName EnemyCharacterKey{ BattleSubsystemConst::BlackboardKeyName::EnemyCharacter };
	const FName SensedCharacterKey{ BattleSubsystemConst::BlackboardKeyName::SensedCharacter };

	const bool bIsEnemyNull{ BlackboardComponent->GetValueAsObject(EnemyCharacterKey) == nullptr };
	const bool bIsSensedNull{ BlackboardComponent->GetValueAsObject(SensedCharacterKey) == nullptr };

	if (bIsEnemyNull)
	{
		if (const UAIPerceptionComponent * Component{ GetController() ? GetController()->FindComponentByClass<UAIPerceptionComponent>() : nullptr })
		{
			TArray<AActor*> SightSensedActors;
			Component->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), SightSensedActors);

			AActor** SightSensedEnemy = SightSensedActors.FindByPredicate([this](const AActor* Actor) {return Actor && GetTeamAttitudeTowards(*Actor) == ETeamAttitude::Type::Hostile; });

			if (SightSensedEnemy) { BlackboardComponent->SetValueAsObject(EnemyCharacterKey, *SightSensedEnemy); }

			//if (!SightSensedActors.IsEmpty()) { BlackboardComponent->SetValueAsObject(EnemyCharacterKey, SightSensedActors[0]); }
			//else { BlackboardComponent->SetValueAsObject(EnemyCharacterKey, Enemy); }
		}
	}

	if (bIsSensedNull) { BlackboardComponent->SetValueAsObject(SensedCharacterKey, Enemy); }
}

void AMyCharacterBase::OnEnemyDisappear_Implementation(const AMyCharacterBase* Enemy)
{
	UBlackboardComponent* BlackboardComponent = GetController() ? GetController()->FindComponentByClass<UBlackboardComponent>() : nullptr;
	if (!BlackboardComponent) { BlackboardComponent = FindComponentByClass<UBlackboardComponent>(); }
	if (!BlackboardComponent) { return; }

	const FName EnemyCharacterKey{ BattleSubsystemConst::BlackboardKeyName::EnemyCharacter };
	const FName SensedCharacterKey{ BattleSubsystemConst::BlackboardKeyName::SensedCharacter };

	const bool bEnemyDisappeared{ BlackboardComponent->GetValueAsObject(EnemyCharacterKey) == Enemy };
	const bool bSensedDisappeared{ BlackboardComponent->GetValueAsObject(SensedCharacterKey) == Enemy };

	if (!bEnemyDisappeared && !bSensedDisappeared) { return; }

	UBattleSubsystem* BattleSubsystem = ULocalPlayer::GetSubsystem<UBattleSubsystem>(GetWorld()->GetFirstLocalPlayerFromController());

	if (AMyCharacterBase * NextSensed{ BattleSubsystem ? Cast<AMyCharacterBase>(BattleSubsystem->GetOneTeamSensedActor(TeamID.GetId())) : nullptr })
	{
		if (bEnemyDisappeared)
		{
			if (const UAIPerceptionComponent * Component{ GetController() ? GetController()->FindComponentByClass<UAIPerceptionComponent>() : nullptr })
			{
				TArray<AActor*> SightSensedActors;
				Component->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), SightSensedActors);

				if (SightSensedActors.IsEmpty()) { BlackboardComponent->ClearValue(EnemyCharacterKey); }
				else { BlackboardComponent->SetValueAsObject(EnemyCharacterKey, NextSensed); }
			}
		}

		if (bSensedDisappeared && NextSensed) { BlackboardComponent->SetValueAsObject(EnemyCharacterKey, NextSensed); }
		else { BlackboardComponent->ClearValue(SensedCharacterKey); }
	}
	else
	{
		if (bEnemyDisappeared) { BlackboardComponent->ClearValue(EnemyCharacterKey); }
		if (bSensedDisappeared) { BlackboardComponent->ClearValue(SensedCharacterKey); }
	}
}

TArray<AWeaponActorBase*> AMyCharacterBase::GetWeapons() const
{
	TArray<AWeaponActorBase*> OutWeapons;

	for (auto It{ Weapons->CreateIterator<AWeaponActorBase>() }; It; ++It)
	{
		OutWeapons.Add(*It);
	}

	return OutWeapons;
}

AWeaponActorBase* AMyCharacterBase::SwitchWeaponByIndex(const int32 Index)
{
	return Cast<AWeaponActorBase>(Weapons->SwitchActorByIndex(Index));
}

AWeaponActorBase* AMyCharacterBase::SwitchWeaponByActor(AWeaponActorBase* Weapon)
{
	return Cast<AWeaponActorBase>(Weapons->SwitchActor(Weapon));
}

void AMyCharacterBase::ShowCharacterWidget_Implementation(bool bIsShow)
{
	const bool bIsVisible{ ICharacterWidgetControllableInterface::Execute_IsCharacterWidgetVisible(this) };

	CharacterWidgetComponent->ShowCharacterUI(bIsShow);

	if (bIsShow != bIsVisible) { UpdateCharacterWidget(); }
}

bool AMyCharacterBase::IsCharacterWidgetVisible_Implementation() const
{
	return CharacterWidgetComponent->GetWidget() && CharacterWidgetComponent->GetWidget()->IsVisible();
}

void AMyCharacterBase::UpdateCharacterWidget()
{
	if (!ICharacterWidgetControllableInterface::Execute_IsCharacterWidgetVisible(this))
	{
		CharacterWidgetComponent->UpdateUICharacterInfo({});

		return;
	}

	if (const AWeaponActorBase * ControlledWeapon{ GetControlledWeapon() })
	{
		FUICharacterInfo UICharacterInfo{ ControlledWeapon->GetWeaponIcon(), ControlledWeapon->GetWeaponDescription() };
		CharacterWidgetComponent->UpdateUICharacterInfo(UICharacterInfo);
	}
	else { CharacterWidgetComponent->UpdateUICharacterInfo({}); }
}

float AMyCharacterBase::GetCurrentHealth() const
{
	bool bFound{ false };
	const float CurrentHealth{ AbilitySystemComponent->GetGameplayAttributeValue(UMyAttributeSet::GetCurrentHealthAttribute(), bFound) };

	return bFound ? CurrentHealth : 0.f;
}

float AMyCharacterBase::GetHealthMax() const
{
	bool bFound{ false };
	const float HealthMax{ AbilitySystemComponent->GetGameplayAttributeValue(UMyAttributeSet::GetMaxHealthAttribute(), bFound) };

	return bFound ? HealthMax : 0.f;
}

void AMyCharacterBase::OnHovered_Implementation(float HoveredDelta)
{
	if (HoveredDelta > 1.f) { ICharacterWidgetControllableInterface::Execute_ShowCharacterWidget(this, true); }
}

void AMyCharacterBase::OnHoverReleased_Implementation()
{
	ICharacterWidgetControllableInterface::Execute_ShowCharacterWidget(this, false);
}

void AMyCharacterBase::Freeze_Implementation()
{
	CustomTimeDilation = 0.f;
	bIsFreezing = true;
}

void AMyCharacterBase::Unfreeze_Implementation()
{
	CustomTimeDilation = 1.f;
	bIsFreezing = false;
}

void AMyCharacterBase::OnWeaponMagazineAmmoChanged_Implementation(const AWeaponActorBase* Weapon, int32 OldMagazineAmmo, int32 NewMagazineAmmo)
{
	if (!Weapon) { return; }

	FGameplayAttribute MagazineAmmoAttribute, ReserveAmmoAttribute;
	if (!UWeaponActorBlueprintLibrary::GetWeaponAttributeFromAttributeSet(MagazineAmmoAttribute, ReserveAmmoAttribute, Weapon)) { return; }

	AbilitySystemComponent->SetNumericAttributeBase(MagazineAmmoAttribute, FMath::Max(0, NewMagazineAmmo));
}

void AMyCharacterBase::OnWeaponReserveAmmoChanged_Implementation(const AWeaponActorBase* Weapon, int32 OldReserveAmmo, int32 NewReserveAmmo)
{
	if (!Weapon) { return; }

	FGameplayAttribute MagazineAmmoAttribute, ReserveAmmoAttribute;
	if (!UWeaponActorBlueprintLibrary::GetWeaponAttributeFromAttributeSet(MagazineAmmoAttribute, ReserveAmmoAttribute, Weapon)) { return; }

	AbilitySystemComponent->SetNumericAttributeBase(ReserveAmmoAttribute, FMath::Max(0, NewReserveAmmo));
}

void AMyCharacterBase::OnWeaponAdded_Implementation(AWeaponActorBase* AddedWeapon)
{
	if (!AddedWeapon) { return; }

	OnWeaponAddedDelegate.Broadcast(AddedWeapon);

	AddedWeapon->SetWeaponLevel(Level);

	FGameplayAttribute MagazineAmmoAttribute, ReserveAmmoAttribute;
	if (!ensure(UWeaponActorBlueprintLibrary::GetWeaponAttributeFromAttributeSet(MagazineAmmoAttribute, ReserveAmmoAttribute, AddedWeapon))) { return; }

	const FWeaponAttributeData WeaponAttribute{ AddedWeapon->GetWeaponAttribute() };

	AbilitySystemComponent->SetNumericAttributeBase(MagazineAmmoAttribute, FMath::Max(0, WeaponAttribute.MagazineAmmo));
	AbilitySystemComponent->SetNumericAttributeBase(ReserveAmmoAttribute, FMath::Max(0, WeaponAttribute.ReserveAmmo));


}

void AMyCharacterBase::OnWeaponAdded_Internal(AActor* AddedActor)
{
	if (AWeaponActorBase* Weapon = Cast<AWeaponActorBase>(AddedActor))
	{
		OnWeaponAdded(Weapon);
		//Weapon->OnMagazineAmmoChangedDelegate.AddDynamic(this, &AMyCharacterBase::OnWeaponMagazineAmmoChanged);
		//Weapon->OnReserveAmmoChangedDelegate.AddDynamic(this, &AMyCharacterBase::OnWeaponReserveAmmoChanged);

		UBehaviorTreeStatics::SetWeaponMagazineAmmo(this, Weapon->GetMagazineAmmo(), Weapon->GetWeaponSlot());
		UBehaviorTreeStatics::SetWeaponMaxMagazineAmmo(this, Weapon->GetMagazineAmmoMax(), Weapon->GetWeaponSlot());
	}

	UpdateCharacterWidget();
}

void AMyCharacterBase::OnWeaponRemoved_Implementation(AWeaponActorBase* RemovedWeapon)
{
	if (!RemovedWeapon) { return; }

	OnWeaponRemovedDelegate.Broadcast(RemovedWeapon);

	RemovedWeapon->SetWeaponLevel(Level);

	FGameplayAttribute MagazineAmmoAttribute, ReserveAmmoAttribute;
	if (!ensure(UWeaponActorBlueprintLibrary::GetWeaponAttributeFromAttributeSet(MagazineAmmoAttribute, ReserveAmmoAttribute, RemovedWeapon))) { return; }

	const FWeaponAttributeData WeaponAttribute{ RemovedWeapon->GetWeaponAttribute() };

	AbilitySystemComponent->SetNumericAttributeBase(MagazineAmmoAttribute, 0);
	AbilitySystemComponent->SetNumericAttributeBase(ReserveAmmoAttribute, 0);
}

void AMyCharacterBase::OnWeaponRemoved_Internal(AActor* RemovedWeapon)
{
	if (AWeaponActorBase* Weapon = Cast<AWeaponActorBase>(RemovedWeapon))
	{
		OnWeaponRemoved(Weapon);
		Weapon->OnMagazineAmmoChangedDelegate.RemoveAll(this);
		Weapon->OnReserveAmmoChangedDelegate.RemoveAll(this);
	}

	UpdateCharacterWidget();
}

void AMyCharacterBase::OnControlledWeaponChanged_Implementation(AWeaponActorBase* OldWeapon, AWeaponActorBase* NewWeapon)
{
	if (NewWeapon) { WeaponType = NewWeapon->GetWeaponType(); }

	UpdateCharacterWidget();
}

void AMyCharacterBase::OnControlledWeaponChanged_Internal(AActor* OldActor, AActor* NewActor)
{
	AWeaponActorBase* OldWeapon{ Cast<AWeaponActorBase>(OldActor) };
	AWeaponActorBase* NewWeapon{ Cast<AWeaponActorBase>(NewActor) };

	OnControlledWeaponChanged(OldWeapon, NewWeapon);
	OnControlledWeaponChangedDelegate.Broadcast(OldWeapon, NewWeapon);

	if (NewWeapon)
	{
		UBehaviorTreeStatics::SetControlledWeaponSlot(NewWeapon->GetWeaponSlot(), this);
		UBehaviorTreeStatics::SetWeaponMagazineAmmo(this, NewWeapon->GetMagazineAmmo(), NewWeapon->GetWeaponSlot());
		UBehaviorTreeStatics::SetWeaponMaxMagazineAmmo(this, NewWeapon->GetMagazineAmmoMax(), NewWeapon->GetWeaponSlot());
	}

	if (OldWeapon)
	{
		UBehaviorTreeStatics::SetWeaponMagazineAmmo(this, OldWeapon->GetMagazineAmmo(), OldWeapon->GetWeaponSlot());
		UBehaviorTreeStatics::SetWeaponMaxMagazineAmmo(this, OldWeapon->GetMagazineAmmoMax(), OldWeapon->GetWeaponSlot());
	}
}

void AMyCharacterBase::OnWeaponAmmoChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	//AbilitySystemComponent->SetNumericAttributeBase(OnAttributeChangeData.Attribute, OnAttributeChangeData.NewValue);

	EWeaponSlot WeaponSlot;
	if (!UWeaponActorBlueprintLibrary::GetWeaponSlotByWeaponAttribute(WeaponSlot, OnAttributeChangeData.Attribute)) { return; }
	CurrentTargetWeaponSlot = WeaponSlot;

	{
		FPredicateFunction PredicateFunction;
		PredicateFunction.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(AMyCharacterBase, FindWeaponByPredicate));

		if (AWeaponActorBase* Weapon = Cast<AWeaponActorBase>(Weapons->GetActorByPredicate(PredicateFunction)))
		{
			ensure(UWeaponActorBlueprintLibrary::SetWeaponAmmoByAttribute(*Weapon, static_cast<int32>(OnAttributeChangeData.NewValue), OnAttributeChangeData.Attribute));

			if (Weapon == GetControlledWeapon())
			{
				const FGameplayAttribute& Attribute{ OnAttributeChangeData.Attribute };
				const float OldValue{ OnAttributeChangeData.OldValue };
				const float NewValue{ OnAttributeChangeData.NewValue };

				if (UMyAttributeSet::IsMagazineAmmoAttribute(Attribute)) { OnControlledWeaponMagazineAmmoChangedDelegate.Broadcast(OldValue, NewValue); }
				else if (UMyAttributeSet::IsReserveAmmoAttribute(Attribute)) { OnControlledWeaponReserveAmmoChangedDelegate.Broadcast(OldValue, NewValue); }
			}

			UBehaviorTreeStatics::SetWeaponMagazineAmmo(this, Weapon->GetMagazineAmmo(), Weapon->GetWeaponSlot());
		}
	}

	UpdateCharacterWidget();
}

void AMyCharacterBase::OnShoot_Implementation()
{
	AWeaponActorBase* Weapon = GetControlledWeapon();
	if (!Weapon) { return; }

	Weapon->OnShoot();
}

void AMyCharacterBase::OnShootStop_Implementation()
{
	AWeaponActorBase* Weapon = GetControlledWeapon();
	if (!Weapon) { return; }

	Weapon->OnShootStop();
}

void AMyCharacterBase::SimulateWeaponTrigger(const bool bIsPress)
{
	if (bIsPress)
	{
		if (AWeaponActorBase* Weapon = GetControlledWeapon())
		{
			if (Weapon->Fire())
			{
				OnShoot();
				if (!Weapon->IsAutomaticWeapon()) { OnShootStop(); }
			}
		}
	}
	else
	{
		OnShootStop();
	}
}

void AMyCharacterBase::InitializeDelegates()
{
	Weapons->OnActorAddedDelegate.AddDynamic(this, &AMyCharacterBase::OnWeaponAdded_Internal);
	Weapons->OnActorRemovedDelegate.AddDynamic(this, &AMyCharacterBase::OnWeaponRemoved_Internal);
	Weapons->OnControlledActorChangedDelegate.AddDynamic(this, &AMyCharacterBase::OnControlledWeaponChanged_Internal);

	if (!ensure(AbilitySystemComponent->GetAttributeSet(UMyAttributeSet::StaticClass()))) { return; }

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMagazineAmmo1Attribute()).AddUObject(this, &AMyCharacterBase::OnWeaponAmmoChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMagazineAmmo2Attribute()).AddUObject(this, &AMyCharacterBase::OnWeaponAmmoChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMagazineAmmo3Attribute()).AddUObject(this, &AMyCharacterBase::OnWeaponAmmoChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMagazineAmmo4Attribute()).AddUObject(this, &AMyCharacterBase::OnWeaponAmmoChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMagazineAmmo5Attribute()).AddUObject(this, &AMyCharacterBase::OnWeaponAmmoChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetReserveAmmo1Attribute()).AddUObject(this, &AMyCharacterBase::OnWeaponAmmoChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetReserveAmmo2Attribute()).AddUObject(this, &AMyCharacterBase::OnWeaponAmmoChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetReserveAmmo3Attribute()).AddUObject(this, &AMyCharacterBase::OnWeaponAmmoChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetReserveAmmo4Attribute()).AddUObject(this, &AMyCharacterBase::OnWeaponAmmoChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetReserveAmmo5Attribute()).AddUObject(this, &AMyCharacterBase::OnWeaponAmmoChanged);

	AbilitySystemComponent->RegisterGameplayTagEvent(PlayerResponseTags::State_Debuff_Stun.GetTag(), EGameplayTagEventType::AnyCountChange).AddUObject(this, &AMyCharacterBase::OnResponseTagCountChanged);
}

void AMyCharacterBase::DeinitializeDelegates()
{
	Weapons->OnActorAddedDelegate.RemoveAll(this);
	Weapons->OnActorRemovedDelegate.RemoveAll(this);
	Weapons->OnControlledObjectChangedDelegate.RemoveAll(this);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMagazineAmmo1Attribute()).RemoveAll(this);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMagazineAmmo2Attribute()).RemoveAll(this);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMagazineAmmo3Attribute()).RemoveAll(this);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMagazineAmmo4Attribute()).RemoveAll(this);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMagazineAmmo5Attribute()).RemoveAll(this);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetReserveAmmo1Attribute()).RemoveAll(this);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetReserveAmmo2Attribute()).RemoveAll(this);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetReserveAmmo3Attribute()).RemoveAll(this);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetReserveAmmo4Attribute()).RemoveAll(this);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetReserveAmmo5Attribute()).RemoveAll(this);
}

bool AMyCharacterBase::FindWeaponByPredicate(const UObject* Object) const
{
	if (!ensure(CurrentTargetWeaponSlot != EWeaponSlot::None)) { return false; }

	if (const AWeaponActorBase* Weapon = Cast<AWeaponActorBase>(Object); Weapon && Weapon->GetWeaponSlot() == CurrentTargetWeaponSlot)
	{
		return true;
	}

	return false;
}

void AMyCharacterBase::OnSenseUpdated(AActor* Enemy, FAIStimulus Stimulus)
{
	AMyCharacterBase* EnemyCharacter{ Cast<AMyCharacterBase>(Enemy) };
	if (!EnemyCharacter || GetTeamAttitudeTowards(*Enemy) != ETeamAttitude::Type::Hostile || Stimulus.Type != UAISense::GetSenseID<UAISense_Sight>()) { return; }

	OnEnemySensed(EnemyCharacter);

	OnSenseUpdatedDelegate.Broadcast(Stimulus.WasSuccessfullySensed(), this, EnemyCharacter);
}

void AMyCharacterBase::OnResponseTagCountChanged(const FGameplayTag Tag, const int32 NewCount)
{
	auto CalculateTagCountChangeType = [](const int32 OldCount, const int32 NewCount)
		{
			if (OldCount < NewCount) { return ETagCountChangeType::Increase; }
			else if (OldCount > NewCount) { return ETagCountChangeType::Decrease; }
			else { return ETagCountChangeType::NoChange; }
		};

	const ETagCountChangeType TagCountChangeType{ CalculateTagCountChangeType(ResponseTagCountMap.FindOrAdd(Tag, 0), NewCount) };

	if (Tag == PlayerResponseTags::State_Debuff_Stun.GetTag()) { OnStunTagCountChanged(TagCountChangeType); }
	else if (Tag == PlayerResponseTags::State_Debuff_Blind.GetTag()) { OnBlindTagCountChanged(TagCountChangeType); }
}

void AMyCharacterBase::OnStunTagCountChanged(const ETagCountChangeType TagCountChangeType)
{
	K2_OnStunTagCountChanged(TagCountChangeType);
}

void AMyCharacterBase::OnBlindTagCountChanged(const ETagCountChangeType TagCountChangeType)
{
	K2_OnBlindTagCountChanged(TagCountChangeType);
}

void AMyCharacterBase::CreateAndSetupComponents()
{
	AbilitySystemComponent = CreateDefaultSubobject<UMyAbilitySystemComponent>(FName("AbilitySystemComponent"));
	AbilitySystemComponent->PrimaryComponentTick.bCanEverTick = false;
	//AbilitySystemComponent->SetIsReplicated(true);
	//AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	CharacterWidgetComponent = CreateDefaultSubobject<UCharacterWidgetComponent>(FName("CharacterWidgetComponent"));
	CharacterWidgetComponent->SetupAttachment(RootComponent);
}
