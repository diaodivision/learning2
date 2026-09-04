// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponActorBlueprintLibrary.h"
#include "GameplayEffectTypes.h"
#include "Bullet/Base/FireArmBulletBase.h"
#include "WeaponBase/WeaponActorBase.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/Base/MyCharacterBase.h"
#include "WeaponTypes.h"
#include "Ability/AttributeSet/MyAttributeSet.h"
#include "Engine/DataTable.h"
#include "Engine/CurveTable.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "EngineUtils.h"
#include "Interface/BulletInterface.h"
#include "Bullet/Base/GrenadeBulletBase.h"

FGameplayEffectSpecHandle UWeaponActorBlueprintLibrary::SetSetByCaller(const UGameplayAbility* Target, const TSubclassOf<UGameplayEffect> GameplayEffectClass, const FGameplayTag DataTag, const float Magtitude)
{
	if (!Target || !GameplayEffectClass.Get() || !DataTag.IsValid()) { return FGameplayEffectSpecHandle{}; }

	FGameplayEffectSpecHandle EffectSpecHandle = Target->MakeOutgoingGameplayEffectSpec(GameplayEffectClass);
	FGameplayEffectSpec* Spec = EffectSpecHandle.Data.Get();

	if (!Spec) { return FGameplayEffectSpecHandle{}; }

	Spec->SetSetByCallerMagnitude(DataTag, Magtitude);
	return EffectSpecHandle;
}

bool UWeaponActorBlueprintLibrary::CommitAbilityCooldownSetByCaller(const UGameplayAbility* Target, const TSubclassOf<UGameplayEffect> CooldownEffectClass, const FGameplayTag CooldownTag, const float Magtitude)
{
	if (!Target || !CooldownEffectClass.Get() || !CooldownTag.IsValid() || Magtitude < 0.f) { return false; }

	FGameplayEffectSpecHandle EffectSpecHandle = SetSetByCaller(Target, CooldownEffectClass, CooldownTag, Magtitude);
	FGameplayEffectSpec* Spec = EffectSpecHandle.Data.Get();

	if (!EffectSpecHandle.IsValid() || !Spec) { return false; }

	UAbilitySystemComponent* ASC = Target->GetAbilitySystemComponentFromActorInfo();

	if (!ASC) { return false; }

	ASC->ApplyGameplayEffectSpecToSelf(*Spec);
	return true;
}

bool UWeaponActorBlueprintLibrary::CommitWeaponFireCostSetByCaller(const UGameplayAbility* Target, const TSubclassOf<UGameplayEffect> WeaponCostEffectClass, const FGameplayTag FireCostTag, const int32 FireCost, const FGameplayTag WeaponSlotTag, const EWeaponSlot WeaponSlot)
{
	if (!Target || !WeaponCostEffectClass.Get() || !FireCostTag.IsValid() || FireCost < 0 || !WeaponSlotTag.IsValid() || !WeaponTypePublic::IsValid(WeaponSlot))
	{
		return false;
	}

	FGameplayEffectSpecHandle EffectSpecHandle = SetSetByCaller(Target, WeaponCostEffectClass, FireCostTag, FireCost);
	FGameplayEffectSpec* Spec = EffectSpecHandle.Data.Get();
	if (!EffectSpecHandle.IsValid() || !Spec) { return false; }
	Spec->SetSetByCallerMagnitude(WeaponSlotTag, static_cast<float>(WeaponSlot));

	UAbilitySystemComponent* ASC = Target->GetAbilitySystemComponentFromActorInfo();

	if (!ASC) { return false; }

	ASC->ApplyGameplayEffectSpecToSelf(*Spec);
	return true;
}

