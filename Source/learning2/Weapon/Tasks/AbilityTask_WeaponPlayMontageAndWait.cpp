// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityTask_WeaponPlayMontageAndWait.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemLog.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"

UAbilityTask_WeaponPlayMontageAndWait* UAbilityTask_WeaponPlayMontageAndWait::CreateWeaponPlayMontageAndWaitProxy(UGameplayAbility* OwningAbility, FName TaskInstanceName, UAnimMontage* CharacterMontageToPlay, UAnimMontage* WeaponMontageToPlay, USkeletalMeshComponent* WeaponMesh, float InRate, float Duration, FName InStartSection, bool bInStopWhenAbilityEnds, float InAnimRootMotionTranslationScale, float InStartTimeSeconds, bool bInAllowInterruptAfterBlendOut)
{
	if (!CharacterMontageToPlay || !WeaponMontageToPlay) { return nullptr; }

	UAbilitySystemGlobals::NonShipping_ApplyGlobalAbilityScaler_Rate(InRate);

	UAbilityTask_WeaponPlayMontageAndWait* MyObj = NewAbilityTask<UAbilityTask_WeaponPlayMontageAndWait>(OwningAbility, TaskInstanceName);
	MyObj->MontageToPlay = CharacterMontageToPlay;
	MyObj->WeaponMontageToPlay = WeaponMontageToPlay;
	MyObj->WeaponMesh = WeaponMesh;

	MyObj->Rate = InRate;
	MyObj->Duration = Duration;
	MyObj->StartSection = InStartSection;
	MyObj->AnimRootMotionTranslationScale = InAnimRootMotionTranslationScale;
	MyObj->bStopWhenAbilityEnds = bInStopWhenAbilityEnds;
	MyObj->bAllowInterruptAfterBlendOut = bInAllowInterruptAfterBlendOut;
	MyObj->StartTimeSeconds = InStartTimeSeconds;

	return MyObj;
}

void UAbilityTask_WeaponPlayMontageAndWait::Activate()
{
	if (Ability == nullptr || !WeaponMesh.IsValid())
	{
		return;
	}

	bool bPlayedMontage = false;

	if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
	{
		const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
		UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();
		UAnimInstance* WeaponAnimInstance = WeaponMesh->GetAnimInstance();
		if (AnimInstance != nullptr && WeaponAnimInstance != nullptr)
		{
			const float CharacterMontagePlayRate = FMath::Max(1.f, MontageToPlay->GetPlayLength() / Duration * Rate);
			const float WeaponMontagePlayRate = FMath::Max(1.f, WeaponMontageToPlay->GetPlayLength() / Duration * Rate);

			UE_LOG(LogTemp, Error, TEXT("MontageToPlay->GetPlayLength(): %f"), MontageToPlay->GetPlayLength());
			UE_LOG(LogTemp, Error, TEXT("WeaponMontageToPlay->GetPlayLength(): %f"), WeaponMontageToPlay->GetPlayLength());
			UE_LOG(LogTemp, Error, TEXT("Duration: %f"), Duration);
			UE_LOG(LogTemp, Error, TEXT("Rate: %f"), Rate);
			UE_LOG(LogTemp, Error, TEXT("CharacterMontagePlayRate: %f"), CharacterMontagePlayRate);
			UE_LOG(LogTemp, Error, TEXT("WeaponMontagePlayRate: %f"), WeaponMontagePlayRate);

			float MontagePlayResult = ASC->PlayMontage(Ability, Ability->GetCurrentActivationInfo(), MontageToPlay, CharacterMontagePlayRate, StartSection, StartTimeSeconds);
			float WeaponMontagePlayResult = WeaponAnimInstance->Montage_Play(WeaponMontageToPlay, WeaponMontagePlayRate, EMontagePlayReturnType::MontageLength, StartTimeSeconds * (Rate / WeaponMontagePlayRate));
			if (MontagePlayResult > 0.f && WeaponMontagePlayResult > 0.f)
			{
				if (StartSection != NAME_None)
				{
					AnimInstance->Montage_JumpToSection(StartSection, WeaponMontageToPlay);
				}

				// Playing a montage could potentially fire off a callback into game code which could kill this ability! Early out if we are  pending kill.
				if (ShouldBroadcastAbilityTaskDelegates() == false)
				{
					return;
				}

				InterruptedHandle = Ability->OnGameplayAbilityCancelled.AddUObject(this, &UAbilityTask_PlayMontageAndWait::OnGameplayAbilityCancelled);
				WeaponInterruptedHandle = Ability->OnGameplayAbilityCancelled.AddUObject(this, &UAbilityTask_WeaponPlayMontageAndWait::StopWeaponPlayingMontage);

				BlendedInDelegate.BindUObject(this, &UAbilityTask_PlayMontageAndWait::OnMontageBlendedIn);
				AnimInstance->Montage_SetBlendedInDelegate(BlendedInDelegate, MontageToPlay);
				WeaponAnimInstance->Montage_SetBlendedInDelegate(BlendedInDelegate, WeaponMontageToPlay);

				BlendingOutDelegate.BindUObject(this, &UAbilityTask_PlayMontageAndWait::OnMontageBlendingOut);
				AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, MontageToPlay);
				WeaponAnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, WeaponMontageToPlay);

				MontageEndedDelegate.BindUObject(this, &UAbilityTask_PlayMontageAndWait::OnMontageEnded);
				AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, MontageToPlay);
				WeaponAnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, WeaponMontageToPlay);

				ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
				if (Character && (Character->GetLocalRole() == ROLE_Authority ||
					(Character->GetLocalRole() == ROLE_AutonomousProxy && Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted)))
				{
					Character->SetAnimRootMotionTranslationScale(AnimRootMotionTranslationScale);
				}

				bPlayedMontage = true;
			}
		}
		else
		{
			ABILITY_LOG(Warning, TEXT("UAbilityTask_PlayMontageAndWait call to PlayMontage failed!"));
		}
	}
	else
	{
		ABILITY_LOG(Warning, TEXT("UAbilityTask_PlayMontageAndWait called on invalid AbilitySystemComponent"));
	}

	if (!bPlayedMontage)
	{
		ABILITY_LOG(Warning, TEXT("UAbilityTask_PlayMontageAndWait called in Ability %s failed to play montage %s; Task Instance Name %s."), *Ability->GetName(), *GetNameSafe(MontageToPlay), *InstanceName.ToString());
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnCancelled.Broadcast();
		}
	}

	SetWaitingOnAvatar();
}

