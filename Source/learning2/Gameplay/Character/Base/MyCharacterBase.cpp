// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacterBase.h"
//#include "AbilitySystemComponent.h"
#include "Ability/AbilitySystemComponent/MyAbilitySystemComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Switchable/SwitchableActorCollection.h"
#include "WeaponBase/WeaponActorBase.h"
#include "Delegates/DelegateCombinations.h"
#include "Ability/AttributeSet/MyAttributeSet.h"
#include "BlueprintFunctionLibrary/WeaponActorBlueprintLibrary.h"
#include "GameplayEffectTypes.h"
#include "Battle/BattleSubsystem.h"
#include "Battle/BattleSubsystemStatics.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/CharacterWidgetTypes.h"
#include "Character/CharacterWidgetComponent.h"
#include "BehaviorTree/BehaviorTreeStatics.h"
#include "WorldPauseSubsystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "FogOfWarSubsystem.h"
#include "FogOfWarComponentStatics.h"
#include "Ability/Tags/PlayerStateGameplayTags.h"
#include "Controller/MyAIController.h"

// Sets default values
AMyCharacterBase::AMyCharacterBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	CreateAndSetupComponents();

	AttributeSet = CreateDefaultSubobject<UMyAttributeSet>("AttributeSet");
}

void AMyCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (NewController->IsA<AAIController>())
	{
		if (UAIBlueprintHelperLibrary::GetBlackboard(this)) { PostBehaviorTreeRun(); }
		else if (AMyAIController* AIController{ Cast<AMyAIController>(NewController) }) 
		{ 
			if (!AIController->PostBehaviorTreeRunDelegate.IsBoundToObject(this))
			{
				AIController->PostBehaviorTreeRunDelegate.AddUObject(this, &AMyCharacterBase::PostBehaviorTreeRun);
			}
		}
	}
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
		Component->OnTargetPerceptionUpdated.AddUniqueDynamic(this, &AMyCharacterBase::OnSenseUpdated);

		if (UBattleSubsystem* BattleSubsystem{ UBattleSubsystemStatics::GetBattleSubsystem(this) })
		{
			BattleSubsystem->RegisterToBattleSubsystem(this);
		}
	}
}

void AMyCharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DeinitializeDelegates();
	
	if (UAIPerceptionComponent * Component{ GetController() ? GetController()->FindComponentByClass<UAIPerceptionComponent>() : nullptr })
	{
		Component->OnTargetPerceptionUpdated.RemoveAll(this);
	}

	if (UBattleSubsystem* BattleSubsystem{ UBattleSubsystemStatics::GetBattleSubsystem(this) })
	{
		BattleSubsystem->UnregisterToBattleSubsystem(this);
	}

	Super::EndPlay(EndPlayReason);
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

