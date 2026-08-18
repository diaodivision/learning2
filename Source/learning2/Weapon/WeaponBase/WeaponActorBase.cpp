// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponActorBase.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "BlueprintFunctionLibrary/WeaponActorBlueprintLibrary.h"
#include "Ability/WeaponOperation/Base/WeaponFireAbilityBase.h"
#include "FogOfWarSubsystem.h"
#include "FogOfWarComponentStatics.h"

// Sets default values
AWeaponActorBase::AWeaponActorBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;
}

void AWeaponActorBase::OnControl_Implementation(UObject* InOwner)
{
	ACharacter* Character{ Cast<ACharacter>(InOwner) };
	if (!Character) { return; }
	SetOwner(Character);
	//AvatarAbilitySystemComponent = Cast<UAbilitySystemComponent>(Character->FindComponentByClass(UAbilitySystemComponent::StaticClass()));
	//AvatarAbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Character);

	USkeletalMeshComponent* CharacterMesh = Character->GetMesh();
	WeaponMesh->AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);

	if (!AvatarAbilitySystemComponent.IsValid()) { return; }

	{
		if (FireAbilityClass.InstancingPolicy == EWeaponAbilityInstancingPolicy::InstancedOnPossession)
		{
			FireAbilityHandle = AvatarAbilitySystemComponent->K2_GiveAbility(FireAbilityClass.AbilityClass, GetWeaponLevel());
		}

		for (const FWeaponAbilityInfo& AbilityInfo : WeaponAbilities)
		{
			if (AbilityInfo.InstancingPolicy == EWeaponAbilityInstancingPolicy::InstancedOnPossession)
			{
				WeaponAbilityHandles.Add(AvatarAbilitySystemComponent->K2_GiveAbility(AbilityInfo.AbilityClass, GetWeaponLevel()), AbilityInfo);
			}
		}
	}

	InitializeDelegates();

	//RootComponent->SetVisibility(true);
	SetActorHiddenInGame(false);
	bOnControl = true;
}

void AWeaponActorBase::OnControlReleased_Implementation()
{
	if (GetOwner()->IsA<ACharacter>())
	{
		WeaponMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

		SetOwner(nullptr);
	}

	//RootComponent->SetVisibility(false);

	SetActorHiddenInGame(true);
	bOnControl = false;
	DeinitializeDelegates();

	{
		if (!AvatarAbilitySystemComponent.IsValid()) { return; }

		if (FireAbilityHandle.IsValid() && FireAbilityClass.InstancingPolicy == EWeaponAbilityInstancingPolicy::InstancedOnPossession)
		{
			AvatarAbilitySystemComponent->ClearAbility(FireAbilityHandle);
		}
		FireAbilityHandle = FGameplayAbilitySpecHandle{};

		for (auto It{ WeaponAbilityHandles.CreateIterator() }; It; ++It)
		{
			const FGameplayAbilitySpecHandle& Handle{ It->Key };
			const FWeaponAbilityInfo& AbilityInfo{ It->Value };
			if (!Handle.IsValid() || AbilityInfo.InstancingPolicy == EWeaponAbilityInstancingPolicy::InstancedOnPossession)
			{
				AvatarAbilitySystemComponent->ClearAbility(Handle);
				It.RemoveCurrent();
			}
		}

		if (const FGameplayTag ShootCountTag{ GetShootCountTag() }; AvatarAbilitySystemComponent->HasMatchingGameplayTag(ShootCountTag))
		{
			AvatarAbilitySystemComponent->RemoveLooseGameplayTag(ShootCountTag, AvatarAbilitySystemComponent->GetTagCount(ShootCountTag));
		}
	}
}

bool AWeaponActorBase::Fire()
{
	static double LastActivate{ 0.f };
	if (AvatarAbilitySystemComponent.IsValid())
	{
		const bool bSucceed = AvatarAbilitySystemComponent->TryActivateAbility(FireAbilityHandle);

		if (bSucceed)
		{
			LastActivate = GetWorld()->GetTimeSeconds();
		}
		return bSucceed;
	}

	return false;
}

bool AWeaponActorBase::ExecuteFireOnce()
{
	OnShoot();
	UE_LOG(LogTemp, Error, TEXT("AvatarAbilitySystemComponent->HasMatchingGameplayTag(GetOnShootTag()) %d"), AvatarAbilitySystemComponent->HasMatchingGameplayTag(GetOnShootTag()));
	UE_LOG(LogTemp, Error, TEXT("AvatarAbilitySystemComponent %d"), AvatarAbilitySystemComponent->GetUniqueID());
	const bool bSucceed = AvatarAbilitySystemComponent->TryActivateAbility(FireAbilityHandle);
	OnShootStop();

	return bSucceed;
}

