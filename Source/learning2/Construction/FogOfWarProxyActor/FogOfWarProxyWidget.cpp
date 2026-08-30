#include "FogOfWarProxyWidget.h"

void UFogOfWarProxyWidget::ShowWidget(const ESlateVisibility InVisibility, AFogOfWarProxyActor* InFogOfWarProxyActor)
{
    FogOfWarProxyActor = InFogOfWarProxyActor;
    SetVisibility(InVisibility);
}

void UFogOfWarProxyWidget::SetVisibility(ESlateVisibility InVisibility)
{
    Super::SetVisibility(InVisibility);
    K2_OnVisibilityChanged(InVisibility);
}