// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"
#include "Engine/EngineBaseTypes.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
//#include "GameFramework/Character.h"
#include "Character/Base/MyCharacterBase.h"
#include "Character/PlayerCharacterBase.h"
#include "InputRecordedDataTypes/RecordedDataDefines.h"
#include "Kismet\GameplayStatics.h"
#include "RewindSystemStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "RewindSubsystem.h"
#include "RewindSystemStatics.h"
#include "InputRecordedDataTypes/RecordedDataTypes.h"
#include "Interface/WeaponInterface.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "WorldPauseSubsystem.h"
#include "AbilitySystemComponent.h"
#include "Targeting/TargetingInstigatorTypes.h"
#include "Switchable/SwitchableActorCollection.h"

AMyPlayerController::AMyPlayerController()
{
	InteractiveTraceComponent = CreateDefaultSubobject<UInteractiveTraceComponent>(FName("InteractTraceComponent"));
}

bool AMyPlayerController::GetHitResultUnderCursorAndCache(FHitResult& OutHitResult)
{
	if (!HitResultUnderCursorCached.NeedUpdate() || HitResultUnderCursorCached.Update(*this))
	{
		OutHitResult = HitResultUnderCursorCached.HitResult;
		return true;
	}

	return false;
}

void AMyPlayerController::PossessToPawn(APawn* NewPawn, const UObject* WorldContextObject)
{
	UWorld* World{ GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) };
	if (!World) { return; }

	AAIController* AIController{ NewPawn ? Cast<AAIController>(NewPawn->GetController()) : nullptr };
	if (!AIController) { return; }

	APlayerController* PlayerController{ World->GetFirstPlayerController() };
	APawn* OldPawn{ PlayerController ? PlayerController->GetPawn() : nullptr };
	if (/*!PlayerController || */!OldPawn) { return; }

	PlayerController->UnPossess();
	AIController->UnPossess();

	PlayerController->Possess(NewPawn);	//必须先调用 PlayerController->Possess(NewPawn)
	AIController->Possess(OldPawn);		//再调用 AIController->Possess(OldPawn)

	if (AMyPlayerController * MP{ Cast<AMyPlayerController>(PlayerController) }) { MP->OnCancelTargeting(); }
}

void AMyPlayerController::SetTargeting(const bool IsTargeting, UAbilitySystemComponent* AbilitySystemComponent)
{
	if (IsTargeting)
	{
		if (!ensureAlways(AbilitySystemComponent)) { return; }
		TargetingAbilitySystemComponent = AbilitySystemComponent;
		if (APlayerCharacterBase * PlayerCharacter{ Cast<APlayerCharacterBase>(GetCharacter()) })
		{
			PlayerCharacter->SetTargetingState(ETargetingState::Targeting);
		}
	}
	else
	{
		TargetingAbilitySystemComponent = nullptr;
		bIsBlockShootAction = true;
	}

	bIsTargeting = IsTargeting;
}

const AActor* AMyPlayerController::AutoPossessPlayerCharacter()
{
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(this, APlayerCharacterBase::StaticClass(), Actors);

	for (AActor* Actor : Actors)
	{
		APlayerCharacterBase* PlayerCharacter{ Cast<APlayerCharacterBase>(Actor) };
		if (!PlayerCharacter || PlayerCharacter == GetPawn() || PlayerCharacter->IsDead()) { continue; }
	
		Possess(PlayerCharacter);
		return PlayerCharacter;
	}
	
	return nullptr;
}

void AMyPlayerController::DisableInput(class APlayerController* PlayerController)
{
	Super::DisableInput(PlayerController);

	OnShootStop();
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	check(InputMapping);

	UEnhancedInputLocalPlayerSubsystem* SubSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	check(InputMapping);
	SubSystem->AddMappingContext(InputMapping, 0);

	SetupMouseAndInput();

	InteractiveTraceComponent->SetComponentTickInterval(InteractiveTraceTickRate);

	InitialzeDelegates();
}

void AMyPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	DeinitialzeDelegates();
}

