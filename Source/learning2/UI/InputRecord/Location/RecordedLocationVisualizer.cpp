// Fill out your copyright notice in the Description page of Project Settings.


#include "RecordedLocationVisualizer.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "RecordedLocationVisualizationComponent.h"
#include "Components/SceneComponent.h"
#include "BlueprintFunctionLibrary/RewindSystemVisualizationBlueprintLibrary.h"

// Sets default values
ARecordedLocationVisualizer::ARecordedLocationVisualizer()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* VisualizerRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VisualizerRoot"));
	VisualizerRoot->SetMobility(EComponentMobility::Static);
	SetRootComponent(VisualizerRoot);
	if (UPrimitiveComponent * PrimitiveComponent{ Cast<UPrimitiveComponent>(GetRootComponent()) })
	{
		PrimitiveComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
		PrimitiveComponent->SetCollisionObjectType(ECC_WorldStatic);
		PrimitiveComponent->SetNotifyRigidBodyCollision(true);
		PrimitiveComponent->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	}
}

void ARecordedLocationVisualizer::PushRecordedLocation(const FVector& Location, const bool bNeedPlaceItem, const URecordedLocationVisualizationComponent& OwnerComponent)
{
	if (!OwnerMap.Contains(&OwnerComponent)) { CreateAndSetUpSplineComponent(OwnerComponent); }
	USplineComponent* SplineComponent{ OwnerMap.FindRef(&OwnerComponent) };

	SplineComponent->AddSplinePoint(Location, ESplineCoordinateSpace::World, false);

	const int32 NumberOfSplinePoints{ SplineComponent->GetNumberOfSplinePoints() };
	UE_LOG(LogTemp, Error, TEXT("ARecordedLocationVisualizer::PushRecordedLocation NumberOfSplinePoints %d"), NumberOfSplinePoints);
	SplineComponent->SetSplinePointType(NumberOfSplinePoints - 1, ESplinePointType::Curve);

	const int32 Index{ NumberOfSplinePoints - 1 };
	if (NumberOfSplinePoints >= 2)
	{
		if (!LineMesh.IsNull()) {
			LineMesh.LoadAsync(FLoadSoftObjectPathAsyncDelegate::CreateWeakLambda(this,
				[this, Index, Location, WeakComponent = MakeWeakObjectPtr(SplineComponent)](const FSoftObjectPath& Path, UObject* InMesh) {
					if (!WeakComponent.IsValid()) { return; }

					UStaticMesh* Mesh{ Cast<UStaticMesh>(InMesh) };
					if (!Mesh) { return; }

					OnLineMeshLoaded(Index, Location, *WeakComponent.Get(), Mesh);
				}));
		}
		else
		{
			OnLineMeshLoaded(Index, Location, *SplineComponent, nullptr);
		}
	}

	if (bNeedPlaceItem && !LocationItemMesh.IsNull())
	{
		LocationItemMesh.LoadAsync(FLoadSoftObjectPathAsyncDelegate::CreateWeakLambda(this,
			[this, Index, Location, WeakComponent = MakeWeakObjectPtr(SplineComponent)](const FSoftObjectPath& Path, UObject* InMesh) {
				if (!WeakComponent.IsValid()) { return; }

				UStaticMesh* Mesh{ Cast<UStaticMesh>(InMesh) };
				if (!Mesh) { return; }

				OnLocationItemMeshLoaded(Index, Location, *WeakComponent.Get(), Mesh);
			}));
	}
}

void ARecordedLocationVisualizer::PopRecordedLocation(const URecordedLocationVisualizationComponent& OwnerComponent)
{
	TObjectPtr<USplineComponent>* SplineComponent{ OwnerMap.Find(&OwnerComponent) };
	if (!SplineComponent) { return; }

	PopRecordedLocation_Internal(**SplineComponent, EVisualizationDataListPopType::PopLast);
}

void ARecordedLocationVisualizer::PopRecordedLocationAtFront(const URecordedLocationVisualizationComponent& OwnerComponent)
{
	TObjectPtr<USplineComponent>* SplineComponent{ OwnerMap.Find(&OwnerComponent) };
	if (!SplineComponent) { return; }

	PopRecordedLocation_Internal(**SplineComponent, EVisualizationDataListPopType::PopFront);
}

