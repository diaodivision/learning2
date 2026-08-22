#include "LevelStatics.h"
#include "Engine/DataTable.h"

TArray<FLevelData> ULevelStatics::GetLevelDatas()
{
    FSoftObjectPath SoftTablePath(LevelTypeConst::LevelDataPath);
    UDataTable* DataTable = Cast<UDataTable>(SoftTablePath.TryLoad());
    if (!DataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("加载数据表失败，路径: %s"), *FString{ LevelTypeConst::LevelDataPath });
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