// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/GEEC_WeaponDamage.h"

struct DamageStatics
{
	// 捕获目标的护甲属性
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArmorThickness);

	// 如果需要捕获源或其他属性，可以继续添加
	// DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower);

	DamageStatics()
	{
		// 定义如何捕获目标的护甲属性
		// 参数：属性类，目标（Target），是否快照
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMyAttributeSet, ArmorThickness, Target, false);
	}
};

// 单例模式，方便在其他地方引用
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

	// 获取Effect Spec，里面包含我们之前设置的SetByCaller数据
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	float Damage{ Spec.GetSetByCallerMagnitude(DamageTag) };
	UE_LOG(LogTemp, Warning, TEXT("Damage: %f"), Damage);

	float WeaponPenetration{ Spec.GetSetByCallerMagnitude(WeaponPenetrationTag) };
	UE_LOG(LogTemp, Warning, TEXT("WeaponPenetration: %f"), WeaponPenetration);

	float TargetArmorThickness{ 0.f };
	if (!ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		GetDamageStatics().ArmorThicknessDef, // 我们要获取的属性定义
		FAggregatorEvaluateParameters(), // 评估参数
		TargetArmorThickness // 输出值
	))
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("TargetArmorThickness: %f"), TargetArmorThickness);
	TargetArmorThickness = FMath::Max(0.f, TargetArmorThickness);

	float OutputDamage{ CalculateFinalDamage(Damage, WeaponPenetration, TargetArmorThickness) };
	ensure(OutputDamage >= 0);

	OutExecutionOutput.AddOutputModifier(
		FGameplayModifierEvaluatedData(UMyAttributeSet::GetCurrentHealthAttribute(), // 目标属性
			EGameplayModOp::Additive, // 操作类型：减法（通过负数实现）
			-OutputDamage // 注意这里是负数，因为我们要减少生命值
		)
	);
}
