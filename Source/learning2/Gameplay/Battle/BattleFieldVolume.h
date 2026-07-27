#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Volume.h"
#include "UObject/ObjectMacros.h"
#include "BattleFieldVolume.generated.h"

UENUM(BlueprintType)
enum class EBattleFieldBoundType :uint8
{
	Fixed,
	Specified
};

USTRUCT(BlueprintType)
struct FBattleFieldBound
{
	GENERATED_BODY()

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//FVector2D LeftDownPosition{ FVector2D::ZeroVector };

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//FVector2D RightTopPosition{ FVector2D::ZeroVector };
	FBox Box;
};

UCLASS(MinimalAPI)
class ABattleFieldVolume : public AVolume
{
	GENERATED_BODY()

public:
	ABattleFieldVolume();

	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	EBattleFieldBoundType BattleFieldBoundType{ EBattleFieldBoundType::Fixed };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true, EditCondition = "BattleFieldBoundType == EBattleFieldBoundType::Specified", EditConditionHides))
	TArray<FBattleFieldBound> BattleFieldBounds;

public:
	bool operator==(const ABattleFieldVolume& Other) const
	{
		return this->GetUniqueID() == Other.GetUniqueID();
	}

	friend uint32 GetTypeHash(const ABattleFieldVolume& BattleFieldVolume)
	{
		return GetTypeHash(BattleFieldVolume.GetUniqueID());
	}
};