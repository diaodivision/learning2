// Fill out your copyright notice in the Description page of Project Settings.


#include "GEEC_WeaponReload.h"
#include "Ability/AttributeSet/MyAttributeSet.h"
#include "WeaponTypes.h"
#include "BlueprintFunctionLibrary/WeaponActorBlueprintLibrary.h"

struct ReloadStatics
{
	// 捕获目标的护甲属性
	DECLARE_ATTRIBUTE_CAPTUREDEF(MagazineAmmo1);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ReserveAmmo1);
	DECLARE_ATTRIBUTE_CAPTUREDEF(MagazineAmmo2);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ReserveAmmo2);
	DECLARE_ATTRIBUTE_CAPTUREDEF(MagazineAmmo3);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ReserveAmmo3);
	DECLARE_ATTRIBUTE_CAPTUREDEF(MagazineAmmo4);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ReserveAmmo4);
	DECLARE_ATTRIBUTE_CAPTUREDEF(MagazineAmmo5);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ReserveAmmo5);

	// 如果需要捕获源或其他属性，可以继续添加
	// DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower);

	ReloadStatics()
	{
		// 定义如何捕获目标的护甲属性
		// 参数：属性类，目标（Target），是否快照
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMyAttributeSet, MagazineAmmo1, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMyAttributeSet, ReserveAmmo1, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMyAttributeSet, MagazineAmmo2, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMyAttributeSet, ReserveAmmo2, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMyAttributeSet, MagazineAmmo3, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMyAttributeSet, ReserveAmmo3, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMyAttributeSet, MagazineAmmo4, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMyAttributeSet, ReserveAmmo4, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMyAttributeSet, MagazineAmmo5, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMyAttributeSet, ReserveAmmo5, Source, false);
	}


	//bool GetAmmoAttributeByWeaponSlot(
	//	FGameplayEffectAttributeCaptureDefinition& MagazineAmmoDefinition,
	//	FGameplayEffectAttributeCaptureDefinition& ReserveAmmoDefinition,
	//	const EWeaponSlot WeaponSlot) const;
};

// 单例模式，方便在其他地方引用
static const ReloadStatics& GetReloadStatics()
{
	static ReloadStatics Statics;
	return Statics;
}

UGEEC_WeaponReload::UGEEC_WeaponReload()
{
	RelevantAttributesToCapture.Add(GetReloadStatics().MagazineAmmo1Def);
	RelevantAttributesToCapture.Add(GetReloadStatics().ReserveAmmo1Def);
	RelevantAttributesToCapture.Add(GetReloadStatics().MagazineAmmo2Def);
	RelevantAttributesToCapture.Add(GetReloadStatics().ReserveAmmo2Def);
	RelevantAttributesToCapture.Add(GetReloadStatics().MagazineAmmo3Def);
	RelevantAttributesToCapture.Add(GetReloadStatics().ReserveAmmo3Def);
	RelevantAttributesToCapture.Add(GetReloadStatics().MagazineAmmo4Def);
	RelevantAttributesToCapture.Add(GetReloadStatics().ReserveAmmo4Def);
	RelevantAttributesToCapture.Add(GetReloadStatics().MagazineAmmo5Def);
	RelevantAttributesToCapture.Add(GetReloadStatics().ReserveAmmo5Def);
}

void UGEEC_WeaponReload::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	// 获取Effect Spec，里面包含我们之前设置的SetByCaller数据
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	const EWeaponSlot WeaponSlot{ static_cast<EWeaponSlot>(Spec.GetSetByCallerMagnitude(WeaponSlotTag)) };
	UE_LOG(LogTemp, Warning, TEXT("Spec.GetSetByCallerMagnitude(WeaponSlotTag): %f"), Spec.GetSetByCallerMagnitude(WeaponSlotTag));
	UE_LOG(LogTemp, Warning, TEXT("WeaponSlot: %d"), WeaponSlot);

	FGameplayAttribute MatchedMagazineAmmoAttribute, MatchedReserveAmmoAttribute;
	if (!ensure(UWeaponActorBlueprintLibrary::GetWeaponAttributeFromAttributeSetByWeaponSlot(MatchedMagazineAmmoAttribute, MatchedReserveAmmoAttribute, WeaponSlot)))
	{
		return;
	}

	const int32 AmountToReload{ static_cast<int32>(Spec.GetSetByCallerMagnitude(AmountToReloadTag)) };
	UE_LOG(LogTemp, Warning, TEXT("AmountToReload: %d"), AmountToReload);
	if (!ensure(AmountToReload >= 0)) { return; }

	OutExecutionOutput.AddOutputModifier(
		FGameplayModifierEvaluatedData(MatchedMagazineAmmoAttribute, // 目标属性
			EGameplayModOp::Additive, // 操作类型：减法（通过负数实现）
			AmountToReload
		)
	);
	OutExecutionOutput.AddOutputModifier(
		FGameplayModifierEvaluatedData(MatchedReserveAmmoAttribute, // 目标属性
			EGameplayModOp::Additive, // 操作类型：加法
			-AmountToReload
		)
	);
}



