// Fill out your copyright notice in the Description page of Project Settings.


#include "WidgetController.h"
#include "Controller/MyPlayerController.h"
#include "Ability/AttributeSet/MyAttributeSet.h"
#include "Ability/AbilitySystemComponent/MyAbilitySystemComponent.h"
//#include "InputRecordComponent.h"
#include "WeaponBase/WeaponActorBase.h"
#include "Character/PlayerCharacterBase.h"
#include "BlueprintFunctionLibrary/WeaponActorBlueprintLibrary.h"
#include "BlueprintFunctionLibrary/UserWidgetBlueprintLibrary.h"

void UWidgetController::SetUpWidgetController(FPossessedCharacterWidgetControllerContext Context)
{
	{
		FPossessedCharacterWidgetControllerContext OldContext{ PlayerController, AttributeSet.Get(), PlayerState, AbilitySystemComponent.Get(), InputRecordComponent.Get(), MyCharacter.Get() };
		DeinitializeDelegates(OldContext, this);
	}

	PlayerController = Context.PlayerController.Get();
	AttributeSet = Context.AttributeSet;
	PlayerState = Context.PlayerState.Get();
	AbilitySystemComponent = Context.AbilitySystemComponent;
	InputRecordComponent = Context.InputRecordComponent;
	MyCharacter = Context.MyCharacter;

	if (Context.IsValid()) { InitializeDelegates(Context, this); }
}

//bool UWidgetController::GetWidgetData(FHUDData& HUDData) const
//{
//	const AWeaponActorBase* Weapon{ MyCharacter->GetControlledWeapon() };
//	if (!Weapon) { return false; }
//
//	TArray<AWeaponActorBase*> Weapons{ MyCharacter->GetWeapons() };
//	FHUDData Data;
//	Data.ControlledWeapon = Weapon->GetWeaponSlot();
//	if (Weapons.IsValidIndex(0)) { Data.Weapon1 = { Weapons[0]->GetWeaponSlot(), Weapons[0]->GetMagazineAmmo(), Weapons[0]->GetMagazineAmmoMax() , Weapons[0]->GetWeaponIcon() }; }
//	if (Weapons.IsValidIndex(1)) { Data.Weapon2 = { Weapons[1]->GetWeaponSlot(), Weapons[1]->GetMagazineAmmo(), Weapons[1]->GetMagazineAmmoMax() ,Weapons[1]->GetWeaponIcon() }; }
//	if (Weapons.IsValidIndex(2)) { Data.Weapon3 = { Weapons[2]->GetWeaponSlot(), Weapons[2]->GetMagazineAmmo(), Weapons[2]->GetMagazineAmmoMax() ,Weapons[2]->GetWeaponIcon() }; }
//	if (Weapons.IsValidIndex(3)) { Data.Weapon4 = { Weapons[3]->GetWeaponSlot(), Weapons[3]->GetMagazineAmmo(), Weapons[3]->GetMagazineAmmoMax() ,Weapons[3]->GetWeaponIcon() }; }
//	if (Weapons.IsValidIndex(4)) { Data.Weapon5 = { Weapons[4]->GetWeaponSlot(), Weapons[4]->GetMagazineAmmo(), Weapons[4]->GetMagazineAmmoMax() ,Weapons[4]->GetWeaponIcon() }; }
//
//	HUDData = Data;
//	return true;
//}

