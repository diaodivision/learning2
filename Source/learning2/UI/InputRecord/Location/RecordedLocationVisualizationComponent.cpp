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

	const bool bNeedVisualizeItem{ Datas.IsEmpty() || (Datas.Num() % VisualizationStep) == 0 };

	RecordedLocationVisualizer->PushRecordedLocation(Location, bNeedVisualizeItem, *this);

	//const bool bNeedVisualizes{ Datas.IsEmpty() || Datas.Num() + 1 == VisualizationStep };

	//if (Datas.IsEmpty())
	//{
	//	UE_LOG(LogTemp, Error, TEXT("URecordedLocationVisualizationComponent::AddRecordedLocation Datas.Num() %d"), Datas.Num());
	//	UE_LOG(LogTemp, Error, TEXT("URecordedLocationVisualizationComponent::AddRecordedLocation GetNumberOfSplinePoints() %d"), GetNumberOfSplinePoints());
	//}

	//FRecordedLocationVisualizationData Data{ Tick, Location, bNeedVisualizes };
	//if (!Data.IsValid()) { return; }

	//AddSplinePoint(Data.Location, ESplineCoordinateSpace::World, false);
	//const int32 NumberOfSplinePoints{ GetNumberOfSplinePoints() };
	//if (NumberOfSplinePoints > 0) { SetSplinePointType(NumberOfSplinePoints - 1, ESplinePointType::Curve); }
	//UpdateSpline();

	//Datas.Add(Data);

	//if (bNeedVisualizes)
	//{
	//	if (NumberOfSplinePoints >= 2)
	//	{
	//		const int32 StartIndex{ NumberOfSplinePoints - 2 };
	//		const int32 EndIndex{ NumberOfSplinePoints - 1 };
	//		const int32 DataIndex{ Datas.Num() - 1 };

	//		PredictionLineMesh.LoadAsync(FLoadSoftObjectPathAsyncDelegate::CreateWeakLambda(this,
	//			[this, StartIndex, EndIndex, DataIndex, Data](const FSoftObjectPath& Path, UObject* InMesh) {
	//				const int32 NumberOfSplinePoints{ GetNumberOfSplinePoints() };
	//				if (StartIndex >= NumberOfSplinePoints || EndIndex >= NumberOfSplinePoints) { return; }

	//				if (!Datas.IsValidIndex(DataIndex) || Datas[DataIndex] != Data) { return; }
	//				UStaticMesh* Mesh{ Cast<UStaticMesh>(InMesh) };
	//				if (!Mesh) { return; }

	//				USplineMeshComponent* SplineMeshComponent{ CreateAndSetupSplineMeshComponent() };
	//				if (!SplineMeshComponent) { return; }

	//				SplineMeshComponent->SetStaticMesh(Mesh);

	//				UE_LOG(LogTemp, Error, TEXT("URecordedLocationVisualizationComponent::AddRecordedLocation StartLocation: %s"), *Datas[StartIndex].Location.ToString());
	//				UE_LOG(LogTemp, Error, TEXT("URecordedLocationVisualizationComponent::AddRecordedLocation EndLocation: %s"), *Datas[EndIndex].Location.ToString());

	//				SplineMeshComponent->SetVisibility(true);
	//				SplineMeshComponent->bHiddenInGame = false;

	//				FVector StartLocation;
	//				FVector StartTangent;
	//				FVector EndLocation;
	//				FVector EndTangent;

	//				GetLocationAndTangentAtSplinePoint(StartIndex, StartLocation, StartTangent, ESplineCoordinateSpace::World);
	//				GetLocationAndTangentAtSplinePoint(EndIndex, EndLocation, EndTangent, ESplineCoordinateSpace::World);
	//				UE_LOG(LogTemp, Error, TEXT("URecordedLocationVisualizationComponent::AddRecordedLocation StartLocation World: %s"), *StartLocation.ToString());
	//				UE_LOG(LogTemp, Error, TEXT("URecordedLocationVisualizationComponent::AddRecordedLocation EndLocation World: %s"), *EndLocation.ToString());

	//				{
	//					FVector StartLocation2;
	//					FVector StartTangent2;
	//					FVector EndLocation2;
	//					FVector EndTangent2;
	//					GetLocationAndTangentAtSplinePoint(StartIndex, StartLocation2, StartTangent2, ESplineCoordinateSpace::Local);
	//					GetLocationAndTangentAtSplinePoint(EndIndex, EndLocation2, EndTangent2, ESplineCoordinateSpace::Local);
	//					UE_LOG(LogTemp, Error, TEXT("URecordedLocationVisualizationComponent::AddRecordedLocation StartLocation2 Local: %s"), *StartLocation2.ToString());
	//					UE_LOG(LogTemp, Error, TEXT("URecordedLocationVisualizationComponent::AddRecordedLocation EndLocation2 Local: %s"), *EndLocation2.ToString());
	//				}

	//				SplineMeshComponent->SetStartAndEnd(StartLocation, StartTangent, EndLocation, EndTangent);

	//				SplineMeshComponents.Add(SplineMeshComponent);
	//			}));
	//	}
	//}


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