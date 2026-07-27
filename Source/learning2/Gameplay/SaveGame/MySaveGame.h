// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MySaveGame.generated.h"

USTRUCT(BlueprintType)
struct FPlayerData
{
	GENERATED_BODY()

	friend UMySaveGame;
public:
	UPROPERTY(BlueprintReadWrite)
	int32 Exp{ 0 };

protected:
	UPROPERTY(BlueprintReadOnly, Category = Save)
	int32 ID{ 0 };
};

/**
 *
 */
UCLASS(Blueprintable, BlueprintType)
class LEARNING2_API UMySaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = Save)
	void Add(FPlayerData Data);

	UFUNCTION(BlueprintCallable, Category = Save)
	void SetDataByID(FPlayerData NewData, int32 ID);

	UFUNCTION(BlueprintCallable, Category = Save)
	FPlayerData GetDataByID(int32 ID);

protected:
	UPROPERTY(BlueprintReadOnly, Category = Save)
	TArray<FPlayerData> SaveGameData;

	int32 GetNextID();
};