void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhanceInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	EnhanceInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMyPlayerController::Move);
	EnhanceInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &AMyPlayerController::SetAimMode, false);
	EnhanceInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AMyPlayerController::SetAimMode, true);
	//EnhanceInputComponent->BindAction(LookAtAction, ETriggerEvent::Triggered, this, &AMyPlayerController::HandleLookAt);

	EnhanceInputComponent->BindAction(SwitchWeaponAction.SwitchToMainWeapon.Get(), ETriggerEvent::Started, this, &AMyPlayerController::SwitchWeaponByIndex, 0);
	EnhanceInputComponent->BindAction(SwitchWeaponAction.SwitchToSecondaryWeapon.Get(), ETriggerEvent::Started, this, &AMyPlayerController::SwitchWeaponByIndex, 1);
	EnhanceInputComponent->BindAction(SwitchWeaponAction.SwitchToGrenade1.Get(), ETriggerEvent::Started, this, &AMyPlayerController::SwitchWeaponByIndex, 2);
	EnhanceInputComponent->BindAction(SwitchWeaponAction.SwitchToGrenade2.Get(), ETriggerEvent::Started, this, &AMyPlayerController::SwitchWeaponByIndex, 3);
	EnhanceInputComponent->BindAction(SwitchWeaponAction.SwitchToGrenade3.Get(), ETriggerEvent::Started, this, &AMyPlayerController::SwitchWeaponByIndex, 4);

	EnhanceInputComponent->BindAction(LeftMouseDownAction, ETriggerEvent::Started, this, &AMyPlayerController::OnLeftMousePressed);
	EnhanceInputComponent->BindAction(LeftMouseDownAction, ETriggerEvent::Completed, this, &AMyPlayerController::OnLeftMouseReleased);

	EnhanceInputComponent->BindAction(ShootAction, ETriggerEvent::Triggered, this, &AMyPlayerController::OnShoot);
	EnhanceInputComponent->BindAction(ShootAction, ETriggerEvent::Completed, this, &AMyPlayerController::OnShootStop);
	EnhanceInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &AMyPlayerController::OnReload);
	EnhanceInputComponent->BindAction(CancelTargetingAction, ETriggerEvent::Started, this, &AMyPlayerController::OnCancelTargeting);
	EnhanceInputComponent->BindAction(RewindToggleAction, ETriggerEvent::Started, this, &AMyPlayerController::OnRewindToggle);
	EnhanceInputComponent->BindAction(CancelRewindAction, ETriggerEvent::Started, this, &AMyPlayerController::OnCancelRewind);
	//for (auto& Action : SwitchWeaponAction)
	//{
	//	EnhanceInputComponent->BindAction(Action, ETriggerEvent::Started, this, &AMyPlayerController::SwitchWeapon);
	//}
}

void AMyPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (bAimMode) { HandleLookAt(); }
}

void AMyPlayerController::PreProcessInput(const float DeltaTime, const bool bGamePaused)
{
	HitResultUnderCursorCached.SetNeedUpdate();
	if (HitResultUnderCursorCached.Update(*this))
	{
		FHitResult& Hit = HitResultUnderCursorCached.HitResult;

		if (HoveredActor.Get() != Hit.GetActor())
		{
			OnHoveredActorChanged(HoveredActor.Get(), Hit.GetActor());
			OnHoveredActorChangedDelegate.Broadcast(HoveredActor.Get(), Hit.GetActor());
		}
		HoveredActor = Hit.GetActor();

		if (SelectedActor.Get() != Hit.GetActor())
		{
			OnSelectedActorChanged(SelectedActor.Get(), Hit.GetActor());
			OnSelectedActorChangedDelegate.Broadcast(SelectedActor.Get(), Hit.GetActor());
		}
		SelectedActor = Hit.GetActor();
	}
	//else
	//{
	//	if (HoveredActor.Get())
	//	{
	//		OnHoveredActorChanged(HoveredActor.Get(), nullptr);
	//		OnHoveredActorChangedDelegate.Broadcast(HoveredActor.Get(), nullptr);
	//	}
	//	HoveredActor.Reset();

	//	if (SelectedActor.Get())
	//	{
	//		OnSelectedActorChanged(SelectedActor.Get(), nullptr);
	//		OnSelectedActorChangedDelegate.Broadcast(SelectedActor.Get(), nullptr);
	//	}
	//	SelectedActor.Reset();
	//}
}

