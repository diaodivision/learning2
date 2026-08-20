#include "PlayerCharacterBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "FogOfWarComponent.h"
#include "InputRecordComponent.h"
#include "Controller/MyPlayerController.h"
#include "Ability/AttributeSet/MyAttributeSet.h"
#include "Ability/AbilitySystemComponent/MyAbilitySystemComponent.h"
#include "MyHUD.h"
#include "Character/CharacterWidgetTypes.h"
#include "Character/CharacterWidgetComponent.h"
#include "Switchable/SwitchableActorCollection.h"
#include "WeaponBase/WeaponActorBase.h"
#include "GameplayEffectTypes.h"
#include "InputRecordedDataTypes/RecordedDataVisitor.h"
#include "InputRecordedDataTypes/RecordedDataTypes.h"
//#include "Ability/MyGameplayAbilityType.h"
#include "InputRecord/Location/RecordedLocationVisualizationComponent.h"
#include "BehaviorTree/BehaviorTreeStatics.h"
#include "Battle/BattleSubsystem.h"
#include "PredictionLineProvider/PredictionLineProviderInterface.h"
#include "Targeting/TargetingInstigatorTypes.h"

APlayerCharacterBase::APlayerCharacterBase() : Super()
{
	CreateAndSetupComponents();
}

void APlayerCharacterBase::UpdateCharacterWidget()
{
	if (!ICharacterWidgetControllableInterface::Execute_IsCharacterWidgetVisible(this))
	{
		CharacterWidgetComponent->UpdateUICharacterInfo({});

		return;
	}

	if (const AWeaponActorBase * ControlledWeapon{ GetControlledWeapon() })
	{
		TArray<FUICharacterWeaponInfo> UICharacterWeaponInfos;
		for (auto It{ Weapons->CreateIterator<AWeaponActorBase>() }; It; ++It)
		{
			const AWeaponActorBase* Weapon{ *It };
			if (Weapon == ControlledWeapon) { continue; }

			UICharacterWeaponInfos.Add(FUICharacterWeaponInfo{ Weapon->GetWeaponIcon(), Weapon->GetMagazineAmmo(), Weapon->GetMagazineAmmoMax() });
		}


		FUIPlayerCharacterInfo UIPlayerCharacterInfo{
			FUICharacterHealthState{GetCurrentHealth(),GetHealthMax()},
			FUICharacterWeaponInfo{ ControlledWeapon->GetWeaponIcon(), ControlledWeapon->GetMagazineAmmo(), ControlledWeapon->GetMagazineAmmoMax() },
			ControlledWeapon->GetWeaponDescription(),
			UICharacterWeaponInfos
		};
		CharacterWidgetComponent->UpdateUIPlayerCharacterInfo(UIPlayerCharacterInfo);
	}
	else { CharacterWidgetComponent->UpdateUIPlayerCharacterInfo({}); }
}

void APlayerCharacterBase::SetTargetingState(ETargetingState TargetingState)
{
	switch (TargetingState)
	{
	case ETargetingState::Targeting:
		if (IPredictionLineProviderInterface * Weapon{ Cast<IPredictionLineProviderInterface>(GetControlledWeapon()) })
		{
			Weapon->HidePredictionLine();
		}

		break;

	case ETargetingState::Confirm: [[fallthrough]];
	case ETargetingState::Cancel:
		if (AWeaponActorBase * Weapon{ Cast<AWeaponActorBase>(GetControlledWeapon()) })
		{
			if (IPredictionLineProviderInterface * PredictionLineProviderInterface{ Cast<IPredictionLineProviderInterface>(GetControlledWeapon()) })
			{
				PredictionLineProviderInterface->ShowPredictionLine({ Weapon->GetWeaponBulletClass(), this });
			}
		}
		break;
	}

	//OnTargetingStateChangedDelegate.Broadcast(TargetingState);
}

void APlayerCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AMyPlayerController * PlayerController{ Cast<AMyPlayerController>(NewController) })
	{
		//APlayerState* PlayerState{ GetPlayerState() };
		AMyHUD* HUD{ Cast<AMyHUD>(PlayerController->GetHUD()) };

		if (GetPlayerState() && HUD)
		{
			HUD->SetUpHUD({ PlayerController, AttributeSet, GetPlayerState(), AbilitySystemComponent, InputRecordComponent, this });
		}

		PlayerController->OnReceiveMoveInputDelegate.AddUObject(this, &APlayerCharacterBase::CancelRewindingState);
		PlayerController->OnReceiveShootInputDelegate.AddUObject(this, &APlayerCharacterBase::CancelRewindingState);
	}
	else if (NewController != nullptr)
	{
		UBehaviorTreeStatics::SetSelfActor(this);

		const TArray<const AWeaponActorBase*> WeaponsList{ GetWeapons() };
		for (int32 i = 0; i < 2; i++)
		{
			if (!WeaponsList.IsValidIndex(i)) { break; }

			const AWeaponActorBase* Weapon{ WeaponsList[i] };

			UBehaviorTreeStatics::SetWeaponMagazineAmmo(this, Weapon->GetMagazineAmmo(), Weapon->GetWeaponSlot());
			UBehaviorTreeStatics::SetWeaponMaxMagazineAmmo(this, Weapon->GetMagazineAmmoMax(), Weapon->GetWeaponSlot());
			if (Weapon->IsOnControl()) { UBehaviorTreeStatics::SetControlledWeaponSlot(Weapon->GetWeaponSlot(), this); }
		}

		ACharacter* EnemyCharacter{ nullptr };
		ACharacter* SensedEnemyCharacter{ nullptr };
		if (UBattleSubsystem* BattleSubsystem = ULocalPlayer::GetSubsystem<UBattleSubsystem>(GetWorld()->GetFirstLocalPlayerFromController()))
		{
			SensedEnemyCharacter = Cast<ACharacter>(BattleSubsystem->GetOneTeamSensedActor(GetGenericTeamId()));

			if (SensedEnemyCharacter)
			{
				FHitResult HitResult;

				FCollisionObjectQueryParams Params;
				Params.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldStatic);
				Params.AddObjectTypesToQuery(ECollisionChannel::ECC_Pawn);
				Params.AddObjectTypesToQuery(ECollisionChannel::ECC_Destructible);
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

void APlayerCharacterBase::UnPossessed()
{
	Super::UnPossessed();

	if (AMyPlayerController* PlayerController{ Cast<AMyPlayerController>(GetController()) })
	{
		PlayerController->OnReceiveMoveInputDelegate.RemoveAll(this);
		PlayerController->OnReceiveShootInputDelegate.RemoveAll(this);
	}
}

void APlayerCharacterBase::InitializeDelegates()
{
	Super::InitializeDelegates();

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetCurrentHealthAttribute()).AddUObject(this, &APlayerCharacterBase::OnCharacterHealthChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMaxHealthAttribute()).AddUObject(this, &APlayerCharacterBase::OnCharacterHealthChanged);

	InputRecordComponent->OnOperationPreviewDelegate.AddUObject(this, &APlayerCharacterBase::OnInputRecordOperationPreview);

	if (AMyPlayerController* PlayerController{ Cast<AMyPlayerController>(GetController()) })
	{
		PlayerController->OnReceiveMoveInputDelegate.AddUObject(this, &APlayerCharacterBase::CancelRewindingState);
		PlayerController->OnReceiveShootInputDelegate.AddUObject(this, &APlayerCharacterBase::CancelRewindingState);
	}
}

void APlayerCharacterBase::DeinitializeDelegates()
{
	Super::DeinitializeDelegates();

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetCurrentHealthAttribute()).RemoveAll(this);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMyAttributeSet::GetMaxHealthAttribute()).RemoveAll(this);

	InputRecordComponent->OnOperationPreviewDelegate.RemoveAll(this);

	if (AMyPlayerController* PlayerController{ Cast<AMyPlayerController>(GetController()) })
	{
		PlayerController->OnReceiveMoveInputDelegate.RemoveAll(this);
		PlayerController->OnReceiveShootInputDelegate.RemoveAll(this);
	}
}