void ARecordedLocationVisualizer::OnOwnerComponentDestroyed(const URecordedLocationVisualizationComponent& OwnerComponent)
{
	TObjectPtr<USplineComponent>* SplineComponent{ OwnerMap.Find(&OwnerComponent) };
	if (!SplineComponent) { return; }

	if (FSplineMeshArrayWrapper * ArrayWrapper{ SplineMeshComponentMap.Find(*SplineComponent) })
	{
		for (TObjectPtr<USplineMeshComponent>& SplineMesh : ArrayWrapper->MeshComponents)
		{
			URewindSystemVisualizationBlueprintLibrary::ReleaseSplineMeshComponentToPool(*SplineMesh);
		}

		SplineMeshComponentMap.Remove(*SplineComponent);
	}

	if (FStaticMeshArrayWrapper* ArrayWrapper{ StaticMeshComponentMap.Find(*SplineComponent) }; ArrayWrapper && !ArrayWrapper->MeshComponents.IsEmpty())
	{
		for (TObjectPtr<UStaticMeshComponent>& StaticMeshComponent : ArrayWrapper->MeshComponents)
		{
			URewindSystemVisualizationBlueprintLibrary::ReleaseStaticMeshComponentToPool(*StaticMeshComponent);
		}

		StaticMeshComponentMap.Remove(*SplineComponent);
	}
}

void ARecordedLocationVisualizer::OnLineMeshLoaded(const int32 Index, const FVector& Location, USplineComponent& SplineComponent, UStaticMesh* Mesh)
{
	//if (Index % 10 != 0) { return; }

	if (!CanAddPointAfterLoadingResource(Index, Location, SplineComponent)) { return; }

	//if (Index - 10 < 0) { return; }

	USplineMeshComponent* SplineMeshComponent{ CreateAndSetUpSplineMeshComponent(SplineComponent) };
	if (!SplineMeshComponent) { return; }

	SplineMeshComponent->SetForwardAxis(ESplineMeshAxis::X);
	//SplineMeshComponent->SetSplineUpDir(FVector(0.f, 0.f, 1.f));
	SplineMeshComponent->SetStaticMesh(Mesh);

	FVector StartLocation;
	FVector StartTangent;
	FVector EndLocation;
	FVector EndTangent;

	SplineComponent.GetLocationAndTangentAtSplinePoint(Index - 2, StartLocation, StartTangent, ESplineCoordinateSpace::World);
	SplineComponent.GetLocationAndTangentAtSplinePoint(Index - 1, EndLocation, EndTangent, ESplineCoordinateSpace::World);

	SplineMeshComponent->SetStartAndEnd(StartLocation, StartTangent, EndLocation, EndTangent);
}

void ARecordedLocationVisualizer::OnLocationItemMeshLoaded(const int32 Index, const FVector& Location, USplineComponent& SplineComponent, UStaticMesh* Mesh)
{
	if (!CanAddPointAfterLoadingResource(Index, Location, SplineComponent)) { return; }

	UStaticMeshComponent* StaticMeshComponent{ CreateAndSetUpStaticMeshComponent(SplineComponent) };
	if (!StaticMeshComponent) { return; }

	StaticMeshComponent->SetStaticMesh(Mesh);
	StaticMeshComponent->SetWorldLocation(Location);
}

USplineComponent* ARecordedLocationVisualizer::CreateAndSetUpSplineComponent(const URecordedLocationVisualizationComponent& OwnerComponent)
{
	ensureAlways(!OwnerMap.Contains(&OwnerComponent));

	USplineComponent* SplineComponent{ NewObject<USplineComponent>(this) };
	SplineComponent->SetupAttachment(RootComponent);
	SplineComponent->ClearSplinePoints();

	OwnerMap.Add(&OwnerComponent, SplineComponent);

	return SplineComponent;
}

