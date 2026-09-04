// Fill out your copyright notice in the Description page of Project Settings.


#include "GEEC_WeaponFire.h"
#include "Ability/AttributeSet/MyAttributeSet.h"
#include "WeaponTypes.h"
#include "BlueprintFunctionLibrary/WeaponActorBlueprintLibrary.h"

struct FireStatics
{
	// ����Ŀ��Ļ�������
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

	// �����Ҫ����Դ���������ԣ����Լ�������
	// DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower);

	FireStatics()
	{
		// ������β���Ŀ��Ļ�������
		// �����������࣬Ŀ�꣨Target�����Ƿ����
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

// ����ģʽ�������������ط�����
static const FireStatics& GetFireStatics()
{
	static FireStatics Statics;
	return Statics;
}

UGEEC_WeaponFire::UGEEC_WeaponFire()
{
	RelevantAttributesToCapture.Add(GetFireStatics().MagazineAmmo1Def);
	RelevantAttributesToCapture.Add(GetFireStatics().ReserveAmmo1Def);
	RelevantAttributesToCapture.Add(GetFireStatics().MagazineAmmo2Def);
	RelevantAttributesToCapture.Add(GetFireStatics().ReserveAmmo2Def);
	RelevantAttributesToCapture.Add(GetFireStatics().MagazineAmmo3Def);
	RelevantAttributesToCapture.Add(GetFireStatics().ReserveAmmo3Def);
	RelevantAttributesToCapture.Add(GetFireStatics().MagazineAmmo4Def);
	RelevantAttributesToCapture.Add(GetFireStatics().ReserveAmmo4Def);
	RelevantAttributesToCapture.Add(GetFireStatics().MagazineAmmo5Def);
	RelevantAttributesToCapture.Add(GetFireStatics().ReserveAmmo5Def);
}

void UGEEC_WeaponFire::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	// ��ȡEffect Spec�������������֮ǰ���õ�SetByCaller����
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	const EWeaponSlot WeaponSlot{ static_cast<EWeaponSlot>(Spec.GetSetByCallerMagnitude(WeaponSlotTag)) };

	FGameplayAttribute MatchedMagazineAmmoAttribute, MatchedReserveAmmoAttribute;
	if (!ensure(UWeaponActorBlueprintLibrary::GetWeaponAttributeFromAttributeSetByWeaponSlot(MatchedMagazineAmmoAttribute, MatchedReserveAmmoAttribute, WeaponSlot)))
	{
		return;
	}

	const int32 FireCost{ static_cast<int32>(Spec.GetSetByCallerMagnitude(FireCostTag)) };
	if (!ensure(FireCost >= 0)) { return; }

	OutExecutionOutput.AddOutputModifier(
		FGameplayModifierEvaluatedData(MatchedMagazineAmmoAttribute, // Ŀ������
			EGameplayModOp::Additive, // �������ͣ�������ͨ������ʵ�֣�
			-FireCost
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
//	// ��ȡEffect Spec�������������֮ǰ���õ�SetByCaller����
//	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
//
//	int32 AmountToReload{ static_cast<int32>(Spec.GetSetByCallerMagnitude(AmountToReloadTag)) };
//
//	EWeaponSlot WeaponSlot{ static_cast<std::underlying_type_t<EWeaponSlot>>(Spec.GetSetByCallerMagnitude(WeaponSlotTag)) };
//
//	FGameplayEffectAttributeCaptureDefinition MagazineAmmoDefinition;
//	FGameplayEffectAttributeCaptureDefinition ReserveAmmoDefinition;
//	if (!GetReloadStatics().GetAmmoAttributeByWeaponSlot(MagazineAmmoDefinition, ReserveAmmoDefinition, WeaponSlot)) { return; }
//
//	OutExecutionOutput.AddOutputModifier(
//		FGameplayModifierEvaluatedData(MagazineAmmoDefinition.AttributeToCapture, // Ŀ������
//			EGameplayModOp::Additive, // �������ͣ�������ͨ������ʵ�֣�
//			AmountToReload // ע�������Ǹ�������Ϊ����Ҫ��������ֵ
//		)
//	);
//	OutExecutionOutput.AddOutputModifier(
//		FGameplayModifierEvaluatedData(ReserveAmmoDefinition., // Ŀ������
//			EGameplayModOp::Additive, // �������ͣ�������ͨ������ʵ�֣�
//			-AmountToReload // ע�������Ǹ�������Ϊ����Ҫ��������ֵ
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