#include "MyUserWidget.h"
#include "WidgetController.h"
#include "Engine/Texture2D.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Styling/SlateBrush.h"
#include "Components/Image.h"

void UMyUserWidget::SetWidgetController(UWidgetController* InWidgetController)
{
	UWidgetController* OldWidgetController{ WidgetController.Get() };

	WidgetController = InWidgetController;

	if (UWidgetController* NewWidgetController{ WidgetController.Get() }; NewWidgetController != OldWidgetController)
	{
		if (OldWidgetController)
		{
			//OldWidgetController->OnPossessedPawnChanged.RemoveAll(this);
			OldWidgetController->OnPossessedCharacterChangedDelegate.RemoveAll(this);
			OldWidgetController->OnCurrentHealthChangedDelegate.RemoveAll(this);
			OldWidgetController->OnMaxHealthChangedDelegate.RemoveAll(this);
			OldWidgetController->OnRewindSubsystemStateChangedDelegate.RemoveAll(this);
			NewWidgetController->OnInputRecordComponentTickChangedDelegate.RemoveAll(this);
			OldWidgetController->OnWeaponChangedDelegate.RemoveAll(this);
			OldWidgetController->OnWeaponMagazineAmmoChangedDelegate.RemoveAll(this);
			OldWidgetController->OnWeaponMagazineAmmoMaxChangedDelegate.RemoveAll(this);
		}

		if (NewWidgetController)
		{
			//if (bIsBindOnPossessedPawnChanged) { NewWidgetController->OnPossessedPawnChanged.AddDynamic(this, &UMyUserWidget::HandlePossessedPawnChanged); }
			if (bIsBindOnPossessedCharacterChangedDelegate) { NewWidgetController->OnPossessedCharacterChangedDelegate.AddDynamic(this, &UMyUserWidget::OnPossessedCharacterChanged); }
			if (bIsBindOnCurrentHealthChangedDelegate) { NewWidgetController->OnCurrentHealthChangedDelegate.AddDynamic(this, &UMyUserWidget::OnCurrentHealthChanged); }
			if (bIsBindOnMaxHealthChangedDelegate) { NewWidgetController->OnMaxHealthChangedDelegate.AddDynamic(this, &UMyUserWidget::OnMaxHealthChanged); }
			if (bIsBindOnRewindSubsystemStateChangedDelegate) { NewWidgetController->OnRewindSubsystemStateChangedDelegate.AddDynamic(this, &UMyUserWidget::OnRewindSubsystemStateChanged); }
			if (bIsBindOnInputRecordComponentTickChangedDelegate) { NewWidgetController->OnInputRecordComponentTickChangedDelegate.AddDynamic(this, &UMyUserWidget::OnInputRecordComponentTickChanged); }
			if (bIsBindOnWeaponChangedDelegate)
			{
				NewWidgetController->OnWeaponChangedDelegate.AddDynamic(this, &UMyUserWidget::OnWeaponChanged);
			}
			if (bIsBindOnWeaponMagazineAmmoChangedDelegate) { NewWidgetController->OnWeaponMagazineAmmoChangedDelegate.AddDynamic(this, &UMyUserWidget::OnWeaponMagazineAmmoChanged); }
			if (bIsBindOnWeaponMagazineAmmoMaxChangedDelegate) { NewWidgetController->OnWeaponMagazineAmmoMaxChangedDelegate.AddDynamic(this, &UMyUserWidget::OnWeaponMagazineAmmoMaxChanged); }
		}

		OnWidgetControllerSet();
	}
}

//void UMyUserWidget::SetUpWidget(const FHUDData& Data)
//{
//	if (!WidgetController.IsValid()) { return; }
//
//
//	//WidgetController->GetWidgetData(Data);
//	OnWidgetSet(Data);
//}

void UMyUserWidget::SetIconResource(TSoftObjectPtr<UTexture2D> InIconResource)
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

//void UMyUserWidget::OnControlledWeaponMagazineAmmoChanged(EWeaponSlot WeaponSlot, int32 OldAmmo, int32 NewAmmo)
//{
//	OnWeaponAmmoChangedDelegate.Broadcast(WeaponSlot, OldAmmo, NewAmmo);
//	K2_OnWeaponAmmoChanged(WeaponSlot, OldAmmo, NewAmmo);
//}

//void UMyUserWidget::OnControlledWeaponMagazineAmmoChanged(int32 OldAmmo, int32 NewAmmo)
//{
//	OnControlledWeaponMagazineAmmoChangedDelegate.Broadcast(OldAmmo, NewAmmo);
//	K2_OnControlledWeaponMagazineAmmoChanged(OldAmmo, NewAmmo);
//}
//
//void UMyUserWidget::OnControlledWeaponReserveAmmoChanged(int32 OldAmmo, int32 NewAmmo)
//{
//	OnControlledWeaponReserveAmmoChangedDelegate.Broadcast(OldAmmo, NewAmmo);
//	K2_OnControlledWeaponReserveAmmoChanged(OldAmmo, NewAmmo);
//}