void AMyCharacterBase::PostBehaviorTreeRun()
{
	if (GetController() && GetController()->IsA<AAIController>())
	{
		UBehaviorTreeStatics::SetSelfActor(this);

		const TArray<const AWeaponActorBase*> WeaponsList{ GetWeapons() };
		for (int32 i = 0; i < 2; i++)
		{
			if (!WeaponsList.IsValidIndex(i)) { break; }

			const AWeaponActorBase* Weapon{ WeaponsList[i] };

			if (Weapon->IsOnControl()) { UBehaviorTreeStatics::SetControlledWeaponSlot(Weapon->GetWeaponSlot(), this); }
			UBehaviorTreeStatics::SetWeaponMagazineAmmo(this, Weapon->GetMagazineAmmo(), Weapon->GetWeaponSlot());
			UBehaviorTreeStatics::SetWeaponMaxMagazineAmmo(this, Weapon->GetMagazineAmmoMax(), Weapon->GetWeaponSlot());
		}

		ACharacter* EnemyCharacter{ nullptr };
		ACharacter* SensedEnemyCharacter{ nullptr };
		if (UBattleSubsystem* BattleSubsystem{ UBattleSubsystemStatics::GetBattleSubsystem(this) })
		{
			SensedEnemyCharacter = Cast<ACharacter>(BattleSubsystem->GetOneTeamSensedActor(GetGenericTeamId()));

			if (SensedEnemyCharacter)
			{
				FHitResult HitResult;

				const FCollisionObjectQueryParams Params{ ConstructCharacterVisionParam() };
				GetWorld()->LineTraceSingleByObjectType(HitResult, GetActorLocation(), SensedEnemyCharacter->GetActorLocation(), Params);

				if (HitResult.GetActor())
				{
					if (HitResult.GetActor() == SensedEnemyCharacter)
					{
						EnemyCharacter = SensedEnemyCharacter;
					}
					else if (const IGenericTeamAgentInterface * TeamAgent{ Cast<IGenericTeamAgentInterface>(HitResult.GetActor()) })
					{
						if (TeamAgent->GetTeamAttitudeTowards(*this) == ETeamAttitude::Hostile)
						{
							EnemyCharacter = Cast<ACharacter>(HitResult.GetActor());
						}

					}
				}
			}

		}
		else { UBehaviorTreeStatics::SetSensedEnemyCharacter(nullptr, this); }

		UBehaviorTreeStatics::SetEnemyCharacter(EnemyCharacter, this);
		UBehaviorTreeStatics::SetSensedEnemyCharacter(SensedEnemyCharacter, this);
	}
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

			AActor** SightSensedEnemy = SightSensedActors.FindByPredicate([this](const AActor* Actor) 
			{ 
				const AMyCharacterBase* Ch{ Cast<AMyCharacterBase>(Actor) };
				return Ch && GetTeamAttitudeTowards(*Ch) == ETeamAttitude::Type::Hostile && !Ch->IsDead();
			});

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

	const UBattleSubsystem* BattleSubsystem{ UBattleSubsystemStatics::GetBattleSubsystem(this) };
	if (AMyCharacterBase * NextSensed{ BattleSubsystem ? Cast<AMyCharacterBase>(BattleSubsystem->GetOneTeamSensedActor(TeamID.GetId())) : nullptr })
	{
		if (bEnemyDisappeared)
		{
			if (const UAIPerceptionComponent * Component{ GetController() ? GetController()->FindComponentByClass<UAIPerceptionComponent>() : nullptr })
			{
				TArray<AActor*> SightSensedActors;
				Component->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), SightSensedActors);

				if (SightSensedActors.IsEmpty()) { BlackboardComponent->ClearValue(EnemyCharacterKey); }
				else 
				{ 
					BlackboardComponent->SetValueAsObject(EnemyCharacterKey, NextSensed);
					BlackboardComponent->SetValueAsObject(SensedCharacterKey, NextSensed);
				}
			}
		}

		if (bSensedDisappeared && NextSensed) 
		{ 
			// BlackboardComponent->SetValueAsObject(EnemyCharacterKey, NextSensed);
			BlackboardComponent->SetValueAsObject(SensedCharacterKey, NextSensed);
		}
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

bool AMyCharacterBase::ContainsWeapon(const AWeaponActorBase* Weapon) const 
{ 
	return Weapons->Contains(Weapon);
}

AWeaponActorBase* AMyCharacterBase::SwitchWeaponByIndex(const int32 Index)
{
	return Cast<AWeaponActorBase>(Weapons->SwitchActorByIndex(Index));
}

AWeaponActorBase* AMyCharacterBase::SwitchWeaponByActor(const AWeaponActorBase* Weapon)
{
	return Cast<AWeaponActorBase>(Weapons->SwitchActor(Weapon));
}

