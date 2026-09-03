#include "LoadingScreenWidget.h"

void ULoadingScreenWidget::UpdateLoadingInfo(const FLoadingInfo& LoadingInfo)
{
    K2_UpdateLoadingInfo(LoadingInfo);

    // Count->SetText(FText::FromString(TEXT("AAAAAAAA")));
    // Total->SetText(FText::FromString(TEXT("BBBBBBBB")));

    // auto Text = Count->GetText();
    // Count->SetText(Total->GetText());
    // Total->SetText(Text);

    // if (Count)
    // {
    //     FText DisplayText = FText::AsNumber(LoadingInfo.CurrentPSOCount);

    //     Count->SetText(DisplayText);
    //     // 关键：强行通知 Slate 渲染线程刷新该节点
    //     // Count->SynchronizeProperties();
    //     // Count->InvalidateLayoutAndVolatility();
    // }

    // if (Total)
    // {
    //     FText DisplayText = FText::AsNumber(LoadingInfo.TotalPSOCount);

    //     Total->SetText(DisplayText);
        
    //     // 关键：强行通知 Slate 渲染线程刷新该节点
    //     // Total->SynchronizeProperties();
    //     // Total->InvalidateLayoutAndVolatility();
    // }
}