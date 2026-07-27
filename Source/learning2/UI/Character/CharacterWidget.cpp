//#include "ActorWidget.h"
//#include "WidgetController.h"
//#include "Engine/Texture2D.h"
//#include "Engine/AssetManager.h"
//#include "Engine/StreamableManager.h"
//#include "Styling/SlateBrush.h"
//#include "Components/Image.h"
//
//void UActorWidget::UpdateInteractionOptions(TArray<FWidgetInteractiveOption>&& InOptions)
//{
//	Options = MoveTemp(InOptions);
//
//	K2_UpdateInteractionOptions(Options);
//}
//
//void UActorWidget::SetOptionActive(bool bIsActive, const FWidgetInteractiveOption& InOption)
//{
//	if (FWidgetInteractiveOption * Option{ Options.FindByKey(InOption) })
//	{
//		Option->SetActive(bIsActive);
//		K2_SetOptionActive(bIsActive, InOption);
//	}
//}
//
////bool UActorWidget::IsOptionActive(const FWidgetInteractiveOption& Option) const
////{
////	if (Options.Contains(Option)) { return Option.IsActive(); }
////
////	return false;
////}
//