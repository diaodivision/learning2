// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WeaponTypes.h"
#include "Delegates/DelegateCombinations.h"
#include "Interface/FreezableInterface.h"
#include "Ability/WeaponOperation/WeaponOperationTypes.h"
#include "GenericTeamAgentInterface.h"
#include "Perception/AIPerceptionTypes.h"
#include "GameplayAbilities/Public/AbilitySystemInterface.h"
#include "Character/CharacterWidgetControllableInterface.h"
#include "HoverReactive/HoverReactiveInterface.h"
#include "InputRecordedDataTypes/RecordedDataDelegates.h"
#include "Ability/Tags/PlayerResponseGameplayTags.h"
#include "FogOfWarMaskedActorInterface.h"
#include "MyCharacterBase.generated.h"

struct FOnAttributeChangeData;
class AWeaponActorBase;
class UMyAttributeSet;
class UMyAbilitySystemComponent;
class USwitchableActorCollection;
class UCharacterWidgetComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCharacterLevelChangedDelegate, int32, OldLevel, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnControlledWeaponChangedDelegate, AWeaponActorBase*, OldWeapon, AWeaponActorBase*, NewWeapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnControlledWeaponMagazineAmmoChangedDelegate, int32, OldAmmo, int32, NewAmmo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnControlledWeaponReserveAmmoChangedDelegate, int32, OldAmmo, int32, NewAmmo);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnSenseUpdatedDelegate, bool bSuccessfullySensed, AMyCharacterBase* Observer, AMyCharacterBase* Enemy);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnWeaponAddedDelegate, AWeaponActorBase* Weapon);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnWeaponRemovedDelegate, AWeaponActorBase* Weapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterDeadDelegate, AMyCharacterBase*, Character);

UENUM(BlueprintType)
enum class ETargetConfirmType : uint8
{
	Confirm,
	Cancel
};

UCLASS(Blueprintable, Blueprinttype)
class LEARNING2_API AMyCharacterBase :
	public ACharacter,
	public IFreezableInterface,
	public IGenericTeamAgentInterface,
	public IAbilitySystemInterface,
	public ICharacterWidgetControllableInterface,
	public IHoverReactiveInterface,
	public IFogOfWarMaskedActorInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyCharacterBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void PostRegisterAllComponents() override;
	virtual void PostUnregisterAllComponents() override;

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	inline UMyAttributeSet* GetAttributeSet() const { return AttributeSet; }

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	AWeaponActorBase* GetControlledWeapon() const;
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	inline EWeaponType GetCurrentWeaponType() const { return WeaponType; }

	UFUNCTION(BlueprintNativeEvent, Category = "Weapon")
	void OnShoot();
	virtual void OnShoot_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Weapon")
	void OnShootStop();
	virtual void OnShootStop_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Weapon")
	virtual void SimulateWeaponTrigger(const bool bIsPress);

	virtual inline void SetGenericTeamId(const FGenericTeamId& InTeamID) override { TeamID = InTeamID; }

	UFUNCTION(BlueprintPure, Category = "Team|ID")
	virtual inline FGenericTeamId GetGenericTeamId() const { return TeamID; }

	UFUNCTION(BlueprintPure, Category = "Team|Attitude")
	inline ETeamAttitude::Type K2_GetTeamAttitudeTowards(const AActor* Other) const { return Other ? GetTeamAttitudeTowards(*Other) : ETeamAttitude::Neutral; }

	virtual void GetActorEyesViewPoint(FVector& Location, FRotator& Rotation) const override;

	UFUNCTION(BlueprintNativeEvent)
	void OnEnemySensed(AMyCharacterBase* Enemy);
	virtual void OnEnemySensed_Implementation(AMyCharacterBase* Enemy);

	UFUNCTION(BlueprintNativeEvent)
	void OnEnemyDisappear(const AMyCharacterBase* Enemy);
	virtual void OnEnemyDisappear_Implementation(const AMyCharacterBase* Enemy);

	TArray<AWeaponActorBase*> GetWeapons() const;

	UFUNCTION(BlueprintCallable)
	AWeaponActorBase* SwitchWeaponByIndex(const int32 Index);

	UFUNCTION(BlueprintCallable)
	AWeaponActorBase* SwitchWeaponByActor(AWeaponActorBase* Weapon);

	virtual void ShowCharacterWidget_Implementation(bool bIsShow) override;
	virtual bool IsCharacterWidgetVisible_Implementation() const override;

	UFUNCTION(BlueprintCallable)
	virtual void UpdateCharacterWidget();

	UFUNCTION(BlueprintPure)
	TOptional<float> GetCurrentHealth() const;
	UFUNCTION(BlueprintPure)
	TOptional<float> GetHealthMax() const;
	UFUNCTION(BlueprintPure)
	bool IsDead() const;

	virtual void OnHovered_Implementation(float HoveredDelta) override;
	virtual void OnHoverReleased_Implementation() override;

	virtual void Freeze_Implementation() override;
	virtual void Unfreeze_Implementation() override;
	virtual FORCEINLINE bool IsFreezing_Implementation() override { return bIsFreezing; };

	virtual void UpdateFogOfWarTexture_Implementation(UTexture2D* FogOfWarTexture) override;

	virtual void EnableTeamDuty(const bool bIsEnable);

