#include "LevelStatics.h"
#include "Engine/DataTable.h"
#include "LevelSettings.h"

TArray<FLevelData> ULevelStatics::GetLevelDatas()
{
    const ULevelSettings* LevelSettings = GetDefault<ULevelSettings>();
    if (!LevelSettings)
    {
        UE_LOG(LogTemp, Error, TEXT("Get LevelSettings failed"));
        return TArray<FLevelData>{};
    }
    
    if (LevelSettings->LevelDatas.IsNull())
    {
        UE_LOG(LogTemp, Warning, TEXT("FogOfWarMaterial not specified"));
        return TArray<FLevelData>{};
    }

    UDataTable* DataTable{ LevelSettings->LevelDatas.IsValid() ? LevelSettings->LevelDatas.Get() : LevelSettings->LevelDatas.LoadSynchronous() };
    if (!DataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("Load LevelDatas, path: %s"), *LevelSettings->LevelDatas.ToString());
        return TArray<FLevelData>{};
    }

    if (DataTable->GetRowStruct() != FLevelData::StaticStruct())
    {
        UE_LOG(LogTemp, Error, TEXT("DataTable RowStruct not match to struct FLevelData: %s"), *LevelSettings->LevelDatas.ToString());
        return TArray<FLevelData>{};
    }

    TArray<FLevelData> Result;
    TArray<FLevelData*> AllRows;
    DataTable->GetAllRows<FLevelData>(FString{ TEXT("LoadDataTableFromPathContext") }, AllRows);
    Result.Reserve(AllRows.Num());
    for (const FLevelData* Row : AllRows)
    {
        Result.Add(*Row);
    }

    return Result;
}