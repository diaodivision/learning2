// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Math/UnrealMathUtility.h"
#include "Subsystems/WorldSubsystem.h"
#include "FogOfWarTypes.h"
#include "FogOfWarComputeShader.h"
#include "Components/ActorComponent.h"
#include "Delegates/DelegateCombinations.h"
#include "FogOfWarSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFogOfWarTextureUpdatedDelegate, UTexture2D *, Texture);
DECLARE_MULTICAST_DELEGATE(FOnViewportSizeChangedDelegate);

class APlayerCameraManager;
class UFogOfWarComponent;
class USceneComponent;
class UMaterialInstanceDynamic;
class FViewport;

/**
 *
 */
UCLASS()
class FOGOFWAR_API UFogOfWarSubsystem : public UWorldSubsystem, public FTickableGameObject
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    virtual FORCEINLINE bool IsTickable() const override { return !IsTemplate(); } // 不是CDO才Tick
    virtual FORCEINLINE TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UFogOfWarSubsystem, STATGROUP_Tickables); }

protected:
    virtual void Initialize(FSubsystemCollectionBase &Collection) override;
    virtual void Deinitialize() override;

    virtual void Tick(float DeltaTime) override;
    virtual void Tick_Internal();

public:
    UFUNCTION(BlueprintCallable, Category = "FogOfWar")
    void OnPostComponentInitialize(UFogOfWarComponent *Component);

    UFUNCTION(BlueprintPure, Category = "Resolution")
    FORCEINLINE TOptional<FIntPoint> GetScreenSize() const
    {
        if (!bIsInitialScale)
        {
            return NullOpt;
        }
        // return FIntPoint{ FMath::FloorToInt32(kScreenBaseWidth * WidthScaleFactor),
        // FMath::FloorToInt32(kScreenBaseHeight * HeightScaleFactor) };
        // return FIntPoint{874, 256};
        return FIntPoint{FMath::FloorToInt32(kScreenBaseWidth * WidthScaleFactor), FMath::FloorToInt32(kScreenBaseHeight * HeightScaleFactor)}; 
    }

    TOptional<FGridSizeType> GetGridSize() const;

    bool IsCameraFOVChanged();

private:
    bool CreateDynamicTexture();
    void SetTextureParameter() const;

    void GetFogOfWarActorData(TArray<FIntPoint> &ActorPositions, TArray<FVector2f> &ActorVision, TArray<int32> &RadiusSqList) const;

    void UploadFogOfWarActorData(
        const TArray<FIntPoint> &ActorPositions, 
        const TArray<FVector2f> &ActorVision,
        const TArray<int32> &RadiusSqList, 
        FFogOfWarComputeShader::FParameters &Parameter,
        FRDGBuilder &GraphBuilder) const;

    void SetComputeShaderOutputTextureCache(FRDGTextureRef &ShaderOutputTexture,FFogOfWarComputeShader::FParameters &Parameter, FRDGBuilder &GraphBuilder, const bool bCreateNewOne);

    TOptional<FIntPoint> ProjectWorldToLand(const FVector2D &WorldLocation, const FBox2D &LandBoundingBox) const;
    // bool ProjectWorldToLand(FIntPoint& Position, const FVector2D& WorldLocation, const FBox2D& LandBoundingBox)
    // const;

    void SetupScaleFactor();

    void OnViewportResized(FViewport *Viewport, uint32 Unused);

    void SetUpPlayerCameraManager();

    void OnFogOfWarComponentOwnerOrCameraTransformUpdated(USceneComponent *SceneComponent,
                                                          EUpdateTransformFlags UpdateTransformFlags,
                                                          ETeleportType Teleport);

public:
    FOnViewportSizeChangedDelegate OnViewportSizeChangedDelegate;

    UPROPERTY(BlueprintAssignable, Category = "FogOfWar")
    FOnFogOfWarTextureUpdatedDelegate OnFogOfWarTextureUpdatedDelegate;

private:
    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> FogOfWarMaterial;

    UPROPERTY()
    TObjectPtr<UTexture2D> DynamicTexture;

    UPROPERTY()
    TArray<uint8> WorldHeightData;
    uint32 CachedWorldHeightDataVersion{0};

    // 缓存 GPU 纹理（用于 RDG 提取）
    TRefCountPtr<IPooledRenderTarget> CachedOutputTexture;

    UPROPERTY()
    TArray<TWeakObjectPtr<UFogOfWarComponent>> FogOfWarComponents;
    mutable bool bHasInvalidComponents{false};

    bool bIsInitialScale{false};
    constexpr static int16 kScreenBaseWidth{ 256 };
    constexpr static int16 kScreenBaseHeight{ 256 };
    float WidthScaleFactor{1};
    float HeightScaleFactor{1};
    bool bViewportResized{false};

    TWeakObjectPtr<APlayerCameraManager> PlayerCameraManager;
    float LastFOVAngle{0.f};

    TMap<TWeakObjectPtr<USceneComponent>, FTransform> LastComponentOwnerOrCameraTransformMap;
};