bool UWeaponActorBlueprintLibrary::CommitWeaponReloadAmountSetByCaller(const UGameplayAbility* Target, const TSubclassOf<UGameplayEffect> WeaponReloadEffectClass, const FGameplayTag AmountToReloadTag, const int32 AmountToReload, const FGameplayTag WeaponSlotTag, const EWeaponSlot WeaponSlot)
{
	if (!Target || !WeaponReloadEffectClass.Get() || !AmountToReloadTag.IsValid() || AmountToReload < 0 || !WeaponSlotTag.IsValid() || !WeaponTypePublic::IsValid(WeaponSlot))
	{
		return false;
	}

	FGameplayEffectSpecHandle EffectSpecHandle = SetSetByCaller(Target, WeaponReloadEffectClass, AmountToReloadTag, AmountToReload);
	FGameplayEffectSpec* Spec = EffectSpecHandle.Data.Get();
	if (!EffectSpecHandle.IsValid() || !Spec) { return false; }
	Spec->SetSetByCallerMagnitude(WeaponSlotTag, static_cast<float>(WeaponSlot));

	UAbilitySystemComponent* ASC = Target->GetAbilitySystemComponentFromActorInfo();

	if (!ASC) { return false; }

	ASC->ApplyGameplayEffectSpecToSelf(*Spec);
	return true;
}

bool UWeaponActorBlueprintLibrary::GetAbilityCooldownGameplayEffectClass(TSubclassOf<UGameplayEffect>& EffectClass, const UGameplayAbility* Target)
{
	if (!Target) { return false; }

	const UGameplayEffect* GameplayEffect = Target->GetCooldownGameplayEffect();
	if (!GameplayEffect) { return false; }

	EffectClass = GameplayEffect->GetClass();
	return true;
}

bool UWeaponActorBlueprintLibrary::GetAbilityCostGameplayEffectClass(TSubclassOf<UGameplayEffect>& EffectClass, const UGameplayAbility* Target)
{
	if (!Target) { return false; }

	const UGameplayEffect* GameplayEffect = Target->GetCostGameplayEffect();
	if (!GameplayEffect) { return false; }

	EffectClass = GameplayEffect->GetClass();
	return true;
}

float UWeaponActorBlueprintLibrary::RandomWeaponSpreadRadius()
{
	return RandomWeaponSpreadRadius(UWeaponActorBlueprintLibrary::WeaponSpreadRadius);
}
float UWeaponActorBlueprintLibrary::RandomWeaponSpreadRadius(float MaxSpreadRadius)
{
	return FMath::FRandRange(-MaxSpreadRadius, MaxSpreadRadius);
}

bool UWeaponActorBlueprintLibrary::CalculateWeaponSpreadDirection(FVector Direction, float SpreadRadius, float Accuracy, FVector& OutNewDirection)
{
	if (!Direction.IsNormalized())
	{
		bool bSuccess = Direction.Normalize();
		if (!ensureMsgf(bSuccess, TEXT("Input a Zero Vector!"))) { return false; }
	}

	const FVector& ForwardVector = Direction;
	FVector RightVector = ForwardVector.RotateAngleAxis(90.0f, FVector::UpVector);

	OutNewDirection = ForwardVector * Accuracy + RightVector * RandomWeaponSpreadRadius(SpreadRadius);
	OutNewDirection.Normalize();
	return true;
}

bool UWeaponActorBlueprintLibrary::GetBulletSpawnLocation(APawn* Instigator, FVector& OutLocation)
{
	if (!ensureMsgf(Instigator, TEXT("Instigator is nullptr"))) { return false; }
	FVector ActorLocation = Instigator->GetActorLocation();
	FVector ActorUpVector = Instigator->GetActorUpVector();

	OutLocation = ActorLocation + ActorUpVector * BulletSpawnHeight;
	return true;
}

FBulletAttributeData UWeaponActorBlueprintLibrary::GetBulletAttributeFromWeapon(const AWeaponActorBase* Weapon)
{
	if (!ensureMsgf(Weapon, TEXT("Weapon is nullptr"))) { return FBulletAttributeData{}; }

	const FWeaponAttributeData WeaponAttribute{ Weapon->GetWeaponAttribute() };

	float Damage{ WeaponAttribute.Damage };
	float Penetration{ WeaponAttribute.Penetration };

	return FBulletAttributeData{ Damage , Penetration };
}

AActor* UWeaponActorBlueprintLibrary::SpawnActorDeferredFromPool(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, const FTransform& Transform)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		AActor* Actor = World->SpawnActor(ActorClass, &Transform);
		if (Actor) { Actor->SetActorTickEnabled(false); }

		return Actor;
	}

	return nullptr;
}

void UWeaponActorBlueprintLibrary::FinishSpawningOfPoolingActor(AActor* Actor)
{
	//if (Actor->Implements<UBulletInterface>()) { IBulletInterface::Execute_PostInitializedBulletData(Actor); }

	Actor->SetActorTickEnabled(true);
}

