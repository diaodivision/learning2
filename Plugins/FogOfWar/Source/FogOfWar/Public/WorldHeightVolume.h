#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Volume.h"
#include "UObject/ObjectMacros.h"
#include "WorldHeightVolume.generated.h"

UCLASS(MinimalAPI)
class AWorldHeightVolume : public AVolume
{
	GENERATED_BODY()

public:
	AWorldHeightVolume();

#if WITH_EDITOR
	//~ Begin UObject Interface
	FOGOFWAR_API virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	FOGOFWAR_API virtual void PostEditUndo() override;
	//~ End UObject Interface
#endif // WITH_EDITOR

	bool operator==(const AWorldHeightVolume& Other) const
	{
		return this->GetUniqueID() == Other.GetUniqueID();
	}

	friend uint32 GetTypeHash(const AWorldHeightVolume& WorldHeightVolume)
	{
		return GetTypeHash(WorldHeightVolume.GetUniqueID());
	}

protected:
	//virtual void BeginPlay() override;


	FOGOFWAR_API virtual void PostRegisterAllComponents() override;
	FOGOFWAR_API virtual void PostUnregisterAllComponents() override;


	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;
};