//bool AWeaponActorBase::Reload() const
//{
//	if (AvatarAbilitySystemComponent.IsValid()) { return  AvatarAbilitySystemComponent->TryActivateAbility(ReloadAbilityHandle);; }
//
//	return false;
//}

int32 AWeaponActorBase::CalculateReloadAmount_Implementation() const
{
	if (!CanReload()) { return 0; }

	int32 MagazineAmmoRequests{ GetMagazineAmmoMax() - GetMagazineAmmo() };
	if (GetMagazineAmmo() > 0) { MagazineAmmoRequests += GetChamberCapacity(); }

	return FMath::Min(MagazineAmmoRequests, GetReserveAmmo());

	//const int32 WeaponLoadedAmmoMax{ GetMagazineAmmoMax() + GetChamberCapacity };

	//const int32 NeedAmmoAmount{ WeaponLoadedAmmoMax - WeaponAttribute.MagazineAmmo };

	//const int32 ReloadAmount{ FMath::Min(NeedAmmoAmount, WeaponAttribute.ReserveAmmo) };

	//if (!ensure(ReloadAmount >= 0)) { return 0; }

	//if (ReloadAmount > WeaponAttribute.MagazineAmmoMax && WeaponAttribute.MagazineAmmo == 0) { return WeaponAttribute.MagazineAmmoMax; }
	//return ReloadAmount;
}

void AWeaponActorBase::OnShoot_Implementation()
{
	if (!AvatarAbilitySystemComponent.IsValid()) { return; }

	if (const FGameplayTag OnShootTag{ GetOnShootTag() }; OnShootTag.IsValid()) { AvatarAbilitySystemComponent->AddLooseGameplayTag(OnShootTag); }
}

void AWeaponActorBase::OnShootStop_Implementation()
{
	if (!AvatarAbilitySystemComponent.IsValid()) { return; }

	if (const FGameplayTag OnShootTag{ GetOnShootTag() }; OnShootTag.IsValid() && AvatarAbilitySystemComponent->HasMatchingGameplayTag(OnShootTag))
	{
		AvatarAbilitySystemComponent->RemoveLooseGameplayTag(OnShootTag, AvatarAbilitySystemComponent->GetTagCount(OnShootTag));
	}

	if (const FGameplayTag ShootCountTag{ GetShootCountTag() }; ShootCountTag.IsValid() && AvatarAbilitySystemComponent->HasMatchingGameplayTag(ShootCountTag))
	{
		AvatarAbilitySystemComponent->RemoveLooseGameplayTag(ShootCountTag, AvatarAbilitySystemComponent->GetTagCount(ShootCountTag));
	}
}

FGameplayTag AWeaponActorBase::GetOnShootTag() const
{
	const UWeaponFireAbilityBase* WeaponFireAbilityCDO = Cast<UWeaponFireAbilityBase>(FireAbilityClass.AbilityClass.GetDefaultObject());
	if (WeaponFireAbilityCDO)
	{
		return WeaponFireAbilityCDO->GetOnShootTag();
	}

	return FGameplayTag{};
}

FGameplayTag AWeaponActorBase::GetShootCountTag() const
{
	const UWeaponFireAbilityBase* WeaponFireAbilityCDO = Cast<UWeaponFireAbilityBase>(FireAbilityClass.AbilityClass.GetDefaultObject());
	if (WeaponFireAbilityCDO)
	{
		return WeaponFireAbilityCDO->GetShotCountTag();
	}

	return FGameplayTag{};
}

FVector AWeaponActorBase::RandomSpread() const
{
	const float Spread = UWeaponActorBlueprintLibrary::RandomWeaponSpreadRadius();

	if (const AActor* Actor = GetOwner())
	{
		const FVector Offset{ Actor->GetActorForwardVector().RotateAngleAxis(90.f, FVector::UpVector) * Spread };

		return (Actor->GetActorForwardVector() * GetAccuracy() + Offset).GetSafeNormal();
	}

	return FVector::Zero();
}

void AWeaponActorBase::UpdateFogOfWarTexture_Implementation(UTexture2D* FogOfWarTexture)
{
	const EDoInitialize DoInitialize{ bIsFogOfWarMaskInitialized ? EDoInitialize::No : EDoInitialize::Yes };
    UpdateFogOfWarTexture_DefaultImplementation(FogOfWarTexture, WeaponMesh, bIsFogOfWarMaskInitialized, DoInitialize);
}