void UWeaponActorBlueprintLibrary::ReleaseActorToPool(AActor* Actor)
{
	Actor->Destroy();
}

ABulletBase* UWeaponActorBlueprintLibrary::SpawnBullet(TSubclassOf<AFireArmBulletBase> BulletClass, TSubclassOf<UGameplayEffect> DamageClass, APawn* Instigator, const AWeaponActorBase* Weapon, bool bSpread)
{
	//if (!ensureMsgf(Instigator && Weapon, TEXT("Instigator or Weapon is nullptr"))) { return nullptr; }

	//FVector Location;
	//if (!GetBulletSpawnLocation(Instigator, Location)) { return nullptr; }

	//FRotator Rotation = Instigator->GetActorRotation();

	///*FActorSpawnParameters Parameters;
	//Parameters.Instigator = Instigator;*/

	//FTransform Transform{ Rotation , Location };

	//ABulletBase* Bullet = Instigator->GetWorld()->SpawnActorDeferred<ABulletBase>(BulletClass, Transform, nullptr, Instigator);
	//if (Bullet)
	//{
	//	const FWeaponAttributeData WeaponAttribute{ Weapon->GetWeaponAttribute() };

	//	Bullet->DamageClass = DamageClass;
	//	Bullet->Speed = WeaponAttribute.MuzzleSpeed;
	//	Bullet->BulletData = GetBulletAttributeFromWeapon(Weapon);

	//	if (bSpread)
	//	{
	//		const FVector OldVector{ Bullet->GetActorForwardVector() };

	//		if (FVector NewDirection{ OldVector }; CalculateWeaponSpreadDirection(OldVector, WeaponSpreadRadius, WeaponAttribute.Accuracy, NewDirection))
	//		{
	//			Bullet->SetActorRotation(NewDirection.Rotation());
	//		}
	//	}

	//	Bullet->FinishSpawning(Transform);
	//}

	//return Bullet;
	return nullptr;
}

//bool UWeaponActorBlueprintLibrary::TriggerAbilityFromGameplayEvent(FInteractionOption Option, FGameplayTag Tag, FGameplayEventData Payload)
//{
//	if (Option.IsValid())
//	{
//		UAbilitySystemComponent* ASC = Option.TargetAbilitySystem.Get();
//
//		return ASC->TriggerAbilityFromGameplayEvent(Option.TargetInteractionAbilityHandle, ASC->AbilityActorInfo.Get(), Tag, &Payload, *ASC);
//	}
//
//	return false;
//}

bool UWeaponActorBlueprintLibrary::GetWeaponAttributeFromAttributeSet(FGameplayAttribute& MagazineAmmoAttribute, FGameplayAttribute& ReserveAmmoAttribute, const AWeaponActorBase* Weapon)
{
	if (!Weapon) { return false; }

	return GetWeaponAttributeFromAttributeSetByWeaponSlot(MagazineAmmoAttribute, ReserveAmmoAttribute, Weapon->GetWeaponSlot());
}

bool UWeaponActorBlueprintLibrary::GetWeaponAttributeFromAttributeSetByWeaponSlot(FGameplayAttribute& MagazineAmmoAttribute, FGameplayAttribute& ReserveAmmoAttribute, const EWeaponSlot WeaponSlot)
{
	const FString MagazineAmmoAttributeName = FString::Printf(TEXT("MagazineAmmo%d"), WeaponSlot);
	const FString ReserveAmmoAttributeName = FString::Printf(TEXT("ReserveAmmo%d"), WeaponSlot);

	FProperty* MagazineAmmoProp = UMyAttributeSet::StaticClass()->FindPropertyByName(*MagazineAmmoAttributeName);
	FProperty* ReserveAmmoProp = UMyAttributeSet::StaticClass()->FindPropertyByName(*ReserveAmmoAttributeName);

	if (MagazineAmmoProp && ReserveAmmoProp)
	{
		MagazineAmmoAttribute = FGameplayAttribute{ MagazineAmmoProp };
		ReserveAmmoAttribute = FGameplayAttribute{ ReserveAmmoProp };

		return true;
	}

	return false;
}