//void UGEEC_WeaponReload::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
//{
//	//UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
//	//UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
//
//	//if (!SourceASC || !TargetASC) { return; }
//
//	// 获取Effect Spec，里面包含我们之前设置的SetByCaller数据
//	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
//
//	int32 AmountToReload{ static_cast<int32>(Spec.GetSetByCallerMagnitude(AmountToReloadTag)) };
//	UE_LOG(LogTemp, Warning, TEXT("AmountToReload: %d"), AmountToReload);
//
//	EWeaponSlot WeaponSlot{ static_cast<std::underlying_type_t<EWeaponSlot>>(Spec.GetSetByCallerMagnitude(WeaponSlotTag)) };
//	UE_LOG(LogTemp, Warning, TEXT("AmountToReload: %d"), WeaponSlot);
//
//	FGameplayEffectAttributeCaptureDefinition MagazineAmmoDefinition;
//	FGameplayEffectAttributeCaptureDefinition ReserveAmmoDefinition;
//	if (!GetReloadStatics().GetAmmoAttributeByWeaponSlot(MagazineAmmoDefinition, ReserveAmmoDefinition, WeaponSlot)) { return; }
//
//	OutExecutionOutput.AddOutputModifier(
//		FGameplayModifierEvaluatedData(MagazineAmmoDefinition.AttributeToCapture, // 目标属性
//			EGameplayModOp::Additive, // 操作类型：减法（通过负数实现）
//			AmountToReload // 注意这里是负数，因为我们要减少生命值
//		)
//	);
//	OutExecutionOutput.AddOutputModifier(
//		FGameplayModifierEvaluatedData(ReserveAmmoDefinition., // 目标属性
//			EGameplayModOp::Additive, // 操作类型：减法（通过负数实现）
//			-AmountToReload // 注意这里是负数，因为我们要减少生命值
//		)
//	);
//}

//bool ReloadStatics::GetAmmoAttributeByWeaponSlot(
//	FGameplayEffectAttributeCaptureDefinition& MagazineAmmoDefinition,
//	FGameplayEffectAttributeCaptureDefinition& ReserveAmmoDefinition,
//	const EWeaponSlot WeaponSlot) const
//{
//	FGameplayAttribute MatchedMagazineAmmoAttribute, MatchedReserveAmmoAttribute;
//
//	if (!UWeaponActorBlueprintLibrary::GetWeaponAttributeFromAttributeSetByWeaponSlot(MatchedMagazineAmmoAttribute, MatchedReserveAmmoAttribute, WeaponSlot))
//	{
//		return false;
//	}
//
//	FGameplayEffectAttributeCaptureDefinition* MatchedMagazineAmmoDefinition, * MatchedReserveAmmoDefinition;
//	if (MatchedMagazineAmmoAttribute == MagazineAmmo1Property) { MatchedMagazineAmmoDefinition = &MagazineAmmo1Def; }
//	else if (MatchedMagazineAmmoAttribute == MagazineAmmo2Property) { MatchedMagazineAmmoDefinition = &MagazineAmmo2Def; }
//	else if (MatchedMagazineAmmoAttribute == MagazineAmmo3Property) { MatchedMagazineAmmoDefinition = &MagazineAmmo3Def; }
//	else if (MatchedMagazineAmmoAttribute == MagazineAmmo4Property) { MatchedMagazineAmmoDefinition = &MagazineAmmo4Def; }
//	else if (MatchedMagazineAmmoAttribute == MagazineAmmo5Property) { MatchedMagazineAmmoDefinition = &MagazineAmmo5Def; }
//	else { return false; }
//
//	if (MatchedReserveAmmoAttribute == ReserveAmmo1Property) { MatchedReserveAmmoDefinition = &ReserveAmmo1Def; }
//	else if (MatchedReserveAmmoAttribute == ReserveAmmo2Property) { MatchedReserveAmmoDefinition = &ReserveAmmo2Def; }
//	else if (MatchedReserveAmmoAttribute == ReserveAmmo3Property) { MatchedReserveAmmoDefinition = &ReserveAmmo3Def; }
//	else if (MatchedReserveAmmoAttribute == ReserveAmmo4Property) { MatchedReserveAmmoDefinition = &ReserveAmmo4Def; }
//	else if (MatchedReserveAmmoAttribute == ReserveAmmo5Property) { MatchedReserveAmmoDefinition = &ReserveAmmo5Def; }
//	else { return false; }
//
//	MagazineAmmoDefinition = *MatchedMagazineAmmoDefinition;
//	ReserveAmmoDefinition = *MatchedReserveAmmoDefinition;
//	return true;
//}