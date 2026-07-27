// Fill out your copyright notice in the Description page of Project Settings.

#include "RewindSystemVisualizationBlueprintLibrary.h"
#include "Components/SplineMeshComponent.h"

USplineMeshComponent* URewindSystemVisualizationBlueprintLibrary::NewSplineMeshComponentFromPool(AActor* InOuter/*const UObject* WorldContextObject*/)
{
	UWorld* World{ GEngine->GetWorldFromContextObject(InOuter, EGetWorldErrorMode::LogAndReturnNull) };
	if (!World) { return nullptr; }

	USplineMeshComponent* SplineMeshComponent = NewObject<USplineMeshComponent>(World);
	SplineMeshComponent->PrimaryComponentTick.bCanEverTick = false;
	SplineMeshComponent->Rename(nullptr, InOuter);
	SplineMeshComponent->SetMobility(EComponentMobility::Static);
	SplineMeshComponent->SetVisibility(true);
	SplineMeshComponent->bHiddenInGame = false;
	SplineMeshComponent->RegisterComponent();

	return SplineMeshComponent;
}

void URewindSystemVisualizationBlueprintLibrary::ReleaseSplineMeshComponentToPool(USplineMeshComponent& SplineMeshComponent)
{
	SplineMeshComponent.DestroyComponent();
}

UStaticMeshComponent* URewindSystemVisualizationBlueprintLibrary::NewStaticMeshComponentFromPool(AActor* InOuter)
{
	UWorld* World{ GEngine->GetWorldFromContextObject(InOuter, EGetWorldErrorMode::LogAndReturnNull) };
	if (!World) { return nullptr; }

	UStaticMeshComponent* StaticMeshComponent = NewObject<UStaticMeshComponent>(World);
	StaticMeshComponent->PrimaryComponentTick.bCanEverTick = false;
	StaticMeshComponent->Rename(nullptr, InOuter);
	StaticMeshComponent->SetMobility(EComponentMobility::Static);
	StaticMeshComponent->SetVisibility(true);
	StaticMeshComponent->bHiddenInGame = false;
	StaticMeshComponent->RegisterComponent();
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	return StaticMeshComponent;
}

void URewindSystemVisualizationBlueprintLibrary::ReleaseStaticMeshComponentToPool(UStaticMeshComponent& StaticMeshComponent)
{
	StaticMeshComponent.DestroyComponent();
}