bool UWeaponActorBlueprintLibrary::GetWeaponSlotByWeaponAttribute(EWeaponSlot& WeaponSlot, const FGameplayAttribute& AmmoAttribute)
{
	FString AttributeName = AmmoAttribute.GetName();
	if (AttributeName.IsEmpty()) { return false; }

	if (const TCHAR LastChar = AttributeName[AttributeName.Len() - 1]; FChar::IsDigit(LastChar))
	{
		if (const int32 LastCharToInt = LastChar - '0'; LastCharToInt > 0 && LastCharToInt < static_cast<int32>(EWeaponSlot::MAX))
		{
			WeaponSlot = static_cast<EWeaponSlot>(LastCharToInt);
			return true;
		}
	}

	return false;
}

bool UWeaponActorBlueprintLibrary::SetWeaponAmmoByAttribute(AWeaponActorBase& Weapon, int32 NewValue, const FGameplayAttribute& AmmoAttribute)
{

	const FString MagazineAmmoAttributeName = FString::Printf(TEXT("MagazineAmmo%d"), Weapon.GetWeaponSlot());
	if (AmmoAttribute.AttributeName == MagazineAmmoAttributeName)
	{
		Weapon.SetCurrentMagazineAmmo(NewValue);

		return true;
	}

	const FString ReserveAmmoAttributeName = FString::Printf(TEXT("ReserveAmmo%d"), Weapon.GetWeaponSlot());
	if (AmmoAttribute.AttributeName == ReserveAmmoAttributeName)
	{
		Weapon.SetReserveAmmo(NewValue);

		return true;
	}

	return false;
}

bool UWeaponActorBlueprintLibrary::GetWeaponAttributeFromDataTable(FWeaponAttributeData& WeaponAttributeData, const AWeaponActorBase* Weapon)
{
	if (!Weapon) { return false; }

	const UDataTable* WeaponAttributeDataTable = Weapon->WeaponAttributeDataTable.AttributeDataTable;
	const UCurveTable* WeaponAccuracyCurveTable = Weapon->WeaponAttributeDataTable.AccuracyCurveTable;

	if (!WeaponAttributeDataTable || !WeaponAccuracyCurveTable) { return false; }

	FWeaponAttributeData Result;
	if (GetGrenadeWeaponAttributeFromDataTable(Result, Weapon))
	{
		GetWeaponAccuracy(Result.Accuracy, Weapon);
		WeaponAttributeData = Result;

		return true;
	}

	return false;
}

bool UWeaponActorBlueprintLibrary::GetGrenadeWeaponAttributeFromDataTable(FWeaponAttributeData& WeaponAttributeData, const AWeaponActorBase* Weapon)
{
	if (!Weapon) { return false; }

	const UDataTable* WeaponAttributeDataTable = Weapon->WeaponAttributeDataTable.AttributeDataTable;
	if (!WeaponAttributeDataTable) { return false; }

	FDataTableRowHandle Handle;
	Handle.DataTable = WeaponAttributeDataTable;
	Handle.RowName = Weapon->GetWeaponName();
	if (FWeaponAttributeData* AttributeData = Handle.GetRow<FWeaponAttributeData>(*FString("Getting Weapon Attribute")))
	{
		AttributeData->MagazineAmmo = AttributeData->MagazineAmmoMax;
		WeaponAttributeData = *AttributeData;
		return true;
	}

	return false;
}

bool UWeaponActorBlueprintLibrary::GetGrenadeBulletAttributeFromDataTable(FGrenadeBulletAttributeData& GrenadeBulletAttributeData, const AGrenadeBulletBase* Bullet)
{
	if (!Bullet) { return false; }

	const UDataTable* GrenadeBulletAttributeDataTable = Bullet->AttributeDataTable;
	if (!GrenadeBulletAttributeDataTable) { return false; }

	FDataTableRowHandle Handle;
	Handle.DataTable = GrenadeBulletAttributeDataTable;
	Handle.RowName = Bullet->GetBulletName();
	if (FGrenadeBulletAttributeData* AttributeData = Handle.GetRow<FGrenadeBulletAttributeData>(*FString("Getting Grenade Bullet Attribute")))
	{
		GrenadeBulletAttributeData.EffectiveRange = AttributeData->EffectiveRange;
		GrenadeBulletAttributeData.GrenadeBulletSpeedFloatCurve = AttributeData->GrenadeBulletSpeedFloatCurve;
		GrenadeBulletAttributeData.EffectDuration = AttributeData->EffectDuration;
		return true;
	}

	return false;
}