FString UAbilityTask_WeaponPlayMontageAndWait::GetDebugString() const
{
	UAnimMontage* PlayingMontage = nullptr;
	UAnimMontage* PlayingMontageOnWeaponMesh = nullptr;
	if (Ability)
	{
		const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
		UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();

		if (AnimInstance != nullptr)
		{
			PlayingMontage = AnimInstance->Montage_IsActive(MontageToPlay) ? ToRawPtr(MontageToPlay) : AnimInstance->GetCurrentActiveMontage();
		}
	}

	if (WeaponMesh.IsValid())
	{
		UAnimInstance* WeaponAnimInstance = WeaponMesh->GetAnimInstance();

		if (WeaponAnimInstance != nullptr)
		{
			PlayingMontageOnWeaponMesh = WeaponAnimInstance->Montage_IsActive(WeaponMontageToPlay) ? ToRawPtr(WeaponMontageToPlay) : WeaponAnimInstance->GetCurrentActiveMontage();
		}
	}

	return FString::Printf(TEXT("WeaponPlayMontageAndWait. CharacterMontageToPlay: %s  WeaponMontageToPlay(MontageToPlay): %s  (Currently Playing): %s and %s(On Weapon Mesh)"), *GetNameSafe(MontageToPlay), *GetNameSafe(WeaponMontageToPlay), *GetNameSafe(PlayingMontage), *GetNameSafe(PlayingMontageOnWeaponMesh));
}

void UAbilityTask_WeaponPlayMontageAndWait::OnDestroy(bool AbilityEnded)
{
	if (Ability)
	{
		Ability->OnGameplayAbilityCancelled.Remove(WeaponInterruptedHandle);
	}

	if (WeaponMesh.IsValid())
	{
		UAnimInstance* WeaponAniInstance = WeaponMesh->GetAnimInstance();

		if (WeaponAniInstance->GetCurrentActiveMontage() == WeaponMontageToPlay)
		{
			StopWeaponPlayingMontage();
		}
	}

	Super::OnDestroy(AbilityEnded);
}

void UAbilityTask_WeaponPlayMontageAndWait::StopWeaponPlayingMontage()
{
	if (!WeaponMesh.IsValid())
	{
		return;
	}

	UAnimInstance* WeaponAnimInstance = WeaponMesh->GetAnimInstance();
	if (WeaponAnimInstance == nullptr)
	{
		return;
	}

	// Check if the montage is still playing
	// The ability would have been interrupted, in which case we should automatically stop the montage
	if (WeaponAnimInstance->GetCurrentActiveMontage() == WeaponMontageToPlay)
	{
		FAnimMontageInstance* WeaponMontageInstance = WeaponAnimInstance->GetActiveInstanceForMontage(WeaponMontageToPlay);
		if (WeaponMontageInstance)
		{
			WeaponMontageInstance->OnMontageBlendedInEnded.Unbind();
			WeaponMontageInstance->OnMontageBlendingOutStarted.Unbind();
			WeaponMontageInstance->OnMontageEnded.Unbind();
		}

		WeaponAnimInstance->Montage_Stop(.0f, WeaponMontageToPlay);
	}
}