void UWidgetController::InitializeDelegates(FPossessedCharacterWidgetControllerContext& Context, UWidgetController* Self)
{
	//if (Context.PlayerController.IsValid()) {
	//	Context.PlayerController->OnPossessedPawnChanged.AddUniqueDynamic(Self, &UWidgetController::HandlePossessedPawnChanged);
	//}

	if (Context.AbilitySystemComponent.IsValid())
	{
		Context.AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetCurrentHealthAttribute()).AddUObject(Self, &UWidgetController::OnCurrentHealthChanged);
		Context.AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMaxHealthAttribute()).AddUObject(Self, &UWidgetController::OnMaxHealthChanged);

		Context.AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMagazineAmmo1Attribute()).AddUObject(Self, &UWidgetController::OnWeaponMagazineAmmoChanged);
		Context.AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMagazineAmmo2Attribute()).AddUObject(Self, &UWidgetController::OnWeaponMagazineAmmoChanged);
		Context.AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMagazineAmmo3Attribute()).AddUObject(Self, &UWidgetController::OnWeaponMagazineAmmoChanged);
		Context.AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMagazineAmmo4Attribute()).AddUObject(Self, &UWidgetController::OnWeaponMagazineAmmoChanged);
		Context.AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMagazineAmmo5Attribute()).AddUObject(Self, &UWidgetController::OnWeaponMagazineAmmoChanged);
	}

	if (URewindSubsystem* RewindSubsystem = GetRewindSubsystem(Context.PlayerController.Get()))
	{
		RewindSubsystem->OnRewindSubsystemStateChangedDelegate.AddUniqueDynamic(Self, &UWidgetController::OnRewindSubsystemStateChanged);
	}

	if (Context.InputRecordComponent.IsValid())
	{
		Context.InputRecordComponent->OnInputRecordComponentTickChangedDelegate.AddUniqueDynamic(Self, &UWidgetController::OnInputRecordComponentTickChanged);
	}

	if (Context.MyCharacter.IsValid())
	{
		Context.MyCharacter->OnControlledWeaponChangedDelegate.AddUniqueDynamic(Self, &UWidgetController::OnWeaponChanged);
		Context.MyCharacter->OnWeaponAddedDelegate.AddUObject(Self, &UWidgetController::OnWeaponAdded);
		Context.MyCharacter->OnWeaponRemovedDelegate.AddUObject(Self, &UWidgetController::OnWeaponRemoved);
		//MyCharacter->OnControlledWeaponMagazineAmmoChangedDelegate.AddDynamic(this, &UWidgetController::OnControlledWeaponMagazineAmmoChanged);
		//MyCharacter->OnControlledWeaponReserveAmmoChangedDelegate.AddDynamic(this, &UWidgetController::OnControlledWeaponReserveAmmoChanged);
	}
}

void UWidgetController::DeinitializeDelegates(FPossessedCharacterWidgetControllerContext& Context, UWidgetController* Self)
{
	if (Context.PlayerController.IsValid()) { Context.PlayerController->OnPossessedPawnChanged.RemoveAll(Self); }

	if (Context.AbilitySystemComponent.IsValid())
	{
		Context.AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetCurrentHealthAttribute()).RemoveAll(Self);
		Context.AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMaxHealthAttribute()).RemoveAll(Self);
		Context.AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMagazineAmmo1Attribute()).RemoveAll(Self);
		Context.AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMagazineAmmo2Attribute()).RemoveAll(Self);
		Context.AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMagazineAmmo3Attribute()).RemoveAll(Self);
		Context.AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMagazineAmmo4Attribute()).RemoveAll(Self);
		Context.AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMagazineAmmo5Attribute()).RemoveAll(Self);
	}

	if (URewindSubsystem* RewindSubsystem = GetRewindSubsystem(Context.PlayerController.Get()))
	{
		RewindSubsystem->OnRewindSubsystemStateChangedDelegate.RemoveAll(Self);
	}

	if (Context.InputRecordComponent.IsValid())
	{
		Context.InputRecordComponent->OnInputRecordComponentTickChangedDelegate.RemoveAll(Self);
	}

	if (Context.MyCharacter.IsValid())
	{
		Context.MyCharacter->OnControlledWeaponChangedDelegate.RemoveAll(Self);
		Context.MyCharacter->OnWeaponAddedDelegate.RemoveAll(Self);
		Context.MyCharacter->OnWeaponRemovedDelegate.RemoveAll(Self);
		//MyCharacter->OnControlledWeaponMagazineAmmoChangedDelegate.RemoveAll(this);
		//MyCharacter->OnControlledWeaponReserveAmmoChangedDelegate.RemoveAll(this);
	}
}