void AMyPlayerController::Move_Implementation(const FInputActionValue& Value)
{
	/*FVector2D InputVector = Value.Get<FVector2D>();
	FRotator YawRotation{ 0.f, GetControlRotation().Yaw, 0.f };

	FVector ForwardVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	FVector RightVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (APawn* ControlledPawn = GetPawn<APawn>()) {
		ControlledPawn->AddMovementInput(ForwardVector, InputVector.Y);
		ControlledPawn->AddMovementInput(RightVector, InputVector.X);
	}*/

	FVector2D InputVector = Value.Get<FVector2D>();

	if (APawn* ControlledPawn = GetPawn())
	{
		ControlledPawn->AddMovementInput(FVector::ForwardVector, InputVector.Y);
		ControlledPawn->AddMovementInput(FVector::RightVector, InputVector.X);

		OnReceiveMoveInputDelegate.Broadcast();
	}
}

void AMyPlayerController::Aim_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Aim"));

	if (!InputEnabled()) { return; }

	if (bIsTargeting) { return; }

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) { return; }

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (LocalPlayer && LocalPlayer->ViewportClient)
	{
		FVector2D MousePosition;
		if (LocalPlayer->ViewportClient->GetMousePosition(MousePosition))
		{
			FVector WorldOrigin;
			FVector WorldDirection;
			UGameplayStatics::DeprojectScreenToWorld(this, MousePosition, WorldOrigin, WorldDirection);

			const FVector ActorLocation = ControlledPawn->GetActorLocation();

			//地面的法向量
			const FVector NormalOfGround = FVector::UpVector;

			//从摄像机出发，沿鼠标方向到 Actor 活动的平面的距离
			const float DistanceFromCursorToGround = FMath::Abs(FVector::DotProduct(NormalOfGround, WorldOrigin - ActorLocation) / FVector::DotProduct(NormalOfGround, WorldDirection));

			const FVector TargetLocation = WorldOrigin + WorldDirection * DistanceFromCursorToGround;

			/*DrawDebugLine(GetWorld(), WorldOrigin, TargetLocation, FColor::Red, true, 1.f);*/

			// 3. 计算方向（保持2D平面）
			FVector PawnLocation = ControlledPawn->GetActorLocation();
			FVector Direction = TargetLocation - PawnLocation;
			Direction.Z = 0;  // 保持2D

			// 4. 设置旋转
			//if (!Direction.IsNearlyZero())
			if (!ControlledPawn->GetActorRotation().EqualsOrientation(Direction.Rotation()))
			{
				if (URewindSystemStatics::GetRewindSubsystemState(this) == ERecordState::Recording)
				{
					if (URewindSubsystem* System = URewindSystemStatics::GetRewindSubsystem(this))
					{
						TUniquePtr<FRotationData> Data = MakeUnique<FRotationData>();
						FRecordedRotationPayloadBase& Payload = Data->Payload;

						Payload.Actor = ControlledPawn;
						Payload.OldRotation = ControlledPawn->GetActorRotation();
						Payload.NewRotation = Direction.Rotation();

						const FRecordedDataObjectHandle Handle{ System->Record(MoveTemp(Data), *ControlledPawn) };
						{
							bool bSuccess{ false };
							URecordedDataPreviewOperationDelegateWrapper* Wrapper{ URewindSystemStatics::GetPreviewOperationDelegateWrapper(bSuccess, Handle) };
							if (bSuccess && Wrapper)
							{
								Wrapper->PreviewOperationDelegate.AddLambda(
									[](const FRecordedDataObjectHandle& Handle, const IRecordedDataObjectInterface* InRecordedData, bool bWillExecute) {
										const FRecordedRotationData* RecordedData{ CustomCast<FRecordedRotationData>(InRecordedData) };

										if (!RecordedData || !RecordedData->IsPayloadValid()) { return; }

										if (const FRecordedRotationPayloadBase& Payload = RecordedData->Payload; bWillExecute)
										{
											Payload.Actor->SetActorRotation(Payload.NewRotation);
										}
										else { Payload.Actor->SetActorRotation(Payload.OldRotation); }
									});
							}
						}
					}
				}

				ControlledPawn->SetActorRotation(Direction.Rotation());
				//OnCharacterRotationUpdated();
			}
		}
	}

	//// 1. 获取鼠标下的物体（使用碰撞检测）
	//if (!HitResultUnderCursorCached.NeedUpdate() || HitResultUnderCursorCached.Update(*this))
	//{
	//	FHitResult& Hit = HitResultUnderCursorCached.HitResult;

	//	//GetHitResultUnderCursor(ECC_Visibility, false, Hit);

	//	FVector TargetLocation;

	//	// 2
	//	TargetLocation = Hit.Location;

	//	// 3. 计算方向（保持2D平面）
	//	FVector PawnLocation = ControlledPawn->GetActorLocation();
	//	FVector Direction = TargetLocation - PawnLocation;
	//	Direction.Z = 0;  // 保持2D

	//	// 4. 设置旋转
	//	if (!Direction.IsNearlyZero())
	//	{
	//		ControlledPawn->SetActorRotation(Direction.Rotation());
	//		//SetControlRotation(Direction.Rotation());
	//		OnCharacterRotationUpdated();
	//	}
	//}
}

