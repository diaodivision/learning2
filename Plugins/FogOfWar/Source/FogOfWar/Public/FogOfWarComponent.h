// FogOfWarComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FogOfWarTypes.h"
#include "FogOfWarComponent.generated.h"

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVisibilityTextureUpdated, UTexture2D*, Texture);

UCLASS(meta = (BlueprintSpawnableComponent))
class FOGOFWAR_API UFogOfWarComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFogOfWarComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Initialize")
	void InitializeComputeShader(int32 InTextureWidth, int32 InTextureHeight);

	bool GetFogOfWarData(FFogOfWarData& Data) const;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true, UIMin = "0", UIMax = "180", ClampMin = "0", ClampMax = "180"))
	float VisionDegree{ 30.f };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true, UIMin = "0", ClampMin = "0"))
	float VisionRadius{ 0.f };

	int16 TextureWidth{ 512 };
	int16 TextureHeight{ 512 };
};