#include "LoadingScreenWidget.h"

void ULoadingScreenWidget::UpdateLoadingInfo(const FLoadingInfo& LoadingInfo)
{
    // K2_UpdateLoadingInfo(LoadingInfo);

    CachedInfo = LoadingInfo;

    UE_LOG(
        LogTemp,
        Error,
        TEXT("UpdateLoadingInfo this=%p Count=%p Total=%p"),
        this,
        Count.Get(),
        Total.Get()
    );

    // Count->SetText(FText::FromString(TEXT("AAAAAAAA")));
    // Total->SetText(FText::FromString(TEXT("BBBBBBBB")));

    // auto Text = Count->GetText();
    // Count->SetText(Total->GetText());
    // Total->SetText(Text);

    UE_LOG(
        LogTemp,
        Error,
        TEXT("Count Text = %s"),
        *Count->GetText().ToString()
    );

    UE_LOG(
        LogTemp,
        Error,
        TEXT("Total Text = %s"),
        *Total->GetText().ToString()
    );
    if (Count)
    {
        FText DisplayText = FText::AsNumber(LoadingInfo.CurrentPSOCount);

        Count->SetText(DisplayText);
        // 关键：强行通知 Slate 渲染线程刷新该节点
        // Count->SynchronizeProperties();
        // Count->InvalidateLayoutAndVolatility();
    }

    if (Total)
    {
        FText DisplayText = FText::AsNumber(LoadingInfo.TotalPSOCount);

        Total->SetText(DisplayText);
        
        // 关键：强行通知 Slate 渲染线程刷新该节点
        // Total->SynchronizeProperties();
        // Total->InvalidateLayoutAndVolatility();
    }
}

void ULoadingScreenWidget::NativeConstruct()
{
    Super::NativeConstruct();

UE_LOG(
        LogTemp,
        Error,
        TEXT("========== NativeConstruct ==========")
    );

    UE_LOG(
        LogTemp,
        Error,
        TEXT("========== NativeConstruct ==========")
    );

    UE_LOG(
        LogTemp,
        Error,
        TEXT("this=%p Name=%s Outer=%p"),
        this,
        *GetFullName(),
        GetOuter()
    );
}

void ULoadingScreenWidget::NativeDestruct()
{
    UE_LOG(
        LogTemp,
        Error,
        TEXT("========== NativeDestruct ==========")
    );

    UE_LOG(
        LogTemp,
        Error,
        TEXT("this=%p Name=%s Outer=%p"),
        this,
        *GetFullName(),
        GetOuter()
    );

    UE_LOG(
        LogTemp,
        Error,
        TEXT("Count=%p"),
        Count.Get()
    );

    Super::NativeDestruct();
}