void APlayerCharacterBase::OnCharacterHealthChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	UpdateCharacterWidget();
}

void APlayerCharacterBase::OnInputRecordOperationPreview(const bool bIsPreview, const IRecordedDataObjectInterface* Data)
{
	OnOperationPreviewDelegate.Broadcast(bIsPreview, Data);

	RecordedDataVisitor::Visit([WeakThis = MakeWeakObjectPtr(this), bIsPreview](const FRecordedLocationData& LocationData) {
		if (!WeakThis.IsValid()) { return; }

		if (bIsPreview) { WeakThis->RecordedLocationVisualizationComponent->AddRecordedLocation(LocationData.Tick, LocationData.Payload.NewLocation); }
		else { WeakThis->RecordedLocationVisualizationComponent->RemoveRecordedLocation(LocationData.Tick); }

		}, Data);

	//RecordedDataVisitor::Visit(RecordedDataVisitor::TOverloaded{
	//	[](const FRecordedLocationData& Location) {UE_LOG(LogTemp, Error, TEXT("OnInputRecordOperationPreview FRecordedLocationPayloadBase")); },
	//	[](const FRecordedRotationData& Rotation) {UE_LOG(LogTemp, Error, TEXT("OnInputRecordOperationPreview FRecordedRotationPayloadBase")); },
	//	[](const FRecordedCombinableAbilityData& AbilityData) {UE_LOG(LogTemp, Error, TEXT("OnInputRecordOperationPreview FRecordedCombinableAbilityData")); }
	//	},
	//	Data);
}

void APlayerCharacterBase::CancelRewindingState()
{
	if (InputRecordComponent)
	{
		InputRecordComponent->CancelRewindingState();
	}
}

void APlayerCharacterBase::CreateAndSetupComponents()
{
	//SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(FName("SpringArmComponent"));
	//SpringArmComponent->SetupAttachment(RootComponent);
	////SpringArmComponent->SetAbsolute(false, true, false);
	//SpringArmComponent->SetUsingAbsoluteRotation(true);
	//SpringArmComponent->PrimaryComponentTick.bCanEverTick = false;
	//SpringArmComponent->SetWorldRotation(FRotator{ -90., 0., 0. });
	//SpringArmComponent->TargetArmLength = 1200.f;
	//SpringArmComponent->bUsePawnControlRotation = false;
	//SpringArmComponent->bInheritPitch = false;
	//SpringArmComponent->bInheritYaw = false;
	//SpringArmComponent->bInheritRoll = false;

	//CameraComponent = CreateDefaultSubobject<UCameraComponent>(FName("CameraComponent"));
	//CameraComponent->PrimaryComponentTick.bCanEverTick = false;
	//CameraComponent->ProjectionMode = ECameraProjectionMode::Orthographic;
	//CameraComponent->OrthoWidth = 1000.f;
	//CameraComponent->SetupAttachment(SpringArmComponent);

	FogOfWarComponent = CreateDefaultSubobject<UFogOfWarComponent>(FName("FogOfWarComponent"));
	FogOfWarComponent->PrimaryComponentTick.bCanEverTick = false;

	InputRecordComponent = CreateDefaultSubobject<UInputRecordComponent>(FName("InputRecordComponent"));
	InputRecordComponent->PrimaryComponentTick.bCanEverTick = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	RecordedLocationVisualizationComponent = CreateDefaultSubobject<URecordedLocationVisualizationComponent>(FName("RecordedLocationVisualizationComponent"));
	RecordedLocationVisualizationComponent->PrimaryComponentTick.bCanEverTick = false;
	//RecordedLocationVisualizationComponent->SetUpAttachment(RootComponent);
}
