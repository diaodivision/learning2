// Fill out your copyright notice in the Description page of Project Settings.

#include "MySaveGame.h"
#include "Algo/MaxElement.h"

void UMySaveGame::Add(FPlayerData Data)
{
	Data.ID = GetNextID();
	SaveGameData.Add(Data);
}

void UMySaveGame::SetDataByID(FPlayerData NewData, int32 ID)
{
	int32 Index = SaveGameData.IndexOfByPredicate([ID](const FPlayerData& OldData) { return ID == OldData.ID; });

	if (Index != INDEX_NONE) { SaveGameData[Index] = NewData; }
}

FPlayerData UMySaveGame::GetDataByID(int32 ID)
{
	if (ID < GetNextID())
	{
		FPlayerData* Data = SaveGameData.FindByPredicate([ID](const FPlayerData& Data) {return ID == Data.ID; });

		if (Data) { return *Data; }
	}

	return FPlayerData{};
}

int UMySaveGame::GetNextID()
{
	if (SaveGameData.Num() == 0) { return 1; }

	const FPlayerData* MaxElement = Algo::MaxElementBy(SaveGameData,
		[](const FPlayerData& Data) { return Data.ID; });

	return MaxElement->ID + 1;
}