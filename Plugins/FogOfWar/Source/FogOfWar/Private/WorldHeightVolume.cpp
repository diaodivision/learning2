#include "WorldHeightVolume.h"
#include "Components/BrushComponent.h"
#include "WorldHeightSubsystem.h"

//AWorldHeightVolume::AWorldHeightVolume(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
AWorldHeightVolume::AWorldHeightVolume()
{
	UBrushComponent* BrushComp = GetBrushComponent();
	BrushComp->SetCollisionProfileName(UCollisionProfile::CustomCollisionProfileName);;
	BrushComp->Mobility = EComponentMobility::Static;
	BrushComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	BrushComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BrushComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	BrushComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	BrushComp->SetGenerateOverlapEvents(true);
}

//void AWorldHeightVolume::BeginPlay()
//{
//	if (UWorldHeightSubsystem* WorldHeightSubsystem = GetWorld()->GetSubsystem<UWorldHeightSubsystem>())
//	{
//		WorldHeightSubsystem->RequestUpdateWorldHeightData(*this);
//	}
//}

void AWorldHeightVolume::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();


	if (UWorldHeightSubsystem* WorldHeightSubsystem = GetWorld()->GetSubsystem<UWorldHeightSubsystem>())
	{
		WorldHeightSubsystem->OnWorlHeightVolumeRegisteredComponents(*this);
		//WorldHeightSubsystem->RequestUpdateWorldHeightData(*this);
	}
}

void AWorldHeightVolume::PostUnregisterAllComponents()
{
	Super::PostUnregisterAllComponents();

	if (UWorldHeightSubsystem* WorldHeightSubsystem = GetWorld()->GetSubsystem<UWorldHeightSubsystem>())
	{
		WorldHeightSubsystem->OnWorlHeightVolumeUnregisteredComponents(*this);
	}
}

void AWorldHeightVolume::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
}

void AWorldHeightVolume::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);

	if (OtherActor)
	{
		if (UWorldHeightSubsystem* HeightSubsystem = GetWorld()->GetSubsystem<UWorldHeightSubsystem>())
		{
			//HeightSubsystem->RequestUpdateWorldHeightData(*OtherActor);
		}
	}
}

#if WITH_EDITOR
void AWorldHeightVolume::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (UWorldHeightSubsystem* HeightSubsystem = GetWorld()->GetSubsystem<UWorldHeightSubsystem>(); GIsEditor && HeightSubsystem)
	{
		const FName PropName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : FName();
		const FName MemberName = (PropertyChangedEvent.MemberProperty != nullptr) ? PropertyChangedEvent.MemberProperty->GetFName() : FName();

		if (PropName == GET_MEMBER_NAME_CHECKED(ABrush, BrushBuilder)
			|| MemberName == USceneComponent::GetRelativeLocationPropertyName()
			|| MemberName == USceneComponent::GetRelativeRotationPropertyName()
			|| MemberName == USceneComponent::GetRelativeScale3DPropertyName())
		{
			HeightSubsystem->RequestUpdateWorldHeightData(*this);
		}
	}
}

void AWorldHeightVolume::PostEditUndo()
{
	Super::PostEditUndo();

	if (UWorldHeightSubsystem* HeightSubsystem = GetWorld()->GetSubsystem<UWorldHeightSubsystem>(); GIsEditor && HeightSubsystem)
	{
		HeightSubsystem->RequestUpdateWorldHeightData(*this);
	}
}

#endif  // WITH_EDITOR