bool UWeaponActorBlueprintLibrary::GetWeaponAccuracy(float& Accuracy, const AWeaponActorBase* Weapon)
{
	const UCurveTable* WeaponAccuracyCurveTable = Weapon->WeaponAttributeDataTable.AccuracyCurveTable;

	if (!WeaponAccuracyCurveTable || !Weapon) { return false; }

	float Result{ 0.f };

	FCurveTableRowHandle Handle;
	Handle.CurveTable = WeaponAccuracyCurveTable;
	Handle.RowName = Weapon->GetWeaponName();
	const bool bSuccess{ Handle.Eval(static_cast<float>(Weapon->GetWeaponLevel()), &Result, *FString("Getting Weapon Accuracy")) };

	if (bSuccess) { Accuracy = Result; }
	return bSuccess;
}

float UWeaponActorBlueprintLibrary::CalculateTimeToTarget(const float Distance, const float InitialSpeed, const UCurveFloat* const SpeedCurve)
{
	if (!SpeedCurve || InitialSpeed <= 0.0f || Distance <= 0.0f) { return 0.0f; }

	// 1. 使用黎曼和（Riemann Sum）计算 0~1 曲线下的面积 A
	float CurveArea = 0.0f;
	const int32 Steps = 100; // 采样步长，100次采样对于游戏来说已经极度精确且耗时可忽略
	const float DeltaU = 1.0f / Steps;

	for (int32 i = 0; i < Steps; ++i)
	{
		// 取每个小矩形中点的 U 值进行采样
		float U = (i + 0.5f) * DeltaU;
		float CurveValue = SpeedCurve->GetFloatValue(U);

		// 累加面积：高 * 宽
		CurveArea += CurveValue * DeltaU;
	}

	// 防御代码：防止策划画出总面积为 0 的无效曲线
	if (CurveArea <= 0.0f) return 0.0f;

	// 2. 根据公式 T = S / (V0 * A) 算出总时间
	float TotalTime = Distance / (InitialSpeed * CurveArea);

	return TotalTime;
}

float UWeaponActorBlueprintLibrary::CalculateDistanceToTarget(const float Time, const float InitialSpeed, const UCurveFloat* const SpeedCurve)
{
	if (!SpeedCurve || InitialSpeed <= 0.0f || Time <= 0.0f) { return 0.0f; }

	// 1. 使用黎曼和（Riemann Sum）计算 0~1 曲线下的面积 A
	float CurveArea = 0.0f;
	const int32 Steps = 100; // 采样步长，100次采样对于游戏来说已经极度精确且耗时可忽略
	const float DeltaU = 1.0f / Steps;

	for (int32 i = 0; i < Steps; ++i)
	{
		// 取每个小矩形中点的 U 值进行采样
		float U = (i + 0.5f) * DeltaU;
		float CurveValue = SpeedCurve->GetFloatValue(U);

		// 累加面积：高 * 宽
		CurveArea += CurveValue * DeltaU;
	}

	// 防御代码：防止策划画出总面积为 0 的无效曲线
	if (CurveArea <= 0.0f) return 0.0f;

	// 2. 根据公式 T = S / (V0 * A) 算出总时间
	return InitialSpeed * CurveArea * Time;
}

bool UWeaponActorBlueprintLibrary::GetLocationUnderCursorOnGround(FVector& Location, const APlayerController* PlayerController)
{
	if (!PlayerController) { return false; }
	const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	const AActor* Actor = PlayerController->GetPawn();
	if (!ensure(LocalPlayer) || !ensure(LocalPlayer->ViewportClient) || !ensure(Actor)) { return false; }

	FVector2D MousePosition;
	if (LocalPlayer->ViewportClient->GetMousePosition(MousePosition))
	{
		FVector WorldOrigin;
		FVector WorldDirection;
		UGameplayStatics::DeprojectScreenToWorld(PlayerController, MousePosition, WorldOrigin, WorldDirection);

		//地面的法向量
		const FVector NormalOfGround = FVector::UpVector;

		//从摄像机出发，沿鼠标方向到 Actor 活动的平面的距离
		const float DistanceFromCursorToGround = FMath::Abs(FVector::DotProduct(NormalOfGround, WorldOrigin - Actor->GetActorLocation()) / FVector::DotProduct(NormalOfGround, WorldDirection));

		Location = WorldOrigin + WorldDirection * DistanceFromCursorToGround;

		{
			FVector ActorOrigin, ActorBoxExtent;
			Actor->GetActorBounds(false, ActorOrigin, ActorBoxExtent);

			Location.Z = ActorOrigin.Z - ActorBoxExtent.Z;
		}

		return true;
	}

	return false;
}