void UWidgetController::BroadcastInitialData()
{
	if (MyCharacter.IsValid() && InputRecordComponent.IsValid())
	{
		const ERecordState RecordState{ InputRecordComponent->GetRecordState() };
		const RewindSystemTickType CurrentTick{ InputRecordComponent->GetCurrentTick() };
		const RewindSystemTickType TickMax{ InputRecordComponent->GetTickMax() };

		OnPossessedCharacterChangedDelegate.Broadcast(nullptr, MyCharacter.Get());
		OnRewindSubsystemStateChangedDelegate.Broadcast(ERecordState::Idle, RecordState);
		OnInputRecordComponentTickChangedDelegate.Broadcast(-1, CurrentTick, TickMax);
	}

	if (AttributeSet.IsValid())
	{
		OnCurrentHealthChangedDelegate.Broadcast(0.f, AttributeSet->GetCurrentHealth());
		OnMaxHealthChangedDelegate.Broadcast(0.f, AttributeSet->GetMaxHealth());
	}

	//if (const URewindSubsystem * RewindSubsystem{ GetRewindSubsystem() })
	//{
	//	OnRewindSubsystemStateChangedDelegate.Broadcast(ERecordState::Idle, RewindSubsystem->GetCurrentState());
	//}

	for (AWeaponActorBase* Weapon : MyCharacter->GetWeapons())
	{
		const AWeaponActorBase* ControlledWeapon{ MyCharacter.IsValid() ? MyCharacter->GetControlledWeapon() : nullptr };

		OnWeaponChangedDelegate.Broadcast(Weapon, Weapon == ControlledWeapon);
		OnWeaponMagazineAmmoChangedDelegate.Broadcast(Weapon->GetWeaponSlot(), 0, Weapon->GetMagazineAmmo());
		OnWeaponMagazineAmmoMaxChangedDelegate.Broadcast(Weapon->GetWeaponSlot(), 0, Weapon->GetMagazineAmmoMax());
	}
}

void UWidgetController::OnSliderChanged(const float NewVolume) const
{
	if (!InputRecordComponent.IsValid()) { return; }

	URewindSubsystem* RewindSubsystem{ GetRewindSubsystem() };
	if (!RewindSubsystem) { return; }

	if (InputRecordComponent->GetRecordState() == ERecordState::Recording) { RewindSubsystem->SwitchState(ERecordState::PreviewPause); }
	if (InputRecordComponent->GetRecordState() != ERecordState::PreviewPause) { return; }

	//const float Tick{ FMath::Max(0.f, NewVolume) * InputRecordComponent->GetTickMax() };
	//const RewindSystemTickType RoundedTick{ static_cast<RewindSystemTickType>(FMath::RoundToInt(Tick)) };

	RewindSubsystem->ToTick(*InputRecordComponent.Get(), UUserWidgetBlueprintLibrary::ToTick_Rate(NewVolume, InputRecordComponent->GetTickMax()));
}

void UWidgetController::OnButtonPressed(const ERecordState ButtonType) const
{
	if (URewindSubsystem * RewindSubsystem{ GetRewindSubsystem() })
	{
		RewindSubsystem->SwitchState(ButtonType);
	}
}

//void UWidgetController::HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
//{
//	APlayerCharacterBase* OldCharacter{ Cast<APlayerCharacterBase>(OldPawn) };
//	APlayerCharacterBase* NewCharacter{ Cast<APlayerCharacterBase>(NewPawn) };
//	MyCharacter = NewCharacter;
//
//	if (OldCharacter != NewCharacter)
//	{
//		OnPossessedCharacterChangedDelegate.Broadcast(OldCharacter, NewCharacter);
//
//		if (OldCharacter) { DeinitializeDelegates({ OldCharacter }, this); }
//
//		{
//			FPossessedCharacterWidgetControllerContext Context{ PlayerController , AttributeSet.Get(), PlayerState, AbilitySystemComponent.Get(), InputRecordComponent.Get(), MyCharacter.Get() };
//			InitializeDelegates(Context, this);
//		}
//
//		if (InputRecordComponent.IsValid())
//		{
//			const ERecordState RecordState{ InputRecordComponent->GetRecordState() };
//			const RewindSystemTickType CurrentTick{ InputRecordComponent->GetCurrentTick() };
//			const RewindSystemTickType TickMax{ InputRecordComponent->GetTickMax() };
//
//			OnRewindSubsystemStateChangedDelegate.Broadcast(ERecordState::Idle, RecordState);
//			OnInputRecordComponentTickChangedDelegate.Broadcast(-1, CurrentTick, TickMax);
//		}
//	}
//
//	BroadcastInitialData();
//
//	//OnPossessedPawnChanged.Broadcast(OldPawn, NewPawn);
//}

