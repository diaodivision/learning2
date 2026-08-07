// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Ability/MyGameplayAbilityType.h"
#include "Interactable/InteractionOption.h"
#include "Interactable/InteractionOptionsBuilder.h"
#include <type_traits>
#include <concepts>
#include "Bullet/Base/BulletBaseTypes.h"
#include "WeaponActorBlueprintLibrary.generated.h"

struct FBulletAttributeData;
class APawn;
class AFireArmBulletBase;
class UGameplayEffect;
struct FGameplayEffectSpecHandle;
class AWeaponActorBase;
struct FGameplayTag;
class UGameplayAbility;
class UDataTable;
class UCurveTable;
class APlayerController;
class AGrenadeBulletBase;

/**
 *
 */
UCLASS(Blueprintable)
class LEARNING2_API UWeaponActorBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Utils", meta = (DefaultToSelf = Target))
	static FGameplayEffectSpecHandle SetSetByCaller(const UGameplayAbility* Target, const TSubclassOf<UGameplayEffect> GameplayEffectClass, const FGameplayTag DataTag, const float Magtitude);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Utils", meta = (DefaultToSelf = Target))
	static bool CommitAbilityCooldownSetByCaller(const UGameplayAbility* Target, const TSubclassOf<UGameplayEffect> CooldownEffectClass, const FGameplayTag CooldownTag, const float Magtitude);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Utils", meta = (DefaultToSelf = Target))
	static bool CommitWeaponFireCostSetByCaller(const UGameplayAbility* Target, const TSubclassOf<UGameplayEffect> WeaponCostEffectClass, const FGameplayTag FireCostTag, const int32 FireCost, const FGameplayTag WeaponSlotTag, const EWeaponSlot WeaponSlot);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Utils", meta = (DefaultToSelf = Target))
	static bool CommitWeaponReloadAmountSetByCaller(const UGameplayAbility* Target, const TSubclassOf<UGameplayEffect> WeaponReloadEffectClass, const FGameplayTag AmountToReloadTag, const int32 AmountToReload, const FGameplayTag WeaponSlotTag, const EWeaponSlot WeaponSlot);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Gameplay Effect", meta = (DefaultToSelf = Target))
	static bool GetAbilityCooldownGameplayEffectClass(TSubclassOf<UGameplayEffect>& EffectClass, const UGameplayAbility* Target);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Gameplay Effect", meta = (DefaultToSelf = Target))
	static bool GetAbilityCostGameplayEffectClass(TSubclassOf<UGameplayEffect>& EffectClass, const UGameplayAbility* Target);

	UFUNCTION(BlueprintCallable, Category = "Utils")
	static float RandomWeaponSpreadRadius();
	static float RandomWeaponSpreadRadius(float MaxSpreadRadius);

	UFUNCTION(BlueprintCallable, Category = "Utils")
	static bool CalculateWeaponSpreadDirection(FVector Direction, float SpreadRadius, float Accuracy, FVector& OutNewDirection);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	static bool GetBulletSpawnLocation(APawn* Instigator, FVector& OutLocation);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	static FBulletAttributeData GetBulletAttributeFromWeapon(const AWeaponActorBase* Weapon);

	static AActor* SpawnActorDeferredFromPool(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, const FTransform& Transform);

	static void FinishSpawningOfPoolingActor(AActor* BulletObject);

	static void ReleaseActorToPool(AActor* BulletObject);

	UFUNCTION(BlueprintCallable, Category = "Weapon", meta = (DeterminesOutputType = "BulletClass"))
	static ABulletBase* SpawnBullet(TSubclassOf<AFireArmBulletBase> BulletClass, TSubclassOf<UGameplayEffect> DamageClass, APawn* Instigator, const AWeaponActorBase* Weapon, bool bSpread = false);

	static constexpr float BulletSpawnHeight{ 90.0f };

	static constexpr float WeaponSpreadRadius{ 5.f };

	UFUNCTION(BlueprintPure, Category = "Weapon Const Parameter")
	static float GetBulletSpawnHeight() { return BulletSpawnHeight; }

	UFUNCTION(BlueprintPure, Category = "Weapon Const Parameter")
	static float GetWeaponSpreadRadius() { return WeaponSpreadRadius; }

	//UFUNCTION(BlueprintCallable, Category = "Abilities")
	//static bool TriggerAbilityFromGameplayEvent(FInteractionOption Option, FGameplayTag Tag, FGameplayEventData Payload);

	static bool GetWeaponAttributeFromAttributeSet(FGameplayAttribute& MagazineAmmoAttribute, FGameplayAttribute& ReserveAmmoAttribute, const AWeaponActorBase* Weapon);
	static bool GetWeaponAttributeFromAttributeSetByWeaponSlot(FGameplayAttribute& MagazineAmmoAttribute, FGameplayAttribute& ReserveAmmoAttribute, const EWeaponSlot WeaponSlot);
	static bool GetWeaponSlotByWeaponAttribute(EWeaponSlot& WeaponSlot, const FGameplayAttribute& AmmoAttribute);
	static bool SetWeaponAmmoByAttribute(AWeaponActorBase& Weapon, int32 NewValue, const FGameplayAttribute& AmmoAttribute);

	UFUNCTION(BlueprintPure, Category = "Weapon Attribute")
	static bool GetWeaponAttributeFromDataTable(FWeaponAttributeData& WeaponAttributeData, const AWeaponActorBase* Weapon);

	UFUNCTION(BlueprintPure, Category = "Weapon Attribute")
	static bool GetGrenadeWeaponAttributeFromDataTable(FWeaponAttributeData& WeaponAttributeData, const AWeaponActorBase* Weapon);

	UFUNCTION(BlueprintPure, Category = "Bullet Attribute")
	static bool GetGrenadeBulletAttributeFromDataTable(FGrenadeBulletAttributeData& GrenadeBulletAttributeData, const AGrenadeBulletBase* Bullet);

	UFUNCTION(BlueprintPure, Category = "Weapon Attribute")
	static bool GetWeaponAccuracy(float& Accuracy, const AWeaponActorBase* Weapon);

	UFUNCTION(BlueprintPure, Category = "Projecttile Fly Time")
	static float CalculateTimeToTarget(const float Distance, const float InitialSpeed, const UCurveFloat* const SpeedCurve);

	UFUNCTION(BlueprintPure, Category = "Projecttile Fly Distance")
	static float CalculateDistanceToTarget(const float Time, const float InitialSpeed, const UCurveFloat* const SpeedCurve);

	UFUNCTION(BlueprintPure)
	static bool GetLocationUnderCursorOnGround(FVector& Location, const APlayerController* PlayerController);

	UFUNCTION(BlueprintPure)
	static TArray<FVector> CalculateProjectilePredictionPath(
		bool& bSuccess,
		const TArray<FVector>& PathPoints,
		const float InitialSpeed,
		const UCurveFloat* SpeedCurve,
		const int32 SamplingNum,
		const float SampleDistanceMin);