// 传入：发射途径点、初速度、速度曲线、采样点数
TArray<FVector> UWeaponActorBlueprintLibrary::CalculateProjectilePredictionPath(
	bool& bSuccess,
	const TArray<FVector>& PathPoints,
	const float InitialSpeed,
	const UCurveFloat* SpeedCurve,
	const int32 SamplingNum,
	const float SampleDistanceMin)
{
	bSuccess = false;
	TArray<FVector> ResultPoints;

	// 1. 基础合法性检查
	if (PathPoints.Num() < 2 || !SpeedCurve || InitialSpeed <= 0.f || SampleDistanceMin <= 0.f)
	{
		return ResultPoints;
	}

	// 2. 首先算出整条折线的总物理长度
	float TotalPathLength = 0.f;
	TArray<float> SegmentLengths; // 记录每段线段的累积长度，方便后续快速做位置插值
	SegmentLengths.Add(0.f);

	for (int32 i = 0; i < PathPoints.Num() - 1; ++i)
	{
		float SegLen = FVector::Dist(PathPoints[i], PathPoints[i + 1]);
		TotalPathLength += SegLen;
		SegmentLengths.Add(TotalPathLength);
	}

	if (TotalPathLength <= 0.f) return ResultPoints;

	// 3. 预计算 0~1 曲线的总面积（用于时间重映射映射）
	float TotalCurveArea = 0.f;
	const float AreaDelta = 1.0f / 100.f;
	for (int32 i = 0; i < 100; ++i)
	{
		TotalCurveArea += SpeedCurve->GetFloatValue((i + 0.5f) * AreaDelta) * AreaDelta;
	}
	if (TotalCurveArea <= 0.f) return ResultPoints;

	// 整个行程理论上需要的总时间 T = S / (V0 * Area)
	const float TotalTime = TotalPathLength / (InitialSpeed * TotalCurveArea);

	// 4. 核心：沿着折线骨架进行定长步进采样
	ResultPoints.Add(PathPoints[0]); // 压入起点

	float CurrentDistanceOnPath = 0.f;
	float CurrentTime = 0.f;
	int32 CurrentSegmentIndex = 0;

	// 辅助 Lambda：根据当前在折线上的距离，精准插值算出世界坐标
	auto GetLocationAtDistance = [&](float Dist) -> FVector
		{
			// 寻找当前距离落在哪两个途径点之间
			while (CurrentSegmentIndex < PathPoints.Num() - 1 && SegmentLengths[CurrentSegmentIndex + 1] < Dist)
			{
				CurrentSegmentIndex++;
			}

			float SegStartDist = SegmentLengths[CurrentSegmentIndex];
			float SegEndDist = SegmentLengths[CurrentSegmentIndex + 1];
			float SegLen = SegEndDist - SegStartDist;

			if (SegLen <= 0.f) return PathPoints[CurrentSegmentIndex];

			float Alpha = (Dist - SegStartDist) / SegLen;
			return FMath::Lerp(PathPoints[CurrentSegmentIndex], PathPoints[CurrentSegmentIndex + 1], Alpha);
		};

	// 步进循环
	while (CurrentDistanceOnPath < TotalPathLength && ResultPoints.Num() < SamplingNum)
	{
		float NextTargetDistance = CurrentDistanceOnPath + SampleDistanceMin;
		if (NextTargetDistance > TotalPathLength)
		{
			// 如果最后一段不够一个步长，直接压入终点并结束
			ResultPoints.Add(PathPoints.Last());
			break;
		}

		// 💡 关键微积分步进：为了保证速度曲线同步，我们需要用微分累加时间
		// 因为 ds = v(t) * dt -> dt = ds / v(t)
		// 我们把 SampleDistanceMin 细切成微小的步长进行数值积分
		const int32 IntegrationSteps = 5;
		const float SubDs = SampleDistanceMin / IntegrationSteps;
		float SubDistance = CurrentDistanceOnPath;

		for (int32 k = 0; k < IntegrationSteps; ++k)
		{
			float NormalizedTime = FMath::Clamp(CurrentTime / TotalTime, 0.f, 1.f);
			float CurrentSpeed = InitialSpeed * SpeedCurve->GetFloatValue(NormalizedTime);

			// 防御弹体静止造成的死循环
			if (CurrentSpeed <= 0.01f) CurrentSpeed = 0.01f;

			float Dt = SubDs / CurrentSpeed;
			CurrentTime += Dt;
			SubDistance += SubDs;
		}

		// 更新大步进距离，并计算当前位置
		CurrentDistanceOnPath = NextTargetDistance;
		FVector ValidSamplePoint = GetLocationAtDistance(CurrentDistanceOnPath);

		// 检查：确保当前算出的点与上一个压入的点之间几何直线距离严格大于等于 SampleDistanceMin
		// (在折线发生大角度急转弯时，沿着折线走的路径距离可能大于几何直线距离，以此做硬性规约)
		if (FVector::Dist(ResultPoints.Last(), ValidSamplePoint) >= SampleDistanceMin)
		{
			ResultPoints.Add(ValidSamplePoint);
		}
	}

	bSuccess = ResultPoints.Num() > 0;
	return ResultPoints;
}

