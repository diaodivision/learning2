// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Switchable/SwitchableInterface.h"
#include "GameplayAbilitySpecHandle.h"
#include "Interface/WeaponInterface.h"
#include "WeaponTypes.h"
#include "Delegates/DelegateCombinations.h"
#include "GameplayTagContainer.h"
#include "WeaponActorBase.generated.h"

class UGameplayAbility;
class USkeletalMeshComponent;
class UAbilitySystemComponent;
class ABulletBase;
class UGameplayEffect;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMagazineAmmoChangedDelegate, const AWeaponActorBase*, Weapon, int32, OldMagazineAmmo, int32, NewMagazineAmmo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnReserveAmmoChangedDelegate, const AWeaponActorBase*, Weapon, int32, OldReserveAmmo, int32, NewReserveAmmo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponAbilityEndedDelegate, const FGameplayAbilitySpecHandle&, Handle);

UCLASS(NotBlueprintable, BlueprintType, Abstract)
class LEARNING2_API AWeaponActorBase : public AActor, public ISwitchableInterface, public IWeaponInterface
{
	GENERATED_BODY()

	friend class UWeaponActorBlueprintLibrary;

public:
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	// Sets default values for this actor's properties
	AWeaponActorBase();

	//virtual void OnAdded_Implementation() {};
	virtual void OnAdded_Implementation(const USwitchableCollection* Container) override {}
	virtual void OnRemoved_Implementation(const USwitchableCollection* Container) override {}
	virtual void OnControl_Implementation(UObject* InOwner) override;
	virtual void OnControlReleased_Implementation() override;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Ability")
	virtual bool Fire() override;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Ability")
	virtual bool ExecuteFireOnce();

	//UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Ability")
	virtual bool Reload() const { return false; }

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Weapon")
	int32 CalculateReloadAmount() const;
	virtual int32 CalculateReloadAmount_Implementation() const;

	UFUNCTION(BlueprintCallable, Category = "Attribute")
	inline int32 GetWeaponLevel() const { return WeaponLevel; };
	virtual inline void SetWeaponLevel(int32 NewLevel)
	{
		if (NewLevel > 0)
		{
			const int32 OldLevel{ WeaponLevel };
			WeaponLevel = NewLevel;

			OnWeaponLevelChanged(OldLevel, NewLevel);
		}
	}

	inline FName GetWeaponName() const { return WeaponName; };

	//UFUNCTION(BlueprintCallable, Category = "Ability")
	//void UpdateAbilityLevelBySpecHandle(FGameplayAbilitySpecHandle Handle, int32 Level) const;

	//UFUNCTION(BlueprintCallable, Category = "Ability")
	//void UpdateAbilitiesLevel(int32 Level) const;

	//UFUNCTION(BlueprintCallable, Category = "Attribute")
	//void UpdateWeaponAccuracy(float Accuracy);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attribute")
	virtual inline FWeaponAttributeData GetWeaponAttribute() const
	{
		FWeaponAttributeData Result;
		Result.Damage = GetDamage();
		Result.Penetration = GetPenetration();
		Result.BulletSpawnsOnFire = GetBulletSpawnsOnFire();
		Result.FireCost = GetFireCost();
		Result.FireRate = GetFireRate();
		Result.ReloadTime = GetReloadTime();
		Result.MagazineAmmo = GetMagazineAmmo();
		Result.MagazineAmmoMax = GetMagazineAmmoMax();
		Result.ChamberCapacity = GetChamberCapacity();
		Result.ReserveAmmo = GetReserveAmmo();
		Result.bIsAutomaticWeapon = IsAutomaticWeapon();
		Result.MuzzleSpeed = GetMuzzleSpeed();
		Result.Accuracy = GetAccuracy();

		return Result;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attribute")
	virtual inline float GetDamage() const { return FMath::Max(0.f, WeaponAttribute.Damage); }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attribute")
	virtual inline float GetPenetration() const { return FMath::Max(0.f, WeaponAttribute.Penetration); }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attribute")
	virtual inline int32 GetBulletSpawnsOnFire() const { return FMath::Max(0.f, WeaponAttribute.BulletSpawnsOnFire); }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attribute")
	virtual inline int32 GetFireCost() const { return FMath::Max(0.f, WeaponAttribute.FireCost); }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attribute")
	virtual inline float GetFireRate() const { return FMath::Max(0.f, WeaponAttribute.FireRate); }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attribute")
	virtual inline float GetReloadTime() const { return FMath::Max(0.f, WeaponAttribute.ReloadTime); }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attribute")
	virtual inline int32 GetMagazineAmmo() const { return FMath::Max(0, WeaponAttribute.MagazineAmmo); }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attribute")
	virtual inline int32 GetMagazineAmmoMax() const { return FMath::Max(0, WeaponAttribute.MagazineAmmoMax); }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attribute")
	virtual inline int32 GetChamberCapacity() const { return FMath::Max(0, WeaponAttribute.ChamberCapacity); }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attribute")
	virtual inline int32 GetReserveAmmo() const { return FMath::Max(0, WeaponAttribute.ReserveAmmo); }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attribute")
	virtual inline float GetMuzzleSpeed() const { return FMath::Max(0.f, WeaponAttribute.MuzzleSpeed); }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attribute")
	virtual inline bool IsAutomaticWeapon() const { return WeaponAttribute.bIsAutomaticWeapon; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attribute")
	virtual inline float GetAccuracy() const { return FMath::Max(0.f, WeaponAttribute.Accuracy); }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attribute")
	inline TSoftObjectPtr<UTexture2D> GetWeaponIcon() const { return WeaponAttribute.Icon; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attribute")
	inline FString GetWeaponDescription() const { return WeaponAttribute.Description; }

	virtual inline void SetCurrentMagazineAmmo(const int32 NewMagazineAmmo)
	{
		const int32 OldMagazineAmmo{ WeaponAttribute.MagazineAmmo };
		WeaponAttribute.MagazineAmmo = FMath::Max(0.f, NewMagazineAmmo);

		OnMagazineAmmoChangedDelegate.Broadcast(this, OldMagazineAmmo, NewMagazineAmmo);
	};
	virtual inline void SetReserveAmmo(const int32 NewReserveAmmo)
	{
		const int32 OldReserveAmmo{ WeaponAttribute.ReserveAmmo };
		WeaponAttribute.ReserveAmmo = FMath::Max(0.f, NewReserveAmmo);

		OnReserveAmmoChangedDelegate.Broadcast(this, OldReserveAmmo, NewReserveAmmo);
	};

	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintNativeEvent, Category = "Attribute")
	bool CanReload() const;
	virtual inline bool CanReload_Implementation() const
	{
		return GetReserveAmmo() > 0 && GetMagazineAmmo() < GetMagazineAmmoMax() + GetChamberCapacity();
	}

	inline EWeaponType GetWeaponType() const { return WeaponType; }
	inline EWeaponSlot GetWeaponSlot() const { return WeaponSlot; }

	virtual FORCEINLINE TSubclassOf<ABulletBase> GetWeaponBulletClass() const { return BulletClass; }
	virtual FORCEINLINE TSubclassOf<UGameplayEffect> GetWeaponEffectClass() const { return EffectClass; }

	UFUNCTION(BlueprintNativeEvent)
	void OnShoot();
	virtual void OnShoot_Implementation();

	UFUNCTION(BlueprintNativeEvent)
	void OnShootStop();
	virtual void OnShootStop_Implementation();

	FGameplayTag GetOnShootTag() const;
	FGameplayTag GetShootCountTag() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure, Category = "Reload")
	bool ShouldRackOnReload() const;
	inline virtual bool ShouldRackOnReload_Implementation() const { return GetMagazineAmmo() == 0; }

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure = false, Category = "Bullet")
	AActor* SpawnBullet() const;
	virtual inline AActor* SpawnBullet_Implementation() const { return nullptr; }

