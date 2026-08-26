#include "WeaponFireBase.h"
#include "Character/Base/MyCharacterBase.h"
#include "Weapon/WeaponBase/FirearmActorBase.h"
#include "Weapon/WeaponBase/WeaponActorBase.h"
#include "AbilitySystemComponent.h"

void UWeaponFireBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (IRecordableInterface::Execute_ShouldRecord(this))
	{
		Super::ActivateAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfoRef(), CachedTriggerEventData.GetPtrOrNull());
	}
	else
	{
		UWorld* World{ GetWorld() };
		//Instigator = TriggerEventData ? Cast<AMyCharacterBase>(TriggerEventData->Instigator) : Cast<AMyCharacterBase>(GetAvatarActorFromActorInfo());
		Instigator = Cast<AMyCharacterBase>(GetAvatarActorFromActorInfo());
		Weapon = Instigator.IsValid() ? Instigator->GetControlledWeapon() : nullptr;
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

		if (!World || !Instigator.IsValid() || !Weapon.IsValid() || !ASC)
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}

		if (TriggerEventData) { CachedTriggerEventData = *TriggerEventData; }
		if (ASC->HasMatchingGameplayTag(OnShootTag)) { ExecuteFire(OnShootTag, ASC->GetTagCount(OnShootTag)); }
		else
		{
			RegisterGameplayTagEventHandle = ASC->RegisterGameplayTagEvent(OnShootTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UWeaponFireBase::ExecuteFire);
			World->GetTimerManager().SetTimer(WaitForTagTimeOutHandle, FTimerDelegate::CreateUObject(this, &UWeaponFireBase::OnWaitForTagTimeOut), .2f, false);
		}
	}
}

void UWeaponFireBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Instigator.Reset();
	Weapon.Reset();
	CachedTriggerEventData.Reset();

	if (RegisterGameplayTagEventHandle.IsValid())
	{
		if (UAbilitySystemComponent * ASC{ GetAbilitySystemComponentFromActorInfo() })
		{
			ASC->RegisterGameplayTagEvent(OnShootTag, EGameplayTagEventType::NewOrRemoved).Remove(RegisterGameplayTagEventHandle);
		}
		RegisterGameplayTagEventHandle.Reset();
	}
	if (WaitForTagTimeOutHandle.IsValid())
	{
		if (UWorld * World{ GetWorld() })
		{
			World->GetTimerManager().ClearTimer(WaitForTagTimeOutHandle);
		}
		WaitForTagTimeOutHandle.Invalidate();
	}
	if (FireLoopHandle.IsValid())
	{
		if (UWorld * World{ GetWorld() })
		{
			World->GetTimerManager().ClearTimer(FireLoopHandle);
		}
		FireLoopHandle.Invalidate();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UWeaponFireBase::ExecuteFire(const FGameplayTag Tag, const int32 NewCount)
{
	UWorld* World{ GetWorld() };
	if (!World)
	{
		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfoRef(), true, true);
		return;
	}
	World->GetTimerManager().ClearTimer(WaitForTagTimeOutHandle);

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RegisterGameplayTagEvent(OnShootTag, EGameplayTagEventType::NewOrRemoved).Remove(RegisterGameplayTagEventHandle);
	}

	RegisterGameplayTagEventHandle.Reset();
	WaitForTagTimeOutHandle.Invalidate();

	if (!Instigator.IsValid() || !Weapon.IsValid() || Weapon->GetFireRate() < 0 || FMath::IsNearlyEqual(Weapon->GetFireRate(), 0))
	{
		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfoRef(), true, true);
		return;
	}

	World->GetTimerManager().SetTimer(FireLoopHandle, FTimerDelegate::CreateUObject(this, &UWeaponFireBase::ExecuteFire_Internal), 1.f / Weapon->GetFireRate(), true);
	if (AFirearmActorBase* Firearm = Cast<AFirearmActorBase>(Weapon.Get())) { Firearm->NotifyExecutingShoot(); }
	ExecuteFire_Internal();
}

void UWeaponFireBase::ExecuteFire_Internal()
{
	if (AFirearmActorBase* Firearm = Cast<AFirearmActorBase>(Weapon.Get()))
	{
		Firearm->NotifyShootCooldownFinished();
	}

	if (!Instigator.IsValid() || !Weapon.IsValid() || !CanExecuteShoot() || !CheckCost(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), nullptr))
	{
		if (AFirearmActorBase* Firearm = Cast<AFirearmActorBase>(Weapon.Get())) { Firearm->NotifyShootFinish(); }
		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfoRef(), true, true);
		return;
	}

	OnExecuteShoot();
	Super::ActivateAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfoRef(), CachedTriggerEventData.GetPtrOrNull());

	if (AFirearmActorBase* Firearm = Cast<AFirearmActorBase>(Weapon.Get())) { Firearm->NotifyShootFinish(); }
}

void UWeaponFireBase::FinishShoot()
{
	if (!IsActive() || !Weapon.IsValid()) { return; }

	for (int32 i = 0; i < Weapon->GetBulletSpawnsOnFire(); i++)
	{
		Weapon->SpawnBullet();
	}
}

void UWeaponFireBase::OnWaitForTagTimeOut()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RegisterGameplayTagEvent(OnShootTag, EGameplayTagEventType::NewOrRemoved).Remove(RegisterGameplayTagEventHandle);
	}

	RegisterGameplayTagEventHandle.Reset();
	WaitForTagTimeOutHandle.Invalidate();

	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfoRef(), true, true);
}