void AMyPlayerController::SetAimMode(bool bAim)
{
	bAimMode = bAim;
}

USwitchableCollection* AMyPlayerController::GetCharacterWeapons()
{
	USwitchableCollection* Weapons = CharacterWeapons.Get();
	if (!Weapons) 
	{
		if (AMyCharacterBase* Ch{ Cast<AMyCharacterBase>(GetCharacter()) })
		{
			CharacterWeapons = Ch->GetWeaponContainer();
		}
	}

	return Weapons;
}

//void AMyPlayerController::SwitchWeapon_Implementation(const FInputActionInstance& Instance)
//{
//	TObjectPtr<const UInputAction> InputAction = Instance.GetSourceAction();
//
//	int32 Index = SwitchWeaponAction.IndexOfByKey(InputAction);
//	if (Index != INDEX_NONE) { Weapons->SwitchActorByIndex(Index); }
//}

void AMyPlayerController::SwitchWeaponByIndex(int32 Index)
{
	USwitchableCollection* Weapons = GetCharacterWeapons();
	if (!Weapons) { return; }

	const UObject* LastControlledWeapon{ Weapons->GetControlledObject() };
	const UObject* ControlledWeapon{ Weapons->SwitchObjectByIndex(Index) };

	if (LastControlledWeapon != ControlledWeapon) { OnCancelTargeting(); }
}

//void AMyPlayerController::OnRightMousePressed()
//{
//	bAimMode = true;
//}
//
//void AMyPlayerController::OnRightMouseReleased()
//{
//	UE_LOG(LogTemp, Warning, TEXT("Released"));
//
//	bAimMode = false;
//}

void AMyPlayerController::OnLeftMousePressed()
{
	bLeftMousePressed = true;
}

void AMyPlayerController::OnLeftMouseReleased()
{
	bLeftMousePressed = false;

	if (SelectedActor.Get())
	{
		OnSelectedActorChanged(SelectedActor.Get(), nullptr);
		OnSelectedActorChangedDelegate.Broadcast(SelectedActor.Get(), nullptr);
	}
	SelectedActor.Reset();
}

void AMyPlayerController::OnShoot()
{
	if (bIsBlockShootAction) { return; }

	if (bIsTargeting)
	{
		//if (AMyCharacterBase* ControlledCharacter = Cast<AMyCharacterBase>(GetCharacter()))
		//{
		//	ControlledCharacter->OnTargetConfirm(ETargetConfirmType::Confirm);
		//}

		if (TargetingAbilitySystemComponent.IsValid()) { TargetingAbilitySystemComponent->TargetConfirm(); }
		if (APlayerCharacterBase * PlayerCharacter{ Cast<APlayerCharacterBase>(GetCharacter()) })
		{
			PlayerCharacter->SetTargetingState(ETargetingState::Confirm);
			OnReceiveShootInputDelegate.Broadcast();
		}

		SetTargeting(false);
	}
	else
	{
		USwitchableCollection* Weapons = GetCharacterWeapons();
		if (!Weapons) { return; }

		UE_LOG(LogTemp, Warning, TEXT("Shooting Weapons->GetControlledObject(): %s"), *GetNameSafe(Weapons->GetControlledObject()));

		if (IWeaponInterface* Weapon = Cast<IWeaponInterface>(Weapons->GetControlledObject()))
		{
			const bool bSucceed{ Weapon->Fire() };
			UE_LOG(LogTemp, Warning, TEXT("Weapon->Fire(): %d"), bSucceed);

			if (bSucceed)
			{
				if (AMyCharacterBase* ControlledCharacter = Cast<AMyCharacterBase>(GetCharacter()))
				{
					ControlledCharacter->OnShoot();
					OnReceiveShootInputDelegate.Broadcast();
				}
			}
		}
	}
}

