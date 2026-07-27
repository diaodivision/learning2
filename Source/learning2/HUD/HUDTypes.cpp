#include "HUDTypes.h"
#include "Character/PlayerCharacterBase.h"
#include "Controller/MyPlayerController.h"
#include "Ability/AbilitySystemComponent/MyAbilitySystemComponent.h"

FPossessedCharacterWidgetControllerContext::FPossessedCharacterWidgetControllerContext(APlayerCharacterBase* Character)
{
	if (!Character) { return; }

	PlayerController = Cast<AMyPlayerController>(Character->GetController());
	if (!PlayerController.IsValid()) { return; }

	AttributeSet = Character->GetAttributeSet();
	PlayerState = Character->GetPlayerState();
	AbilitySystemComponent = Cast<UMyAbilitySystemComponent>(Character->GetAbilitySystemComponent());
	InputRecordComponent = Character->GetInputRecordComponent();
	MyCharacter = Character;
}

FPossessedCharacterWidgetControllerContext::FPossessedCharacterWidgetControllerContext(
	AMyPlayerController* PlayerController,
	UMyAttributeSet* AttributeSet,
	APlayerState* PlayerState,
	UMyAbilitySystemComponent* AbilitySystemComponent,
	UInputRecordComponent* InputRecordComponent,
	APlayerCharacterBase* MyCharacter
) : PlayerController(PlayerController),
AttributeSet(AttributeSet),
PlayerState(PlayerState),
AbilitySystemComponent(AbilitySystemComponent),
InputRecordComponent(InputRecordComponent),
MyCharacter(MyCharacter)
{
}
