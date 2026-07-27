#include "ActorWidget.h"
#include "WidgetController.h"
#include "Engine/Texture2D.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Styling/SlateBrush.h"
#include "Components/Image.h"

void UActorWidget::UpdateInteractionOptions(TArray<UInteractionOptionBase*> InOptions)
{
	Options.Empty();
	for (UInteractionOptionBase* Option : InOptions)
	{
		Options.Add(Option);
	}

	K2_UpdateInteractionOptions(InOptions);
}

void UActorWidget::SetOptionActive(bool bIsActive, const UInteractionOptionBase& InOption)
{
	if (TWeakObjectPtr<UInteractionOptionBase> *Option{ Options.FindByKey(&InOption) })
	{
		(*Option)->SetWillBeActivate(bIsActive);
		K2_SetOptionActive(bIsActive, Option->Get());
	}
}

//bool UActorWidget::IsOptionActive(const FWidgetInteractiveOption& Option) const
//{
//	if (Options.Contains(Option)) { return Option.IsActive(); }
//
//	return false;
//}

void UActorWidget::SetIconResource(TSoftObjectPtr<UTexture2D> InIconResource)
{
	// 取消旧加载
	if (LoadingHandle.IsValid())
	{
		LoadingHandle->CancelHandle();
		LoadingHandle.Reset();
	}

	IconResource = InIconResource;

	if (IconResource.IsNull())
	{
		if (Icon)
		{
			Icon->SetBrush(FSlateBrush());
		}
		return;
	}

	FSoftObjectPath Path = IconResource.ToSoftObjectPath();

	LoadingHandle =
		UAssetManager::GetStreamableManager().RequestAsyncLoad(
			Path,
			FStreamableDelegate::CreateWeakLambda(
				this,
				[this, Path]()
				{
					if (!IsValid(this))
					{
						return;
					}

					// 防止旧请求覆盖新请求
					if (Path != IconResource.ToSoftObjectPath())
					{
						return;
					}

					UTexture2D* Texture =
						Cast<UTexture2D>(IconResource.Get());

					if (Texture && Icon)
					{
						Icon->SetBrushFromTexture(Texture);
					}
				}));
}