void AMyPlayerController::OnShootStop()
{
	bIsBlockShootAction = false;

	if (AMyCharacterBase* ControlledCharacter = Cast<AMyCharacterBase>(GetCharacter()))
	{
		ControlledCharacter->OnShootStop();
	}
}

void AMyPlayerController::OnReload()
{
	USwitchableCollection* Weapons = GetCharacterWeapons();
	if (!Weapons) { return; }

	if (IWeaponInterface* Weapon = Cast<IWeaponInterface>(Weapons->GetControlledObject()))
	{
		Weapon->Reload();
	}
}

void AMyPlayerController::HandleLookAt()
{
	if (bAimMode) { Aim(); }
}

void AMyPlayerController::OnCancelTargeting()
{
	if (bIsTargeting)
	{
		SetTargeting(false);

		if (TargetingAbilitySystemComponent.IsValid()) { TargetingAbilitySystemComponent->TargetCancel(); }
		if (APlayerCharacterBase * PlayerCharacter{ Cast<APlayerCharacterBase>(GetCharacter()) })
		{
			PlayerCharacter->SetTargetingState(ETargetingState::Cancel);
		}
	}
}

void AMyPlayerController::OnRewindToggle()
{
	if (URewindSubsystem* RewindSubsystem{ URewindSystemStatics::GetRewindSubsystem(this) }; RewindSubsystem && RewindSubsystem->GetCurrentState() == ERecordState::Idle)
	{
		RewindSubsystem->SwitchState(ERecordState::Recording);
	}
}

void AMyPlayerController::OnCancelRewind()
{
	if (URewindSubsystem* RewindSubsystem{ URewindSystemStatics::GetRewindSubsystem(this) }; RewindSubsystem && RewindSubsystem->GetCurrentState() != ERecordState::Idle)
	{
		RewindSubsystem->SwitchState(ERecordState::Idle);
	}
}

void AMyPlayerController::SetupMouseAndInput()
{
	// 1. 使用纯游戏模式（此时视口拥有最高优先级的连续捕获，鼠标一动就触发 Axis）
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockInFullscreen);
	// 开启消费捕获点击，防止点击时出现奇怪的视口焦点丢失
	// InputMode.SetConsumeCaptureMouseDown(false);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

	// 2. 强行在 GameOnly 模式下把鼠标画出来
	bShowMouseCursor = true;
	SetMouseCursor(EMouseCursor::Default);
	//// 显示鼠标光标
	//bShowMouseCursor = true;
	//SetMouseCursor(EMouseCursor::Default);

	//// 设置输入模式
	//FInputModeGameAndUI InputMode;
	//InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	//InputMode.SetHideCursorDuringCapture(false);

	//SetInputMode(InputMode);
}

void AMyPlayerController::SetMouseCursor(EMouseCursor::Type Cursor)
{
	DefaultMouseCursor = Cursor;

	// 强制更新光标显示
	if (bShowMouseCursor)
	{
		// 这里可以添加自定义光标更新逻辑
		// 例如：根据当前状态改变光标类型

		// 重新设置当前光标
		CurrentMouseCursor = DefaultMouseCursor;
	}
}

void AMyPlayerController::BlockGameInput(bool bBlock)
{
	//if (bBlock)
	//{
	//	FInputModeUIOnly InputMode;

	//	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockInFullscreen);
	//	SetInputMode(InputMode);
	//}
	//else
	//{
	//	FInputModeGameAndUI InputMode;
	//	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockInFullscreen);
	//	InputMode.SetHideCursorDuringCapture(false);
	//	SetInputMode(InputMode);
	//}
}

//void AMyPlayerController::HandlePossessedPawnChanged(APawn* InOldPawn, APawn* InNewPawn)
//{
//	if (InOldPawn == InNewPawn) { return; }
//
//	if (ACharacter* OldCharacter = Cast<ACharacter>(InOldPawn))
//	{
//		OldCharacter->OnCharacterMovementUpdated.RemoveAll(this);
//	}
//
//	if (ACharacter* NewCharacter = Cast<ACharacter>(InNewPawn))
//	{
//		NewCharacter->OnCharacterMovementUpdated.AddDynamic(this, &AMyPlayerController::OnPossessedCharacterMovementUpdated);
//	}
//
//	CharacterWeapons = GetCharacterWeapons();
//}