//bool UWeaponActorBlueprintLibrary::GetInteractionOption(UPARAM(ref) FInteractionOptionsBuilder& OptionsBuilder, int32 Index, FInteractionOption& OutOption)
//{
//	FInteractionOption Option = OptionsBuilder.GetOption(Index);
//	if (Option.IsValid())
//	{
//		OutOption = Option;
//		return true;
//	}
//
//	return false;
//}

//void UWeaponActorBlueprintLibrary::GetInteractionOptions(UPARAM(ref) FInteractionOptionsBuilder& OptionsBuilder, TArray<FInteractionOption>& Options)
//{
//	Options = OptionsBuilder.GetOptions();
//}

UGameplayAbility* UWeaponActorBlueprintLibrary::GetGameplayAbilityFromHandle(const UAbilitySystemComponent* ASC, const FGameplayAbilitySpecHandle& Handle, TSubclassOf<UGameplayAbility> AbilityClass, bool& bIsInstance)
{
	if (!ASC)
	{
		bIsInstance = false;
		return nullptr;
	}

	FGameplayAbilitySpec* AbilitySpec = ASC->FindAbilitySpecFromHandle(Handle);

	if (!AbilitySpec)
	{
		bIsInstance = false;
		return nullptr;
	}

	UGameplayAbility* AbilityInstance = AbilitySpec->GetPrimaryInstance();
	bIsInstance = true;

	// default to the CDO if we can't
	if (!AbilityInstance)
	{
		AbilityInstance = AbilitySpec->Ability;
		bIsInstance = false;
	}

	return AbilityInstance;
}

void UWeaponActorBlueprintLibrary::InitAbilityActorInfo(UAbilitySystemComponent* AbilitySystemComponent, AActor* InOwnerActor, AActor* InAvatarActor)
{
	AbilitySystemComponent->InitAbilityActorInfo(InOwnerActor, InAvatarActor);
}

void UWeaponActorBlueprintLibrary::PauseCharacter(ACharacter* Character)
{
	if (Character)
	{
		Character->SetActorTickEnabled(false);

		if (USkeletalMeshComponent* Mesh = Character->GetMesh())
		{
			if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
			{
				// 暂停动画更新
				Mesh->bPauseAnims = true;
			}
		}

		// 4. 暂停物理模拟
		if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Character->GetRootComponent()))
		{

			RootPrimitive->SetSimulatePhysics(false);
		}

		// 3. 暂停移动组件（包括重力）
		if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
		{
			//MoveComp->GravityScale = 0.0f;              // 取消重力
			//MoveComp->SetMovementMode(MOVE_None);       // 设置为无移动模式
			//MoveComp->StopMovementImmediately();        // 立即停止速度
			//MoveComp->Velocity = FVector::ZeroVector;   // 清空速度
			MoveComp->SetComponentTickEnabled(false);   // 暂停组件 Tick
		}
	}
}

