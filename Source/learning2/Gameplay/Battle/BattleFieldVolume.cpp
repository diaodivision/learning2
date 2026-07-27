#include "BattleFieldVolume.h"
#include "BattleSubsystem.h"

ABattleFieldVolume::ABattleFieldVolume()
{
	if (UPrimitiveComponent * PrimitiveComponent{ Cast<UPrimitiveComponent>(GetRootComponent()) })
	{
		PrimitiveComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
		PrimitiveComponent->SetCollisionObjectType(ECC_WorldStatic);
		PrimitiveComponent->SetNotifyRigidBodyCollision(true);
		PrimitiveComponent->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	}
}

void ABattleFieldVolume::BeginPlay()
{
	if (UBattleSubsystem* BattleSubsystem = ULocalPlayer::GetSubsystem<UBattleSubsystem>(GetWorld()->GetFirstLocalPlayerFromController()))
	{
		BattleSubsystem->OnBattleFieldVolumeBeginPlay(this);
	}
}

void ABattleFieldVolume::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	if (UBattleSubsystem* BattleSubsystem = ULocalPlayer::GetSubsystem<UBattleSubsystem>(GetWorld()->GetFirstLocalPlayerFromController()))
	{
		BattleSubsystem->OnBattleFieldVolumeEndPlay(this, EndPlayReason);
	}
}