void AMyPlayerController::OnPossess(APawn* NewPawn)
{
	if (ACharacter* OldCharacter = Cast<ACharacter>(GetPawn()))
	{
		OldCharacter->OnCharacterMovementUpdated.RemoveAll(this);
	}

	if (ACharacter* NewCharacter = Cast<ACharacter>(NewPawn))
	{
		NewCharacter->OnCharacterMovementUpdated.AddUniqueDynamic(this, &AMyPlayerController::OnPossessedCharacterMovementUpdated);
	}

	// CharacterWeapons = GetCharacterWeapons();

	Super::OnPossess(NewPawn);
}

void AMyPlayerController::OnUnPossess()
{
	if (ACharacter* OldCharacter = Cast<ACharacter>(GetPawn()))
	{
		OldCharacter->OnCharacterMovementUpdated.RemoveAll(this);
	}
	OnShootStop();
	OnCancelTargeting();
	CharacterWeapons = nullptr;

	Super::OnUnPossess();
}

void AMyPlayerController::OnPossessedCharacterMovementUpdated(float DeltaSeconds, FVector OldLocation, FVector OldVelocity)
{
	HandlePossessedCharacterMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
}

void AMyPlayerController::HandlePossessedCharacterMovementUpdated(float DeltaSeconds, FVector OldLocation, FVector OldVelocity) const
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) { return; }

	const FVector NewLocation = ControlledPawn->GetActorLocation();
	if (OldLocation.Equals(NewLocation)) { return; }

	URewindSubsystem* System = URewindSystemStatics::GetRewindSubsystem(this);
	if (!System || System->GetCurrentState() != ERecordState::Recording) { return; }

	TUniquePtr<FLocationData> Data = MakeUnique<FLocationData>();
	FRecordedLocationPayloadBase& Payload = Data->Payload;
	Payload.Actor = ControlledPawn;
	Payload.OldLocation = OldLocation;
	Payload.NewLocation = ControlledPawn->GetActorLocation();

	FRecordedDataObjectHandle Handle{ System->Record(MoveTemp(Data), *ControlledPawn) };

	{
		bool bSuccess{ false };
		URecordedDataPreviewOperationDelegateWrapper* Wrapper{ URewindSystemStatics::GetPreviewOperationDelegateWrapper(bSuccess, Handle) };
		if (bSuccess && Wrapper)
		{
			Wrapper->PreviewOperationDelegate.AddLambda(
				[](const FRecordedDataObjectHandle& Handle, const IRecordedDataObjectInterface* InRecordedData, bool bWillExecute) {
					const FRecordedLocationData* RecordedData{ CustomCast<FRecordedLocationData>(InRecordedData) };

					if (!RecordedData || !RecordedData->IsPayloadValid()) { return; }

					if (const FRecordedLocationPayloadBase& Payload = RecordedData->Payload; bWillExecute) { Payload.Actor->SetActorLocation(Payload.NewLocation); }
					else { Payload.Actor->SetActorLocation(Payload.OldLocation); }
				});
		}
	}
}

void AMyPlayerController::InitialzeDelegates()
{
	//OnPossessedPawnChanged.AddDynamic(this, &AMyPlayerController::HandlePossessedPawnChanged);

	if (ACharacter* ControlledCharacter = GetCharacter(); ControlledCharacter)
	{
		if (!ControlledCharacter->OnCharacterMovementUpdated.IsAlreadyBound(this, &AMyPlayerController::OnPossessedCharacterMovementUpdated))
		{
			ControlledCharacter->OnCharacterMovementUpdated.AddDynamic(this, &AMyPlayerController::OnPossessedCharacterMovementUpdated);
		}
	}

	//OnCharacterMovementUpdated
}

void AMyPlayerController::DeinitialzeDelegates()
{
	//OnPossessedPawnChanged.RemoveAll(this);

	//OnCharacterMovementUpdated
}

bool FHitResultUnderCursorCached::NeedUpdate() const
{
	return bNeedUpdate;
}

void FHitResultUnderCursorCached::SetNeedUpdate()
{
	bNeedUpdate = true;
}

bool FHitResultUnderCursorCached::Update(const APlayerController& PlayerController)
{
	if (PlayerController.GetHitResultUnderCursor(ECC_Visibility, true, HitResult))
	{
		bNeedUpdate = false;
		return true;
	}

	return false;
}