void UWeaponActorBlueprintLibrary::ResumeCharacter(ACharacter* Character)
{
	if (Character)
	{
		Character->SetActorTickEnabled(true);

		if (USkeletalMeshComponent* Mesh = Character->GetMesh())
		{
			if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
			{
				// 暂停动画更新
				Mesh->bPauseAnims = false;
			}
		}

		// 4. 暂停物理模拟
		if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Character->GetRootComponent()))
		{
			RootPrimitive->SetSimulatePhysics(true);
		}
	}
}

bool UWeaponActorBlueprintLibrary::CommitWeaponFire(AWeaponActorBase* Weapon)
{
	if (!Weapon) { return false; }

	const int32 CurrentMagazineAmmo = Weapon->GetMagazineAmmo();
	if (CurrentMagazineAmmo < 1) { return false; }

	Weapon->SetCurrentMagazineAmmo(CurrentMagazineAmmo - 1);
	return true;
}

bool UWeaponActorBlueprintLibrary::CommitWeaponReload(int32& ClampedMovedAmmo, AWeaponActorBase* Weapon)
{
	ClampedMovedAmmo = 0;
	if (!Weapon) { return false; }

	FWeaponAttributeData WeaponAttribute = Weapon->GetWeaponAttribute();

	const int32 CurrentMagazineAmmo = WeaponAttribute.MagazineAmmo;
	const int32 ReserveAmmo = WeaponAttribute.ReserveAmmo;
	{
		const int32 WeaponLoadedAmmoMax{ WeaponAttribute.MagazineAmmoMax + WeaponAttribute.ChamberCapacity };
		const int32 NeedAmmoMoved{ WeaponLoadedAmmoMax - CurrentMagazineAmmo };
		if (!ensure(WeaponLoadedAmmoMax > 0 && NeedAmmoMoved > 0)) { return false; }

		ClampedMovedAmmo = FMath::Min(NeedAmmoMoved, WeaponAttribute.ReserveAmmo);
		if (!ensure(ClampedMovedAmmo > 0))
		{
			ClampedMovedAmmo = 0;
			return false;
		}
	}

	Weapon->SetCurrentMagazineAmmo(CurrentMagazineAmmo + ClampedMovedAmmo);
	Weapon->SetReserveAmmo(ReserveAmmo - ClampedMovedAmmo);

	return true;
}

//bool UWeaponActorBlueprintLibrary::UpdateWeaponAmmoToAttributeSet(const AActor* Owner, const AWeaponActorBase* Weapon)
//{
//	if (!Owner || !Weapon) { return false; }
//
//	UAbilitySystemComponent* ASC = Cast<UAbilitySystemComponent>(Owner->FindComponentByClass(UAbilitySystemComponent::StaticClass()));
//	if (!ASC) { return false; }
//
//	const UMyAttributeSet* AttributeSet = Cast<UMyAttributeSet>(ASC->GetAttributeSet(UMyAttributeSet::StaticClass()));
//	if (!AttributeSet) { return; }
//
//	const FString MagazineAmmoAttributeName = FString::Printf(TEXT("MagazineAmmo%d"), Weapon->GetWeaponType());
//	FProperty* MagazineAmmoProp = UMyAttributeSet::StaticClass()->FindPropertyByName(*MagazineAmmoAttributeName);
//	if (!MagazineAmmoProp) { return; }
//
//	const FString ReserveAmmoAttributeName = FString::Printf(TEXT("ReserveAmmo%d"), Weapon->GetWeaponType());
//	FProperty* ReserveAmmoProp = UMyAttributeSet::StaticClass()->FindPropertyByName(*ReserveAmmoAttributeName);
//	if (!ReserveAmmoProp) { return; }
//
//	ASC->SetNumericAttributeBase(MagazineAmmoProp, FMath::Max(0, Weapon->GetCurrentMagazineAmmo()));
//	ASC->SetNumericAttributeBase(ReserveAmmoProp, FMath::Max(0, Weapon->GetReserveAmmo()));
//
//	return false;
//}