// Called when the game starts or when spawned
void AWeaponActorBase::BeginPlay()
{
	Super::BeginPlay();

	AvatarAbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());

	check(WeaponAttributeDataTable.IsValid());
	InitilizeWeaponAttribute();

	checkf(FireAbilityClass, TEXT("Need to specify FireAbilityClass"));

	InstantiateAbilityOnBeginPlay();
}

void AWeaponActorBase::InitializeDelegates()
{
	if (!AvatarAbilitySystemComponent.IsValid()) { return; }

	AvatarAbilitySystemComponent->OnAbilityEnded.AddUObject(this, &AWeaponActorBase::OnAbilityEnded);

	if (UFogOfWarSubsystem* FogOfWarSubsystem{ UFogOfWarComponentStatics::GetFogOfWarSubsystem(this) })
	{
		FogOfWarSubsystem->OnFogOfWarTextureUpdatedDelegate.AddUniqueDynamic(this, &AWeaponActorBase::UpdateFogOfWarTexture);
	}
}

void AWeaponActorBase::DeinitializeDelegates() const
{
	if (!AvatarAbilitySystemComponent.IsValid()) { return; }

	AvatarAbilitySystemComponent->OnAbilityEnded.RemoveAll(this);

	if (UFogOfWarSubsystem* FogOfWarSubsystem{ UFogOfWarComponentStatics::GetFogOfWarSubsystem(this) })
	{
		FogOfWarSubsystem->OnFogOfWarTextureUpdatedDelegate.RemoveAll(this);
	}
}

void AWeaponActorBase::OnWeaponLevelChanged(const int32 OldLevel, const int32 NewLevel)
{
	if (NewLevel <= 0) { return; }

	float NewAccuracy{ 0.f };
	if (ensure(UWeaponActorBlueprintLibrary::GetWeaponAccuracy(NewAccuracy, this)))
	{
		WeaponAttribute.Accuracy = NewAccuracy;
	}
}

void AWeaponActorBase::InitilizeWeaponAttribute()
{
	if (FWeaponAttributeData Result; ensure(UWeaponActorBlueprintLibrary::GetWeaponAttributeFromDataTable(Result, this)))
	{
		WeaponAttribute = Result;
	}
}

void AWeaponActorBase::OnAbilityEnded(const FAbilityEndedData& AbilityEndedData)
{
	if (AbilityEndedData.bWasCancelled) { return; }
	if (!AvatarAbilitySystemComponent.IsValid() || !AvatarAbilitySystemComponent->FindAbilitySpecFromHandle(AbilityEndedData.AbilitySpecHandle)) { return; }

	OnWeaponAbilityEndedDelegate.Broadcast(AbilityEndedData.AbilitySpecHandle);
	K2_OnAbilityEnded(AbilityEndedData);
}

void AWeaponActorBase::InstantiateAbilityOnBeginPlay()
{
	if (FireAbilityClass.InstancingPolicy == EWeaponAbilityInstancingPolicy::InstancedOnAddition)
	{
		FireAbilityHandle = AvatarAbilitySystemComponent->K2_GiveAbility(FireAbilityClass.AbilityClass, GetWeaponLevel());
	}

	UE_LOG(LogTemp, Error, TEXT("this: %s"), *GetNameSafe(this));

	for (const FWeaponAbilityInfo& AbilityInfo : WeaponAbilities)
	{
		if (AbilityInfo.InstancingPolicy == EWeaponAbilityInstancingPolicy::InstancedOnAddition)
		{
			WeaponAbilityHandles.Add(AvatarAbilitySystemComponent->K2_GiveAbility(AbilityInfo.AbilityClass, GetWeaponLevel()), AbilityInfo);
		}
	}
}

//void AWeaponActorBase::UpdateAbilityLevelBySpecHandle(FGameplayAbilitySpecHandle Handle, int32 Level) const
//{
//	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromOwner();
//	if (!ASC) { return; }
//
//	FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Handle);
//	if (Spec) { Spec->Level = Level; }
//}

//void AWeaponActorBase::UpdateAbilitiesLevel(int32 Level) const
//{
//	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromOwner();
//	if (!ASC) { return; }
//
//	for (FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities()) { Spec.Level = Level; }
//}

//void AWeaponActorBase::UpdateWeaponAccuracy(float Accuracy)
//{
//	WeaponAttribute.Accuracy = Accuracy;
//}

//void AWeaponActorBase::OnRemoved_Implementation()
//{
//}
