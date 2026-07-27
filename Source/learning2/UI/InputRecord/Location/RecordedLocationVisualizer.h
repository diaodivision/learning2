// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RecordedLocationVisualizationTypes.h"
#include "RecordedLocationVisualizer.generated.h"

class USplineComponent;
class USplineMeshComponent;
class URecordedLocationVisualizationComponent;
class UStaticMesh;
struct FSoftObjectPath;

UCLASS()
class LEARNING2_API ARecordedLocationVisualizer : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARecordedLocationVisualizer();

	virtual void PushRecordedLocation(const FVector& Location, const bool bNeedPlaceItem, const URecordedLocationVisualizationComponent& OwnerComponent);
	virtual void PopRecordedLocation(const URecordedLocationVisualizationComponent& OwnerComponent);
	virtual void PopRecordedLocationAtFront(const URecordedLocationVisualizationComponent& OwnerComponent);

	UFUNCTION(BlueprintCallable)
	virtual void PushRecordedLocation(const FVector& Location, const bool bNeedPlaceItem, const URecordedLocationVisualizationComponent* OwnerComponent)
	{
		if (OwnerComponent) { PushRecordedLocation(Location, bNeedPlaceItem, *OwnerComponent); }
	}
	UFUNCTION(BlueprintCallable)
	virtual void PopRecordedLocation(const URecordedLocationVisualizationComponent* OwnerComponent)
	{
		if (OwnerComponent) { PopRecordedLocation(*OwnerComponent); }
	}

	virtual void OnOwnerComponentDestroyed(const URecordedLocationVisualizationComponent& OwnerComponent);

protected:
	virtual void OnLineMeshLoaded(const int32 Index, const FVector& Location, USplineComponent& SplineComponent, UStaticMesh* Mesh);
	virtual void OnLocationItemMeshLoaded(const int32 Index, const FVector& Location, USplineComponent& SplineComponent, UStaticMesh* Mesh);

	USplineComponent* CreateAndSetUpSplineComponent(const URecordedLocationVisualizationComponent& OwnerComponent);

	USplineMeshComponent* CreateAndSetUpSplineMeshComponent(const URecordedLocationVisualizationComponent& OwnerComponent);
	USplineMeshComponent* CreateAndSetUpSplineMeshComponent(USplineComponent& SplineComponent);

	UStaticMeshComponent* CreateAndSetUpStaticMeshComponent(USplineComponent& SplineComponent);

	bool CanAddPointAfterLoadingResource(const int32 Index, const FVector& Location, USplineComponent& SplineComponent) const;

	virtual void PopRecordedLocation_Internal(USplineComponent& SplineComponent, const EVisualizationDataListPopType PopType);

private:
	UPROPERTY()
	TMap<TWeakObjectPtr<const URecordedLocationVisualizationComponent>, TObjectPtr<USplineComponent>> OwnerMap;

	UPROPERTY()
	TMap<TObjectPtr<USplineComponent>, FSplineMeshArrayWrapper> SplineMeshComponentMap;

	UPROPERTY()
	TMap<TObjectPtr<USplineComponent>, FStaticMeshArrayWrapper> StaticMeshComponentMap;

	//UPROPERTY()
	//TArray<TObjectPtr<USplineMeshComponent>> SplineMeshComponents;

	UPROPERTY(EditAnywhere, Category = "PredictionLine|Mesh", meta = (AllowPrivateAccess = true))
	TSoftObjectPtr<UStaticMesh> LineMesh;

	UPROPERTY(EditAnywhere, Category = "PredictionLine|Mesh", meta = (AllowPrivateAccess = true))
	TSoftObjectPtr<UStaticMesh> LocationItemMesh;
};
