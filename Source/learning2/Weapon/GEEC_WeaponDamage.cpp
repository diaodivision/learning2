// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/GEEC_WeaponDamage.h"

struct DamageStatics
{
	// ����Ŀ��Ļ�������
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArmorThickness);

	// �����Ҫ����Դ���������ԣ����Լ�������
	// DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower);

	DamageStatics()
	{
		// ������β���Ŀ��Ļ�������
		// �����������࣬Ŀ�꣨Target�����Ƿ����
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMyAttributeSet, ArmorThickness, Target, false);
	}
};

// ����ģʽ�������������ط�����
static const DamageStatics& GetDamageStatics()
{
	static DamageStatics Statics;
	return Statics;
}

UGEEC_WeaponDamage::UGEEC_WeaponDamage()
{
	RelevantAttributesToCapture.Add(GetDamageStatics().ArmorThicknessDef);
}

void UGEEC_WeaponDamage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	if (!SourceASC || !TargetASC) { return; }

	// ��ȡEffect Spec�������������֮ǰ���õ�SetByCaller����
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	float Damage{ Spec.GetSetByCallerMagnitude(DamageTag) };

	float WeaponPenetration{ Spec.GetSetByCallerMagnitude(WeaponPenetrationTag) };

	float TargetArmorThickness{ 0.f };
	if (!ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		GetDamageStatics().ArmorThicknessDef, // ����Ҫ��ȡ�����Զ���
		FAggregatorEvaluateParameters(), // ��������
		TargetArmorThickness // ���ֵ
	))
	{
		return;
	}

	TargetArmorThickness = FMath::Max(0.f, TargetArmorThickness);

	float OutputDamage{ CalculateFinalDamage(Damage, WeaponPenetration, TargetArmorThickness) };
	ensure(OutputDamage >= 0);

	OutExecutionOutput.AddOutputModifier(
		FGameplayModifierEvaluatedData(UMyAttributeSet::GetCurrentHealthAttribute(), // Ŀ������
			EGameplayModOp::Additive, // �������ͣ�������ͨ������ʵ�֣�
			-OutputDamage // ע�������Ǹ�������Ϊ����Ҫ��������ֵ
		)
	);
}