void UWidgetController::OnCurrentHealthChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	OnCurrentHealthChangedDelegate.Broadcast(OnAttributeChangeData.OldValue, OnAttributeChangeData.NewValue);
}

void UWidgetController::OnMaxHealthChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	OnMaxHealthChangedDelegate.Broadcast(OnAttributeChangeData.OldValue, OnAttributeChangeData.NewValue);
}

void UWidgetController::OnRewindSubsystemStateChanged(ERecordState OldState, ERecordState NewState)
{
	OnRewindSubsystemStateChangedDelegate.Broadcast(OldState, NewState);
}

void UWidgetController::OnInputRecordComponentTickChanged(int32 OldTick, int32 NewTick, int32 TickMax)
{
	OnInputRecordComponentTickChangedDelegate.Broadcast(OldTick, NewTick, TickMax);
}

void UWidgetController::OnWeaponChanged(AWeaponActorBase* OldWeapon, AWeaponActorBase* NewWeapon)
{
	const AWeaponActorBase* ControlledWeapon{ MyCharacter.IsValid() ? MyCharacter->GetControlledWeapon() : nullptr };
	OnWeaponChangedDelegate.Broadcast(NewWeapon, NewWeapon == ControlledWeapon);
	OnWeaponMagazineAmmoChangedDelegate.Broadcast(NewWeapon->GetWeaponSlot(), OldWeapon ? OldWeapon->GetMagazineAmmo() : 0, NewWeapon ? NewWeapon->GetMagazineAmmo() : 0);
}

void UWidgetController::OnWeaponAdded(AWeaponActorBase* Weapon)
{
	const AWeaponActorBase* ControlledWeapon{ MyCharacter.IsValid() ? MyCharacter->GetControlledWeapon() : nullptr };
	OnWeaponChangedDelegate.Broadcast(Weapon, Weapon == ControlledWeapon);
	OnWeaponMagazineAmmoChangedDelegate.Broadcast(Weapon->GetWeaponSlot(), 0, Weapon ? Weapon->GetMagazineAmmo() : 0);
}

void UWidgetController::OnWeaponRemoved(AWeaponActorBase* Weapon)
{
	const AWeaponActorBase* ControlledWeapon{ MyCharacter.IsValid() ? MyCharacter->GetControlledWeapon() : nullptr };
	OnWeaponChangedDelegate.Broadcast(Weapon, Weapon == ControlledWeapon);
	OnWeaponMagazineAmmoChangedDelegate.Broadcast(Weapon->GetWeaponSlot(), Weapon ? Weapon->GetMagazineAmmo() : 0, 0);
}

void UWidgetController::OnWeaponMagazineAmmoChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	EWeaponSlot WeaponSlot{ EWeaponSlot::None };
	if (UWeaponActorBlueprintLibrary::GetWeaponSlotByWeaponAttribute(WeaponSlot, OnAttributeChangeData.Attribute))
	{
		OnWeaponMagazineAmmoChangedDelegate.Broadcast(WeaponSlot, OnAttributeChangeData.OldValue, OnAttributeChangeData.NewValue);
	}
}

URewindSubsystem* UWidgetController::GetRewindSubsystem() const
{
	const UWorld* World{ GetWorld() };
	if (!World) { nullptr; }

	return ULocalPlayer::GetSubsystem<URewindSubsystem>(World->GetFirstLocalPlayerFromController());
}

URewindSubsystem* UWidgetController::GetRewindSubsystem(const AMyPlayerController* Controller)
{
	return ULocalPlayer::GetSubsystem<URewindSubsystem>(Controller ? Controller->GetLocalPlayer() : nullptr);
}

//void UWidgetController::OnControlledWeaponMagazineAmmoChanged(int32 OldAmmo, int32 NewAmmo)
//{
//	OnControlledWeaponMagazineAmmoChangedDelegate.Broadcast(OldAmmo, NewAmmo);
//}
//
//void UWidgetController::OnControlledWeaponReserveAmmoChanged(int32 OldAmmo, int32 NewAmmo)
//{
//	OnControlledWeaponReserveAmmoChangedDelegate.Broadcast(OldAmmo, NewAmmo);
//}