AWeaponActorBase* AMyCharacterBase::SwitchWeaponByClass(const TSubclassOf<AWeaponActorBase> WeaponClass)
{
	const int32 Index{ Weapons->IndexOfByPredicate([WeaponClass](const UObject* Weapon){ return Weapon->IsA(WeaponClass); }) };
	if (Weapons->IsValidIndex(Index)) { return Cast<AWeaponActorBase>(Weapons->SwitchActorByIndex(Index)); }
	return nullptr;
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

TOptional<float> AMyCharacterBase::GetCurrentHealth() const
{
	bool bFound{ false };
	const float CurrentHealth{ AbilitySystemComponent->GetGameplayAttributeValue(UMyAttributeSet::GetCurrentHealthAttribute(), bFound) };

	return bFound ? TOptional<float>{ CurrentHealth } : NullOpt;
}

TOptional<float> AMyCharacterBase::GetHealthMax() const
{
	bool bFound{ false };
	const float HealthMax{ AbilitySystemComponent->GetGameplayAttributeValue(UMyAttributeSet::GetMaxHealthAttribute(), bFound) };

	return bFound ? TOptional<float>{ HealthMax } : NullOpt;
}

bool AMyCharacterBase::IsDead() const
{
	const TOptional<float> CurrentHealth{ GetCurrentHealth() };
	if (!CurrentHealth.IsSet()) { return false; }

	return FMath::IsNearlyZero(CurrentHealth.GetValue()) || CurrentHealth.GetValue() < 0.f;
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

void AMyCharacterBase::UpdateFogOfWarTexture_Implementation(UTexture2D* FogOfWarTexture)
{
	const EDoInitialize DoInitialize{ bIsFogOfWarMaskInitialized ? EDoInitialize::No : EDoInitialize::Yes };
    UpdateFogOfWarTexture_DefaultImplementation(FogOfWarTexture, GetMesh(), bIsFogOfWarMaskInitialized, DoInitialize);
}

void AMyCharacterBase::EnableTeamDuty(const bool bIsEnable)
{
	if (UBattleSubsystem* BattleSubsystem{ UBattleSubsystemStatics::GetBattleSubsystem(this) }) 
	{ 
		UAIPerceptionComponent* AIPerceptionComponent{ GetController() ? GetController()->FindComponentByClass<UAIPerceptionComponent>() : nullptr };
		
		if (bIsEnable) 
		{ 
			BattleSubsystem->RegisterToBattleSubsystem(this); 

			if (AIPerceptionComponent)
			{
				AIPerceptionComponent->OnTargetPerceptionUpdated.AddUniqueDynamic(this, &AMyCharacterBase::OnSenseUpdated);
			}
		}
		else
		{
			BattleSubsystem->UnregisterToBattleSubsystem(this);

			if (AIPerceptionComponent) { AIPerceptionComponent->OnTargetPerceptionUpdated.RemoveAll(this); }

			if (UBlackboardComponent* BlackboardComponent{ GetController() ? GetController()->FindComponentByClass<UBlackboardComponent>() : nullptr })
			{
				if (!BlackboardComponent) { BlackboardComponent = FindComponentByClass<UBlackboardComponent>(); }
				if (BlackboardComponent)
				{
					BlackboardComponent->ClearValue(BattleSubsystemConst::BlackboardKeyName::EnemyCharacter);
					BlackboardComponent->ClearValue(BattleSubsystemConst::BlackboardKeyName::SensedCharacter);
				}
			}
		}
	}
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

		if (Weapon == GetControlledWeapon()) { UBehaviorTreeStatics::SetControlledWeaponSlot(Weapon->GetWeaponSlot(), this); }
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

void AMyCharacterBase::OnCharacterHealthChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	K2_OnCharacterHealthChanged(OnAttributeChangeData.NewValue);
	if (IsDead()) { OnCharacterDeath(); }
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
	Weapons->OnActorAddedDelegate.AddUniqueDynamic(this, &AMyCharacterBase::OnWeaponAdded_Internal);
	Weapons->OnActorRemovedDelegate.AddUniqueDynamic(this, &AMyCharacterBase::OnWeaponRemoved_Internal);
	Weapons->OnControlledActorChangedDelegate.AddUniqueDynamic(this, &AMyCharacterBase::OnControlledWeaponChanged_Internal);

	if (!ensure(AbilitySystemComponent->GetAttributeSet(UMyAttributeSet::StaticClass()))) { return; }

	if (!AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetCurrentHealthAttribute()).IsBoundToObject(this))
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetCurrentHealthAttribute()).AddUObject(this, &AMyCharacterBase::OnCharacterHealthChanged);
	}
	if (!AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMaxHealthAttribute()).IsBoundToObject(this))
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMaxHealthAttribute()).AddUObject(this, &AMyCharacterBase::OnCharacterHealthChanged);
	}
	if (!AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMagazineAmmo1Attribute()).IsBoundToObject(this))
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMagazineAmmo1Attribute()).AddUObject(this, &AMyCharacterBase::OnWeaponAmmoChanged);
	}
	if (!AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMagazineAmmo2Attribute()).IsBoundToObject(this))
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMagazineAmmo2Attribute()).AddUObject(this, &AMyCharacterBase::OnWeaponAmmoChanged);
	}
	if (!AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMagazineAmmo3Attribute()).IsBoundToObject(this))
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMagazineAmmo3Attribute()).AddUObject(this, &AMyCharacterBase::OnWeaponAmmoChanged);
	}
	if (!AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMagazineAmmo4Attribute()).IsBoundToObject(this))
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMagazineAmmo4Attribute()).AddUObject(this, &AMyCharacterBase::OnWeaponAmmoChanged);
	}
	if (!AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMagazineAmmo5Attribute()).IsBoundToObject(this))
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMagazineAmmo5Attribute()).AddUObject(this, &AMyCharacterBase::OnWeaponAmmoChanged);
	}
	if (!AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetReserveAmmo1Attribute()).IsBoundToObject(this))
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetReserveAmmo1Attribute()).AddUObject(this, &AMyCharacterBase::OnWeaponAmmoChanged);
	}
	if (!AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetReserveAmmo2Attribute()).IsBoundToObject(this))
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetReserveAmmo2Attribute()).AddUObject(this, &AMyCharacterBase::OnWeaponAmmoChanged);
	}
	if (!AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetReserveAmmo3Attribute()).IsBoundToObject(this))
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetReserveAmmo3Attribute()).AddUObject(this, &AMyCharacterBase::OnWeaponAmmoChanged);
	}
	if (!AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetReserveAmmo4Attribute()).IsBoundToObject(this))
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetReserveAmmo4Attribute()).AddUObject(this, &AMyCharacterBase::OnWeaponAmmoChanged);
	}
	if (!AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetReserveAmmo5Attribute()).IsBoundToObject(this))
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetReserveAmmo5Attribute()).AddUObject(this, &AMyCharacterBase::OnWeaponAmmoChanged);
	}

	RegisterGameplayTagEvent();

	if (UFogOfWarSubsystem* FogOfWarSubsystem{ UFogOfWarComponentStatics::GetFogOfWarSubsystem(this) })
	{
		FogOfWarSubsystem->OnFogOfWarTextureUpdatedDelegate.AddUniqueDynamic(this, &AMyCharacterBase::UpdateFogOfWarTexture);
	}
}