	virtual FVector RandomSpread() const;

	UFUNCTION(BlueprintCallable, Category = "Controller")
	inline bool IsOnControl() const { return bOnControl; }

	UFUNCTION(BlueprintCallable, Category = "Ability")
	inline FGameplayAbilitySpecHandle GetWeaponFireHandle() const { return FireAbilityHandle; };

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void InitializeDelegates();
	virtual void DeinitializeDelegates() const;

	virtual void OnWeaponLevelChanged(const int32 OldLevel, const int32 NewLevel);

	UFUNCTION(BlueprintCallable, Category = "Attribute")
	virtual void InitilizeWeaponAttribute();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnAbilityEnded"))
	void K2_OnAbilityEnded(const FAbilityEndedData& AbilityEndedData);

	virtual void OnAbilityEnded(const FAbilityEndedData& AbilityEndedData);

	virtual void InstantiateAbilityOnBeginPlay();

public:
	UPROPERTY(BlueprintAssignable, Category = "Attribute")
	FOnMagazineAmmoChangedDelegate OnMagazineAmmoChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Attribute")
	FOnReserveAmmoChangedDelegate OnReserveAmmoChangedDelegate;

protected:
	UPROPERTY(BlueprintAssignable)
	FOnWeaponAbilityEndedDelegate OnWeaponAbilityEndedDelegate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data Table")
	FWeaponAttributeDataTable WeaponAttributeDataTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	EWeaponType WeaponType{ EWeaponType::AR4 };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	EWeaponSlot WeaponSlot{ EWeaponSlot::Ammo1 };

	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	TWeakObjectPtr<UAbilitySystemComponent> AvatarAbilitySystemComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	FWeaponAbilityInfo FireAbilityClass;

	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	FGameplayAbilitySpecHandle FireAbilityHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	TSet<FWeaponAbilityInfo> WeaponAbilities;

	UPROPERTY(BlueprintReadWrite, Category = "Ability")
	TMap<FGameplayAbilitySpecHandle, FWeaponAbilityInfo> WeaponAbilityHandles;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	FName WeaponName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	FName WeaponSocket{ "RightWeapon" };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attribute")
	int32 WeaponLevel;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bone")
	FName WeaponMagazineBoneName{ TEXT("b_gun_mag") };

	FWeaponAttributeData WeaponAttribute;

	bool bOnControl{ false };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bullet Class")
	TSubclassOf<ABulletBase> BulletClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ExposeOnSpawn = "true"), Category = "Effect")
	TSubclassOf<UGameplayEffect> EffectClass;

private:
	//// 1. 定义材质实例的软引用（允许在编辑器中指定任何 MIC）
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Materials", meta = (AllowPrivateAccess = true))
	//TSoftObjectPtr<UMaterialInstanceConstant> SoftIconMaterial;

	//// 2. 用于存储运行时生成的动态材质实例 (MID)
	//UPROPERTY()
	//TObjectPtr<UMaterialInstanceDynamic> IconMaterial;
};
