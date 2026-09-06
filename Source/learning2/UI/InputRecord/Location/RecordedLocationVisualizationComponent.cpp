// Fill out your copyright notice in the Description page of Project Settings.

#include "RecordedLocationVisualizationComponent.h"
#include "Components/SplineMeshComponent.h"
#include "RecordedLocationVisualizer.h"
#include "BlueprintFunctionLibrary/RewindSystemVisualizationBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"

void URecordedLocationVisualizationComponent::AddRecordedLocation(const RewindSystemTickType Tick, const FVector& Location)
{
	if (!RecordedLocationVisualizer.IsValid()) { return; }

	Datas.Add(FRecordedLocationVisualizationData{ Tick, Location });

	const bool bNeedVisualizeItem{ VisualizationStep > 0 && (Datas.IsEmpty() || (Datas.Num() % VisualizationStep) == 0) };

	RecordedLocationVisualizer->PushRecordedLocation(Location, bNeedVisualizeItem, *this);
}

void URecordedLocationVisualizationComponent::RemoveRecordedLocation(const RewindSystemTickType Tick)
{
	if (!RecordedLocationVisualizer.IsValid()) { return; }

	const int32 Index{ Datas.IndexOfByPredicate([Tick](const FRecordedLocationVisualizationData& Data) {return Data.Tick == Tick; }) };
	if (Datas.IsValidIndex(Index))
	{
		if (Index == 0) { RecordedLocationVisualizer->PopRecordedLocationAtFront(*this); }
		else if (Index + 1 == Datas.Num()) { RecordedLocationVisualizer->PopRecordedLocation(*this); };

		Datas.RemoveAt(Index);
	}

	//const int32 Index{ Datas.IndexOfByPredicate([Tick](const FRecordedLocationVisualizationData& Data) {return Data.Tick == Tick; }) };
	//const bool bIsVisualized{ Datas[Index].bIsbVisualized };

	//if (Datas.IsValidIndex(Index)) { Datas.RemoveAt(Index); }

	//if (bIsVisualized)
	//{
	//	if (SplineMeshComponents.IsValidIndex(Index))
	//	{
	//		URewindSystemVisualizationBlueprintLibrary::ReleaseSplineMeshComponentToPool(SplineMeshComponents[Index]);
	//		SplineMeshComponents.RemoveAt(Index);
	//	}

	//	RemoveSplinePoint(Index, true);
	//}
}

void URecordedLocationVisualizationComponent::BeginPlay()
{
	Super::BeginPlay();

	RecordedLocationVisualizer = Cast<ARecordedLocationVisualizer>(UGameplayStatics::GetActorOfClass(this, ARecordedLocationVisualizer::StaticClass()));
	if (!RecordedLocationVisualizer.IsValid())
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]() {
			RecordedLocationVisualizer = Cast<ARecordedLocationVisualizer>(UGameplayStatics::GetActorOfClass(this, ARecordedLocationVisualizer::StaticClass()));
			}));
	}
}

void URecordedLocationVisualizationComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);

	if (!RecordedLocationVisualizer.IsValid()) { return; }
	RecordedLocationVisualizer->OnOwnerComponentDestroyed(*this);
}