void AMyCharacterBase::DeinitializeDelegates()
{
	if (Weapons)
	{
		Weapons->OnActorAddedDelegate.RemoveAll(this);
		Weapons->OnActorRemovedDelegate.RemoveAll(this);
		Weapons->OnControlledObjectChangedDelegate.RemoveAll(this);
	}

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetCurrentHealthAttribute()).RemoveAll(this);
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

	if (UFogOfWarSubsystem* FogOfWarSubsystem{ UFogOfWarComponentStatics::GetFogOfWarSubsystem(this) })
	{
		FogOfWarSubsystem->OnFogOfWarTextureUpdatedDelegate.RemoveAll(this);
	}
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
	if (!EnemyCharacter || EnemyCharacter->IsDead() || GetTeamAttitudeTowards(*Enemy) != ETeamAttitude::Type::Hostile || Stimulus.Type != UAISense::GetSenseID<UAISense_Sight>()) { return; }

	OnEnemySensed(EnemyCharacter);

	OnSenseUpdatedDelegate.Broadcast(Stimulus.WasSuccessfullySensed(), this, EnemyCharacter);
}

void AMyCharacterBase::RegisterGameplayTagEvent()
{
	AbilitySystemComponent->RegisterGameplayTagEvent(PlayerResponseTags::State_Debuff_Stun.GetTag(), EGameplayTagEventType::AnyCountChange).AddUObject(this, &AMyCharacterBase::OnResponseTagCountChanged);
	AbilitySystemComponent->RegisterGameplayTagEvent(PlayerResponseTags::State_Debuff_Blind.GetTag(), EGameplayTagEventType::AnyCountChange).AddUObject(this, &AMyCharacterBase::OnResponseTagCountChanged);
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
	AController* CurrentController{ GetController() };
	if (!CurrentController) { return; }

	CurrentController->SetIgnoreMoveInput(AbilitySystemComponent->HasMatchingGameplayTag(PlayerResponseTags::State_Debuff_Stun.GetTag()));
	EnableTeamDuty(!AbilitySystemComponent->HasMatchingGameplayTag(PlayerResponseTags::State_Debuff_Stun.GetTag()));
	if (UBehaviorTreeComponent* BehaviorTreeComponent{ GetController() ? GetController()->FindComponentByClass<UBehaviorTreeComponent>() : nullptr })
	{
		if (AbilitySystemComponent->HasMatchingGameplayTag(PlayerResponseTags::State_Debuff_Stun.GetTag()))
		{
			BehaviorTreeComponent->StopLogic(TEXT("Stunned"));
		}
		else
		{
			BehaviorTreeComponent->StartLogic();
		}
	}

	K2_OnStunTagCountChanged(TagCountChangeType);
}

void AMyCharacterBase::OnBlindTagCountChanged(const ETagCountChangeType TagCountChangeType)
{
	EnableTeamDuty(!AbilitySystemComponent->HasMatchingGameplayTag(PlayerResponseTags::State_Debuff_Blind.GetTag()));

	K2_OnBlindTagCountChanged(TagCountChangeType);
}

void AMyCharacterBase::OnCharacterDeath()
{
	AbilitySystemComponent->AddLooseGameplayTag(PlayerStateTags::State_Dead.GetTag());
	EnableTeamDuty(false);
	OnCharacterDeath_Internal();

	K2_OnCharacterDeath();
	OnCharacterDeadDelegate.Broadcast(this);
}

void AMyCharacterBase::OnCharacterDeath_Internal()
{
	if (AController* CurrentController{ GetController() }) 
	{ 
		if (UBehaviorTreeComponent* BehaviorTreeComponent{ CurrentController->FindComponentByClass<UBehaviorTreeComponent>() })
		{
			BehaviorTreeComponent->StopLogic(TEXT("CharacterDead"));
		}
		
		if (UAIPerceptionComponent* AIPerceptionComponent{ FindComponentByClass<UAIPerceptionComponent>() })
		{
			AIPerceptionComponent->SetSenseEnabled(UAISense_Sight::StaticClass(), false);
			AIPerceptionComponent->ForgetAll();
			AIPerceptionComponent->Deactivate();
		}
		
		CurrentController->UnPossess();
	}

	if (CharacterWidgetComponent) { CharacterWidgetComponent->Deactivate(); }
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