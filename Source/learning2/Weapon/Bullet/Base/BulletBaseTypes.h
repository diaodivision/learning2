#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "BulletBaseTypes.generated.h"

class UGameplayEffect;

UCLASS(Blueprintable, BlueprintType)
class UBulletBaseInitData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage{ 0.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Penetration{ 0.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed{ 3000.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Direction{ FVector::ForwardVector };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableGravity{ false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayEffect> EffectClass;
};

UCLASS(Blueprintable, BlueprintType)
class UGrenadeBulletBaseInitData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage{ 0.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Penetration{ 0.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Direction{ FVector::ForwardVector };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedRate{ 100.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LifeTime{ false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayEffect> EffectClass;
};

USTRUCT(BlueprintType)
struct FGrenadeBulletAttributeData : public FTableRowBase
{
	GENERATED_BODY()

public:

	FGrenadeBulletAttributeData() = default;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon Attribute")
	float Damage{ 0.f };

	UPROPERTY(BlueprintReadOnly, Category = "Weapon Attribute")
	float Penetration{ 0.f };

	UPROPERTY(BlueprintReadOnly, Category = "Weapon Attribute")
	FVector Direction{ FVector::ForwardVector };

	UPROPERTY(BlueprintReadOnly, Category = "Weapon Attribute")
	float SpeedRate{ 1.f };

	UPROPERTY(BlueprintReadOnly, Category = "Weapon Attribute")
	float LifeTime{ 0.f };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Attribute")
	float EffectiveRange{ 0.f };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Attribute")
	UCurveFloat* GrenadeBulletSpeedFloatCurve{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Attribute")
	float EffectDuration{ 0.f };
};