// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "HUDTypes.h"
#include "MyHUD.generated.h"

class UMyUserWidget;
class UWidgetController;
class UMyAbilitySystemComponent;
class UInputRecordComponent;
class AMyPlayerController;
class UMyAttributeSet;
class APlayerCharacterBase;
class APlayerState;

/**
 *
 */
UCLASS()
class LEARNING2_API AMyHUD : public AHUD
{
	GENERATED_BODY()

public:
	//void SetUpHUD(
	//	AMyPlayerController* InPlayerController,
	//	UMyAttributeSet* InMyAttributeSet,
	//	APlayerState* InPlayerState,
	//	UMyAbilitySystemComponent* InMyAbilitySystemComponent,
	//	UInputRecordComponent* InInputRecordComponent,
	//	APlayerCharacterBase* InMyCharacterBase
	//);
	void SetUpHUD(FPossessedCharacterWidgetControllerContext Context);

	inline UWidgetController* GetWidgetController() const { return WidgetController; };

protected:
	virtual void BeginDestroy() override;

	UFUNCTION()
	virtual void OnPossessedPawnChanged(APawn* InOldPawn, APawn* InNewPawn);

public:
	UPROPERTY()
	TObjectPtr<UMyUserWidget> Widget;

private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMyUserWidget> WidgetClass;

	UPROPERTY()
	TObjectPtr<UWidgetController> WidgetController;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UWidgetController> WidgetControllerClass;

	FPossessedCharacterWidgetControllerContext Context;
};