private:
	//UFUNCTION(BlueprintPure, Category = "Combined Ability Handle", meta = (DisplayName = "Is Valid"))
	//static bool IsCombinedAbilityHandleValid(const FCombinedAbilityHandle& Handle) { return Handle.IsValid(); }

	//UFUNCTION(BlueprintPure, Category = "Interaction Option", meta = (DisplayName = "Is Valid"))
	//static bool IsInteractionOptionValid(const FInteractionOption& Option) { return Option.IsValid(); }

	/*UFUNCTION(BlueprintPure, Category = "Interaction Query", meta = (DisplayName = "Is Valid"))
	static bool IsInteractionQueryValid(const FInteractionQuery& Query) { return Query.IsValid(); }*/

	//UFUNCTION(BlueprintPure, Category = "CombinedAbilityHandle", meta = (DisplayName = "Get Interaction Option"))
	//static bool GetInteractionOption(UPARAM(ref) FInteractionOptionsBuilder& OptionsBuilder, int32 Index, FInteractionOption& OutOption);

	//UFUNCTION(BlueprintPure, Category = "CombinedAbilityHandle", meta = (DisplayName = "Get Interaction Options"))
	//static void GetInteractionOptions(UPARAM(ref) FInteractionOptionsBuilder& OptionsBuilder, TArray<FInteractionOption>& Options);

	UFUNCTION(BlueprintPure, Category = "Tool|Casting", meta = (DeterminesOutputType = "AbilityClass"))
	static UGameplayAbility* GetGameplayAbilityFromHandle(const UAbilitySystemComponent* ASC, const FGameplayAbilitySpecHandle& Handle, TSubclassOf<UGameplayAbility> AbilityClass, bool& bIsInstance);

	UFUNCTION(BlueprintCallable, Category = "Ability System Component")
	static void InitAbilityActorInfo(UAbilitySystemComponent* AbilitySystemComponent, AActor* InOwnerActor, AActor* InAvatarActor);

	UFUNCTION(BlueprintCallable)
	static void PauseCharacter(ACharacter* Character);

	UFUNCTION(BlueprintCallable)
	static void ResumeCharacter(ACharacter* Character);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	static bool CommitWeaponFire(AWeaponActorBase* Weapon);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	static bool CommitWeaponReload(int32& ClampedMovedAmmo, AWeaponActorBase* Weapon);
};
