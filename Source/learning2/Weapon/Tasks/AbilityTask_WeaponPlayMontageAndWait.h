// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilityTask_WeaponPlayMontageAndWait.generated.h"

/**
 *
 */
class USkeletalMeshComponent;

UCLASS(MinimalAPI)
class UAbilityTask_WeaponPlayMontageAndWait : public UAbilityTask_PlayMontageAndWait
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Ability|Weapon|Tasks", meta = (DisplayName = "PlayWeaponMontageAndWait",
		HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static LEARNING2_API UAbilityTask_WeaponPlayMontageAndWait* CreateWeaponPlayMontageAndWaitProxy(UGameplayAbility* OwningAbility, FName TaskInstanceName, UAnimMontage* CharacterMontageToPlay, UAnimMontage* WeaponMontageToPlay, USkeletalMeshComponent* WeaponMesh, float InRate, float Duration = 1.f, FName InStartSection = NAME_None, bool bInStopWhenAbilityEnds = true, float InAnimRootMotionTranslationScale = 1.f, float InStartTimeSeconds = 0.f, bool bInAllowInterruptAfterBlendOut = false);

	LEARNING2_API virtual void Activate() override;

	LEARNING2_API virtual FString GetDebugString() const override;

protected:
	LEARNING2_API virtual void OnDestroy(bool AbilityEnded) override;

	LEARNING2_API void StopWeaponPlayingMontage();

private:
	static UAbilityTask_PlayMontageAndWait* CreatePlayMontageAndWaitProxy(UGameplayAbility* OwningAbility,
		FName TaskInstanceName, UAnimMontage* MontageToPlay, float Rate = 1.f, FName StartSection = NAME_None, bool bStopWhenAbilityEnds = true, float AnimRootMotionTranslationScale = 1.f, float StartTimeSeconds = 0.f, bool bAllowInterruptAfterBlendOut = false)
	{
		return Super::CreatePlayMontageAndWaitProxy(OwningAbility, TaskInstanceName, MontageToPlay, Rate, StartSection, bStopWhenAbilityEnds, AnimRootMotionTranslationScale, StartTimeSeconds, bAllowInterruptAfterBlendOut);
	}

protected:
	UPROPERTY()
	TObjectPtr<UAnimMontage> WeaponMontageToPlay;

	UPROPERTY()
	TWeakObjectPtr<USkeletalMeshComponent> WeaponMesh;

	FDelegateHandle WeaponInterruptedHandle;

	float Duration{ 1.f };
};
