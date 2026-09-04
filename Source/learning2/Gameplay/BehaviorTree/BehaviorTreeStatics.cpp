// Fill out your copyright notice in the Description page of Project Settings.


#include "BehaviorTreeStatics.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Battle/BattleSubsystemTypes.h"
#include "GameFramework/Character.h"

void UBehaviorTreeStatics::SetSelfActor(AActor* SelfActor)
{
	if (!SelfActor) { return; }

	UBlackboardComponent* BlackboardComponent{ UAIBlueprintHelperLibrary::GetBlackboard(SelfActor) };
	if (!BlackboardComponent) { return; }

	BlackboardComponent->SetValueAsObject(BattleSubsystemConst::BlackboardKeyName::SelfActor, SelfActor);
}

void UBehaviorTreeStatics::SetEnemyCharacter(ACharacter* Enemy, AActor* Actor)
{
	if (!Actor) { return; }

	UBlackboardComponent* BlackboardComponent{ UAIBlueprintHelperLibrary::GetBlackboard(Actor) };
	if (!BlackboardComponent) { return; }

	if (Enemy) { BlackboardComponent->SetValueAsObject(BattleSubsystemConst::BlackboardKeyName::EnemyCharacter, Enemy); }
	else { BlackboardComponent->ClearValue(BattleSubsystemConst::BlackboardKeyName::EnemyCharacter); }
}

ACharacter* UBehaviorTreeStatics::GetEnemyCharacter(AActor* Actor)
{
	if (!Actor) { return nullptr; }

	UBlackboardComponent* BlackboardComponent{ UAIBlueprintHelperLibrary::GetBlackboard(Actor) };
	if (!BlackboardComponent) { return nullptr; }

	return Cast<ACharacter>(BlackboardComponent->GetValueAsObject(BattleSubsystemConst::BlackboardKeyName::EnemyCharacter));
}

void UBehaviorTreeStatics::SetWeaponMagazineAmmo(AActor* Actor, const int32 Ammo, const EWeaponSlot Slot)
{
	if (!Actor) { return; }

	UBlackboardComponent* BlackboardComponent{ UAIBlueprintHelperLibrary::GetBlackboard(Actor) };
	if (!BlackboardComponent) { return; }

	const TOptional<EWeaponSlot> CurrentWeaponSlot{ GetControlledWeaponSlot(Actor) };

	const FName KeyName{ !CurrentWeaponSlot.IsSet() || CurrentWeaponSlot.GetValue() == Slot ? BattleSubsystemConst::BlackboardKeyName::ControlledWeaponAmmo : BattleSubsystemConst::BlackboardKeyName::UncontrolledWeaponAmmo };
	BlackboardComponent->SetValueAsInt(KeyName, Ammo);
}

void UBehaviorTreeStatics::SetWeaponMaxMagazineAmmo(AActor* Actor, const int32 AmmoMax, const EWeaponSlot Slot)
{
	if (!Actor) { return; }

	UBlackboardComponent* BlackboardComponent{ UAIBlueprintHelperLibrary::GetBlackboard(Actor) };
	if (!BlackboardComponent) { return; }

	const TOptional<EWeaponSlot> CurrentWeaponSlot{ GetControlledWeaponSlot(Actor) };

	const FName KeyName{ !CurrentWeaponSlot.IsSet() || CurrentWeaponSlot.GetValue() == Slot ? BattleSubsystemConst::BlackboardKeyName::ControlledWeaponMaxAmmo : BattleSubsystemConst::BlackboardKeyName::UncontrolledWeaponMaxAmmo };
	BlackboardComponent->SetValueAsInt(KeyName, AmmoMax);
}

TOptional<EWeaponSlot> UBehaviorTreeStatics::GetControlledWeaponSlot(AActor* Actor)
{
	if (!Actor) { return NullOpt; }

	const UBlackboardComponent* BlackboardComponent{ UAIBlueprintHelperLibrary::GetBlackboard(Actor) };
	if (!BlackboardComponent) { return NullOpt; }

	const EWeaponSlot WeaponSlot{ static_cast<EWeaponSlot>(BlackboardComponent->GetValueAsInt(BattleSubsystemConst::BlackboardKeyName::ControlledWeaponSlot)) };
	return FMath::IsWithin(WeaponSlot, EWeaponSlot::Ammo1, EWeaponSlot::MAX) ? TOptional<EWeaponSlot>(WeaponSlot) : NullOpt;
}

void UBehaviorTreeStatics::SetTargetLocation(const FVector& TargetLocation, AActor* Actor)
{
	if (!Actor) { return; }

	UBlackboardComponent* BlackboardComponent{ UAIBlueprintHelperLibrary::GetBlackboard(Actor) };
	if (!BlackboardComponent) { return; }

	BlackboardComponent->SetValueAsVector(BattleSubsystemConst::BlackboardKeyName::TargetLocation, TargetLocation);
}

bool UBehaviorTreeStatics::GetTargetLocation(FVector& TargetLocation, AActor* Actor)
{
	if (!Actor) { return false; }

	UBlackboardComponent* BlackboardComponent{ UAIBlueprintHelperLibrary::GetBlackboard(Actor) };
	if (!BlackboardComponent) { return false; }

	TargetLocation = BlackboardComponent->GetValueAsVector(BattleSubsystemConst::BlackboardKeyName::TargetLocation);
	return true;
}

void UBehaviorTreeStatics::SetControlledWeaponSlot(const EWeaponSlot& WeaponSlot, AActor* Actor)
{
	if (!Actor) { return; }

	UBlackboardComponent* BlackboardComponent{ UAIBlueprintHelperLibrary::GetBlackboard(Actor) };
	if (!BlackboardComponent) { return; }

	BlackboardComponent->SetValueAsInt(BattleSubsystemConst::BlackboardKeyName::ControlledWeaponSlot, static_cast<int32>(WeaponSlot));
}

void UBehaviorTreeStatics::SetSensedEnemyCharacter(ACharacter* Enemy, AActor* Actor)
{
	if (!Actor) { return; }
	
	UBlackboardComponent* BlackboardComponent{ UAIBlueprintHelperLibrary::GetBlackboard(Actor) };
	if (!BlackboardComponent) { return; }
	
	if (Enemy) { BlackboardComponent->SetValueAsObject(BattleSubsystemConst::BlackboardKeyName::SensedCharacter, Enemy); }
	else { BlackboardComponent->ClearValue(BattleSubsystemConst::BlackboardKeyName::SensedCharacter); }
}

ACharacter* UBehaviorTreeStatics::GetSensedEnemyCharacter(AActor* Actor)
{
	if (!Actor) { return nullptr; }

	UBlackboardComponent* BlackboardComponent{ UAIBlueprintHelperLibrary::GetBlackboard(Actor) };
	if (!BlackboardComponent) { return nullptr; }

	return Cast<ACharacter>(BlackboardComponent->GetValueAsObject(BattleSubsystemConst::BlackboardKeyName::SensedCharacter));
}

bool UBehaviorTreeStatics::GetControlledWeaponSlot(EWeaponSlot& WeaponSlot, AActor* Actor)
{
	const TOptional<EWeaponSlot> WeaponSlotOptional{ GetControlledWeaponSlot(Actor) };
	if (!WeaponSlotOptional.IsSet()) { return false; }

	WeaponSlot = WeaponSlotOptional.GetValue();
	return true;
}