USplineMeshComponent* ARecordedLocationVisualizer::CreateAndSetUpSplineMeshComponent(const URecordedLocationVisualizationComponent& OwnerComponent)
{
	TObjectPtr<USplineComponent>* SplineComponent{ OwnerMap.Find(&OwnerComponent) };
	if (!SplineComponent) { return nullptr; }

	return CreateAndSetUpSplineMeshComponent(**SplineComponent);
}

USplineMeshComponent* ARecordedLocationVisualizer::CreateAndSetUpSplineMeshComponent(USplineComponent& SplineComponent)
{
	USplineMeshComponent* SplineMeshComponent{ URewindSystemVisualizationBlueprintLibrary::NewSplineMeshComponentFromPool(this) };
	SplineMeshComponent->SetMobility(EComponentMobility::Movable);
	SplineMeshComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);

	SplineMeshComponent->SetVisibility(true);
	SplineMeshComponent->bHiddenInGame = false;

	SplineMeshComponentMap.FindOrAdd(&SplineComponent).MeshComponents.Add(SplineMeshComponent);

	return SplineMeshComponent;
}

UStaticMeshComponent* ARecordedLocationVisualizer::CreateAndSetUpStaticMeshComponent(USplineComponent& SplineComponent)
{
	UStaticMeshComponent* StaticMeshComponent{ URewindSystemVisualizationBlueprintLibrary::NewStaticMeshComponentFromPool(this) };
	StaticMeshComponent->SetMobility(EComponentMobility::Movable);
	StaticMeshComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);

	StaticMeshComponent->SetVisibility(true);
	StaticMeshComponent->bHiddenInGame = false;

	StaticMeshComponentMap.FindOrAdd(&SplineComponent).MeshComponents.Add(StaticMeshComponent);

	return StaticMeshComponent;
}

bool ARecordedLocationVisualizer::CanAddPointAfterLoadingResource(const int32 Index, const FVector& Location, USplineComponent& SplineComponent) const
{
	if (SplineComponent.GetNumberOfSplinePoints() <= Index) { return false; }

	const bool bIsSameLocation{ SplineComponent.GetSplinePointAt(Index, ESplineCoordinateSpace::World).Position.Equals(Location) };
	return bIsSameLocation;
}

void ARecordedLocationVisualizer::PopRecordedLocation_Internal(USplineComponent& SplineComponent, const EVisualizationDataListPopType PopType)
{
	if (const int32 NumberOfSplinePoints{ SplineComponent.GetNumberOfSplinePoints() }; NumberOfSplinePoints == 0)
	{
		SplineComponent.RemoveSplinePoint(NumberOfSplinePoints - 1);
	}

	if (FSplineMeshArrayWrapper* ArrayWrapper{ SplineMeshComponentMap.Find(&SplineComponent) }; ArrayWrapper && !ArrayWrapper->MeshComponents.IsEmpty())
	{
		TArray<TObjectPtr<USplineMeshComponent>>& Array{ ArrayWrapper->MeshComponents };
		const int32 Index{ PopType == EVisualizationDataListPopType::PopFront ? 0 : Array.Num() - 1 };

		URewindSystemVisualizationBlueprintLibrary::ReleaseSplineMeshComponentToPool(*Array[Index]);
		Array.RemoveAt(Index);

		if (Array.IsEmpty()) { SplineMeshComponentMap.Remove(&SplineComponent); }
	}

	if (FStaticMeshArrayWrapper* ArrayWrapper{ StaticMeshComponentMap.Find(&SplineComponent) }; ArrayWrapper && !ArrayWrapper->MeshComponents.IsEmpty())
	{
		TArray<TObjectPtr<UStaticMeshComponent>>& Array{ ArrayWrapper->MeshComponents };
		const int32 Index{ PopType == EVisualizationDataListPopType::PopFront ? 0 : Array.Num() - 1 };

		URewindSystemVisualizationBlueprintLibrary::ReleaseStaticMeshComponentToPool(*Array[Index]);
		Array.RemoveAt(Index);

		if (Array.IsEmpty()) { StaticMeshComponentMap.Remove(&SplineComponent); }
	}
}