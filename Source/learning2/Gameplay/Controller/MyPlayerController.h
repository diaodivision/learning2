// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InteractiveTraceComponent.h"
#include "Delegates/DelegateCombinations.h"
#include "Switchable/SwitchableCollection.h"

#include "MyPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
struct FInputActionValue;
class AActor;
class APawn;
class UAbilitySystemComponent;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnHoveredActorChangedDelegate, AActor*, AActor*);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSelectedActorChangedDelegate, AActor*, AActor*);
DECLARE_MULTICAST_DELEGATE(FOnReceiveMoveInputDelegate);
DECLARE_MULTICAST_DELEGATE(FOnReceiveShootInputDelegate);

USTRUCT(BlueprintType)
struct FSwitchWeaponAction
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	TObjectPtr<UInputAction> SwitchToMainWeapon = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	TObjectPtr<UInputAction> SwitchToSecondaryWeapon = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	TObjectPtr<UInputAction> SwitchToGrenade1 = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	TObjectPtr<UInputAction> SwitchToGrenade2 = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	TObjectPtr<UInputAction> SwitchToGrenade3 = nullptr;
};

USTRUCT(BlueprintType)
struct FHitResultUnderCursorCached
{
	GENERATED_BODY()

	FHitResultUnderCursorCached() = default;

	bool NeedUpdate() const;

	void SetNeedUpdate();

	bool Update(const APlayerController& PlayerController);

	UPROPERTY(BlueprintReadOnly, Category = "Hit Result")
	FHitResult HitResult;

private:
	bool bNeedUpdate{ true };
};

/**
 *
 */
UCLASS(BlueprintType, Blueprintable)
class LEARNING2_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMyPlayerController();

	UFUNCTION(BlueprintCallable, Category = "Hit Result")
	bool GetHitResultUnderCursorAndCache(FHitResult& OutHitResult);

	UFUNCTION(BlueprintCallable, meta = (DefaultToSelf = WorldContextObject))
	static void PossessToPawn(APawn* NewPawn, const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable)
	void SetTargeting(const bool IsTargeting, UAbilitySystemComponent* AbilitySystemComponent = nullptr);

	const AActor* AutoPossessPlayerCharacter();

	virtual void DisableInput(class APlayerController* PlayerController) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void PreProcessInput(const float DeltaTime, const bool bGamePaused) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gameplay|Input")
	void Move(const FInputActionValue& Value);
	virtual void Move_Implementation(const FInputActionValue& Value);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gameplay|Input")
	void Aim();
	virtual void Aim_Implementation();

	//virtual void SwitchToAimMode(bool bIsSwitch);
	virtual void SetAimMode(bool bAim);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Character Weapons")
	USwitchableCollection* GetCharacterWeapons() const;
	USwitchableCollection* GetCharacterWeapons_Internal() const;
	TWeakObjectPtr<USwitchableCollection> CharacterWeapons{ nullptr };

	//UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gameplay|Input")
	//void SwitchWeapon(const FInputActionInstance& Instance);
	//virtual void SwitchWeapon_Implementation(const FInputActionInstance& Instance);

	void SwitchWeaponByIndex(int32 Index);

	virtual void SetupMouseAndInput();

	//UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Gameplay")
	//void OnCharacterRotationUpdated();

	virtual void OnLeftMousePressed();
	virtual void OnLeftMouseReleased();

	virtual void OnShoot();
	virtual void OnShootStop();
	virtual void OnReload();
	virtual void HandleLookAt();
	virtual void OnCancelTargeting();

	virtual void OnRewindToggle();
	virtual void OnCancelRewind();

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void SetMouseCursor(EMouseCursor::Type Cursor);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void BlockGameInput(bool bBlock);

	UFUNCTION(BlueprintImplementableEvent, Category = "Actor")
	void OnHoveredActorChanged(AActor* OldActor, AActor* NewActor) const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Actor")
	void OnSelectedActorChanged(AActor* OldActor, AActor* NewActor) const;

	//UFUNCTION()
	//virtual void HandlePossessedPawnChanged(APawn* InOldPawn, APawn* InNewPawn);

	virtual void OnPossess(APawn* NewPawn) override;
	virtual void OnUnPossess() override;

	UFUNCTION()
	void OnPossessedCharacterMovementUpdated(float DeltaSeconds, FVector OldLocation, FVector OldVelocity);
	virtual void HandlePossessedCharacterMovementUpdated(float DeltaSeconds, FVector OldLocation, FVector OldVelocity) const;

	virtual void InitialzeDelegates();
	virtual void DeinitialzeDelegates();

public:
	UPROPERTY(BlueprintReadOnly, Category = "Hit Result")
	FHitResultUnderCursorCached HitResultUnderCursorCached;

	FOnHoveredActorChangedDelegate OnHoveredActorChangedDelegate;
	FOnSelectedActorChangedDelegate OnSelectedActorChangedDelegate;

	FOnReceiveMoveInputDelegate OnReceiveMoveInputDelegate;
	FOnReceiveShootInputDelegate OnReceiveShootInputDelegate;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace")
	float InteractiveTraceTickRate{ .1f };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UInteractiveTraceComponent> InteractiveTraceComponent;

	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> HoveredActor;

	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> SelectedActor;

	UPROPERTY(BlueprintReadOnly, Category = "Gameplay|Input|State")
	bool bLeftMousePressed{ false };

	UPROPERTY(BlueprintReadOnly, Category = "Gameplay|Input|State")
	bool bAimMode{ true };

	//bool GetActorLastLocation(FVector& Location, const AActor& Actor);
	//void SetActorLastLocation(const FVector& Location, const AActor& Actor);
	//bool GetActorLastRotation(FRotator& Rotation, const AActor& Actor);
	//void SetActorLastRotation(const FRotator& Rotation, const AActor& Actor);
	//TMap<TWeakObjectPtr<const AActor>, const FVector> LastLocationMap;
	//TMap<TWeakObjectPtr<const AActor>, const FRotator> LastRotationMap;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Input")
	TObjectPtr<UInputMappingContext> InputMapping;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Input")
	TObjectPtr<UInputAction> LookAtAction;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Input")
	TObjectPtr<UInputAction> AimAction;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Input")
	TObjectPtr<UInputAction> LeftMouseDownAction;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Input")
	TObjectPtr<UInputAction> ShootAction;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Input")
	TObjectPtr<UInputAction> ReloadAction;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Input")
	FSwitchWeaponAction SwitchWeaponAction;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Input")
	TObjectPtr<UInputAction> CancelTargetingAction;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Input")
	TObjectPtr<UInputAction> RewindToggleAction;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Input")
	TObjectPtr<UInputAction> CancelRewindAction;

	bool bIsTargeting{ false };
	bool bIsBlockShootAction{ false };

	TWeakObjectPtr<UAbilitySystemComponent> TargetingAbilitySystemComponent;
};