protected:
	UFUNCTION(BlueprintNativeEvent, Category = "Weapon")
	void OnWeaponMagazineAmmoChanged(const AWeaponActorBase* Weapon, int32 OldMagazineAmmo, int32 NewMagazineAmmo);
	virtual void OnWeaponMagazineAmmoChanged_Implementation(const AWeaponActorBase* Weapon, int32 OldMagazineAmmo, int32 NewMagazineAmmo);

	UFUNCTION(BlueprintNativeEvent, Category = "Weapon")
	void OnWeaponReserveAmmoChanged(const AWeaponActorBase* Weapon, int32 OldReserveAmmo, int32 NewReserveAmmo);
	void OnWeaponReserveAmmoChanged_Implementation(const AWeaponActorBase* Weapon, int32 OldReserveAmmo, int32 NewReserveAmmo);

	UFUNCTION(BlueprintNativeEvent, Category = "Weapon")
	void OnWeaponAdded(AWeaponActorBase* AddedWeapon);
	virtual void OnWeaponAdded_Implementation(AWeaponActorBase* AddedWeapon);
	UFUNCTION()
	void OnWeaponAdded_Internal(AActor* AddedActor);

	UFUNCTION(BlueprintNativeEvent, Category = "Weapon")
	void OnWeaponRemoved(AWeaponActorBase* RemovedWeapon);
	virtual void OnWeaponRemoved_Implementation(AWeaponActorBase* RemovedWeapon);
	UFUNCTION()
	void OnWeaponRemoved_Internal(AActor* RemovedWeapon);

	UFUNCTION(BlueprintNativeEvent, Category = "Weapon")
	void OnControlledWeaponChanged(AWeaponActorBase* OldWeapon, AWeaponActorBase* NewWeapon);
	virtual void OnControlledWeaponChanged_Implementation(AWeaponActorBase* OldWeapon, AWeaponActorBase* NewWeapon);
	UFUNCTION()
	void OnControlledWeaponChanged_Internal(AActor* OldActor, AActor* NewActor);

	virtual void OnCharacterHealthChanged(const FOnAttributeChangeData& OnAttributeChangeData);
	UFUNCTION(BlueprintImplementableEvent, Category = "Character Attribute | HP")
	void K2_OnCharacterHealthChanged(const float NewHealth);

	void OnWeaponAmmoChanged(const FOnAttributeChangeData& OnAttributeChangeData);

	virtual void InitializeDelegates();
	virtual void DeinitializeDelegates();

	UFUNCTION()
	bool FindWeaponByPredicate(const UObject* Object) const;

	UFUNCTION()
	void OnSenseUpdated(AActor* Enemy, FAIStimulus Stimulus);

	virtual void RegisterGameplayTagEvent();
	virtual void OnResponseTagCountChanged(const FGameplayTag Tag, const int32 NewCount);

	virtual void OnStunTagCountChanged(const ETagCountChangeType TagCountChangeType);
	UFUNCTION(BlueprintImplementableEvent, Category = "Character Response", meta = (DisplayName = "On StunTag Count Changed"))
	void K2_OnStunTagCountChanged(const ETagCountChangeType TagCountChangeType);

	virtual void OnBlindTagCountChanged(const ETagCountChangeType TagCountChangeType);
	UFUNCTION(BlueprintImplementableEvent, Category = "Character Response", meta = (DisplayName = "On BlindTag Count Changed"))
	void K2_OnBlindTagCountChanged(const ETagCountChangeType TagCountChangeType);

	void OnCharacterDeath();
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Character Death"))
	void K2_OnCharacterDeath();

private:
	void CreateAndSetupComponents();

	virtual void OnCharacterDeath_Internal();

private:
	EWeaponSlot CurrentTargetWeaponSlot{ EWeaponSlot::None };

public:
	UPROPERTY(BlueprintAssignable, Category = "Level")
	FOnCharacterLevelChangedDelegate OnCharacterLevelChangedDelegate;

	FOnSenseUpdatedDelegate OnSenseUpdatedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FOnControlledWeaponChangedDelegate OnControlledWeaponChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Weapon|Ammo")
	FOnControlledWeaponMagazineAmmoChangedDelegate OnControlledWeaponMagazineAmmoChangedDelegate;
	UPROPERTY(BlueprintAssignable, Category = "Weapon|Ammo")
	FOnControlledWeaponReserveAmmoChangedDelegate OnControlledWeaponReserveAmmoChangedDelegate;

	FOnWeaponAddedDelegate OnWeaponAddedDelegate;
	FOnWeaponRemovedDelegate OnWeaponRemovedDelegate;

	FOnCharacterDeadDelegate OnCharacterDeadDelegate;

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UMyAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AttributeSet")
	TObjectPtr<UMyAttributeSet> AttributeSet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "Weapons")
	TObjectPtr<USwitchableActorCollection> Weapons;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	EWeaponType WeaponType;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Character")
	int32 Level{ 1 };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Socket")
	FName LeftHandleWeaponSocketName{ TEXT("LeftWeapon") };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Socket")
	FName RightHandleWeaponSocketName{ TEXT("RightWeapon") };

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCharacterWidgetComponent> CharacterWidgetComponent;

	TMap<FGameplayTag, int32> ResponseTagCountMap;

	UPROPERTY(BlueprintReadWrite, Category = "Fog Of War Mask")
	bool bIsFogOfWarMaskInitialized{ false };

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Team|ID", meta = (AllowPrivateAccess = true))
	FGenericTeamId TeamID;

	bool bIsFreezing{ false };
};
