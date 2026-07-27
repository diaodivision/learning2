//Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputRecordedDataTypes/RecordedDataDefines.h"
#include "RecordedLocationVisualizationTypes.generated.h"

class USplineMeshComponent;
class UStaticMeshComponent;

enum class EVisualizationDataListPopType : uint8
{
	PopFront,
	PopLast
};

struct FRecordedLocationVisualizationData
{
	FORCEINLINE bool IsValid() const { return Tick != REWIND_TICK_NONE; }
	FORCEINLINE bool operator==(const FRecordedLocationVisualizationData& Other) const
	{
		return Tick == Other.Tick && Location.Equals(Other.Location);
	}

	RewindSystemTickType Tick{ REWIND_TICK_NONE };

	FVector Location{ FVector::ZeroVector };
};

USTRUCT()
struct FSplineMeshArrayWrapper
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<USplineMeshComponent>> MeshComponents;
};

USTRUCT()
struct FStaticMeshArrayWrapper
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> MeshComponents;
};

//struct FRecordedLocationVisualizationDataHandle
//{
//	FRecordedLocationVisualizationDataHandle() = default;
//
//	FRecordedLocationVisualizationDataHandle(const FRecordedLocationVisualizationData& Data) : Data(Data)
//	{
//		GenerateNewHandle();
//	}
//
//	FORCEINLINE bool IsValid() const { return Handle != INDEX_NONE && Data.IsValid(); }
//	FORCEINLINE bool operator==(const FRecordedLocationVisualizationDataHandle& Other) const { return Handle == Other.Handle && Data == Other.Data; }
//
//	friend uint32 GetTypeHash(const FRecordedLocationVisualizationDataHandle& Handle)
//	{
//		return ::GetTypeHash(Handle.Handle);
//	}
//
//private:
//	void GenerateNewHandle();
//
//	FRecordedLocationVisualizationData Data;
//
//private:
//	int32 Handle{ INDEX_NONE };
//};