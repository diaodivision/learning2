// Fill out your copyright notice in the Description page of Project Settings.


#include "GrenadeTargetActor.h"
#include "Engine/OverlapResult.h"
#include "Abilities/GameplayAbility.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DecalComponent.h"
#include "BlueprintFunctionLibrary/WeaponActorBlueprintLibrary.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Bullet/Base/BulletBase.h"

AGrenadeTargetActor::AGrenadeTargetActor()
{
	PrimaryActorTick.bCanEverTick = true;

	DecalComponent = CreateDefaultSubobject<UDecalComponent>(FName("DecalComponent"));
	DecalComponent->PrimaryComponentTick.bCanEverTick = false;

	if (!RootComponent) { SetRootComponent(DecalComponent); }
	else if (UPrimitiveComponent * PrimitiveComponent{ Cast<UPrimitiveComponent>(RootComponent) })
	{
		PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	SplineComponent = CreateDefaultSubobject<USplineComponent>(FName("PredictionLine"));
}

void AGrenadeTargetActor::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);

	OwningAbility = Ability;
	SourceActor = Ability->GetOwningActorFromActorInfo();
	PrimaryPC = Cast<APlayerController>(SourceActor->GetInstigatorController());
}
//
//void AGrenadeTargetActor::ConfirmTargetingAndContinue()
//{
// Super::ConfirmTargetingAndContinue();
//	FGameplayAbilityTargetDataHandle TargetDataHandle;
//
//	TArray<FOverlapResult> OverlapResults;
//	bool bResult = GetWorld()->OverlapMultiByObjectType(OverlapResults, GetActorLocation(), FQuat::Identity, FCollisionObjectQueryParams(ECC_Pawn), FCollisionShape::MakeSphere(Radius), FCollisionQueryParams{});
//
//	TArray<TWeakObjectPtr<AActor>> OverlapActors;
//	if (bResult)
//	{
//		for (const FOverlapResult& Result : OverlapResults)
//		{
//			if (APawn* OverlappedPawn = Cast<APawn>(Result.GetActor()))
//			{
//				OverlapActors.AddUnique(OverlappedPawn);
//			}
//		}
//	}
//
//	if (OverlapActors.Num() > 0)
//	{
//		const FGameplayAbilityTargetDataHandle TargetData = StartLocation.MakeTargetDataHandleFromActors(OverlapActors);
//		TargetDataReadyDelegate.Broadcast(TargetData);
//	}
//
//	/*TArray<AActor*> Result;
//	UGameplayStatics::GetAllActorsOfClass(GetWorld(), TSubclassOf<ACharacter>{}, Result);*/
//}

void AGrenadeTargetActor::ConfirmTargetingAndContinue()
{
	// 1. 创建坐标信息结构体 (使用 FGameplayAbilityTargetData_LocationInfo)
	FGameplayAbilityTargetData_LocationInfo* LocationData = new FGameplayAbilityTargetData_LocationInfo();

	// 2. 设置起点和终点 (例如：起点为玩家位置，终点为技能作用点)
	LocationData->SourceLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
	LocationData->SourceLocation.LiteralTransform = FTransform{ SourceActor->GetActorLocation() };

	LocationData->TargetLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
	LocationData->TargetLocation.LiteralTransform = FTransform{ GetActorLocation() };

	// 3. 构建 TargetDataHandle 并添加该数据
	FGameplayAbilityTargetDataHandle TargetDataHandle;

	// 注意：Add 接收的是 raw pointer，Handle 内部的 SharedPtr 会自动接管内存释放，无需手动 delete
	TargetDataHandle.Add(LocationData);

	// 4. 将 Handle 广播给 WaitTargetData Task
	TargetDataReadyDelegate.Broadcast(TargetDataHandle);
}

void AGrenadeTargetActor::SetActorHiddenInGame(bool bNewHidden)
{
	Super::SetActorHiddenInGame(bNewHidden);

	SetActorTickEnabled(!bNewHidden);
}

void AGrenadeTargetActor::SetRadius(const float InRadius)
{
	Radius = InRadius;
	DecalComponent->DecalSize = FVector{ Radius };
}

void AGrenadeTargetActor::ShowPredictionLine(const FPredictionLineParams& Params)
{
	if (GetInstigator() != Params.Instigator) { SetInstigator(Params.Instigator); }
	if (BulletClass != Params.BulletClass) { SetBulletClass(Params.BulletClass); }

	if (!IsActorTickEnabled()) { SetActorTickEnabled(true); }
	if (IsHidden()) { SetActorHiddenInGame(false); }

	CalculatePredictionLine();

	UpdatePredictionLine();
}

void AGrenadeTargetActor::HidePredictionLine()
{
	SetActorTickEnabled(false);
	SetActorHiddenInGame(true);

	SplineComponent->ClearSplinePoints();
	for (const TObjectPtr<USplineMeshComponent>& SplineMeshComponentPtr : SplineMeshComponents)
	{
		SplineMeshComponentPtr->SetStaticMesh(nullptr);
		SplineMeshComponentPtr->SetVisibility(false);
		SplineMeshComponentPtr->bHiddenInGame = true;
	}
}

bool AGrenadeTargetActor::IsPredictionLineVisible() const
{
	return SplineComponent && SplineComponent->GetNumberOfSplinePoints() > 1;
}

void AGrenadeTargetActor::ShowPredictionLineStatic(const FPredictionLineParams& Params, const FVector& TargetLocation)
{
	SetInstigator(Params.Instigator);
	SetBulletClass(Params.BulletClass);

	HidePredictionLine();
	SetActorHiddenInGame(false);
	SetActorTickEnabled(false);

	CalculatePredictionLine(TargetLocation);
	UpdatePredictionLine();
}

void AGrenadeTargetActor::BeginPlay()
{
	Super::BeginPlay();

	SetActorHiddenInGame(true);

	DecalComponent->SetRelativeRotation(FRotator{ 90., 0., 0. });

	auto InitializeDelegates = [WeakThis = MakeWeakObjectPtr(this)]() {
		if (!WeakThis.IsValid()) { return false; }

		APlayerController* PlayerController{ WeakThis->GetWorld() ? WeakThis->GetWorld()->GetFirstPlayerController() : nullptr };
		if (!PlayerController) { return false; }

		PlayerController->OnPossessedPawnChanged.AddUniqueDynamic(WeakThis.Get(), &AGrenadeTargetActor::OnPossessedPawnChanged);
		return true;
		};

	if (!InitializeDelegates()) { GetWorld()->GetTimerManager().SetTimerForNextTick(InitializeDelegates); }

	//if (PrimaryActorTick.bCanEverTick)
	//{
	//	RegisterAllActorTickFunctions(true, true); // 强制向引擎的 Tick 管理器注册
	//}

	HidePredictionLine();
	for (int32 i = 0; i < IterationNum + 1; i++)
	{
		USplineMeshComponent* SplineMeshComponent = NewObject<USplineMeshComponent>(this);
		//SplineMeshComponent->SetStaticMesh(PredictionLineMesh);
		SplineMeshComponent->SetMobility(EComponentMobility::Movable);
		SplineMeshComponent->SetForwardAxis(ESplineMeshAxis::X);
		SplineMeshComponent->RegisterComponent();

		SplineMeshComponent->SetVisibility(false);
		SplineMeshComponent->bHiddenInGame = true;

		SplineMeshComponents.Add(SplineMeshComponent);
	}
}

void AGrenadeTargetActor::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	APlayerController* PlayerController{ GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr };
	if (!PlayerController) { return; }

	PlayerController->OnPossessedPawnChanged.RemoveAll(this);
}

void AGrenadeTargetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const UWorld* World{ GetWorld() };
	if (!World) { return; }

	FVector CursorLocation;
	if (!UWeaponActorBlueprintLibrary::GetLocationUnderCursorOnGround(CursorLocation, World->GetFirstPlayerController())) { return; }

	SetActorLocation(CursorLocation);

	ShowPredictionLine({ BulletClass, GetInstigator() });
}

void AGrenadeTargetActor::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	SetActorHiddenInGame(true);
}

void AGrenadeTargetActor::CalculatePredictionLine(TOptional<FVector> TargetLocation)
{
	SplineComponent->ClearSplinePoints();

	const APawn* PredictionLineInstigator{ GetInstigator() };
	if (!PredictionLineInstigator) { return; }

	const ABulletBase* Bullet = BulletClass.GetDefaultObject();
	if (!Bullet) { return; }

	const UPrimitiveComponent* BulletRootComponent = Cast<UPrimitiveComponent>(Bullet->GetRootComponent());
	{
		FVector PredictionStartLocation{ GetOwner()->GetActorLocation() };
		if (!TargetLocation.IsSet())
		{
			FVector OutTargetLocation;
			if (!UWeaponActorBlueprintLibrary::GetLocationUnderCursorOnGround(OutTargetLocation, GetWorld()->GetFirstPlayerController()) || TargetLocation == PredictionStartLocation) { return; }
			OutTargetLocation.Z = PredictionStartLocation.Z;
			TargetLocation = OutTargetLocation;
		}

		SplineComponent->AddSplinePoint(PredictionStartLocation, ESplineCoordinateSpace::World);

		const FCollisionObjectQueryParams BulletCollisionObjectQueryParams = GetBulletCollisionObjectQueryParams();
		const FCollisionShape BulletCollisionShape{ BulletRootComponent->GetCollisionShape() };
		FCollisionQueryParams CollisionQueryParams;
		CollisionQueryParams.AddIgnoredActor(GetOwner());
		CollisionQueryParams.AddIgnoredActor(PredictionLineInstigator);

		int32 i{ 0 };
		do
		{
			TArray<FHitResult> HitResults;
			const bool bHit{ GetWorld()->SweepMultiByObjectType(HitResults, PredictionStartLocation, TargetLocation.GetValue(), PredictionLineInstigator->GetActorRotation().Quaternion(), BulletCollisionObjectQueryParams, BulletCollisionShape, CollisionQueryParams) };

			FHitResult HitResult;
			for (const FHitResult& Result : HitResults)
			{
				UE_LOG(LogTemp, Error, TEXT("Hited: %s"), *GetNameSafe(Result.GetActor()));

				if (!HitResult.HasValidHitObjectHandle() && Result.GetActor() && !Result.GetActor()->IsHidden())
				{
					HitResult = Result;
					CollisionQueryParams.AddIgnoredActor(HitResult.GetActor());
					break;
				}
			}

			if (!bHit || HitResult.Location == TargetLocation)
			{
				SplineComponent->AddSplinePoint(TargetLocation.GetValue(), ESplineCoordinateSpace::World);
				break;
			}

			SplineComponent->AddSplinePoint(HitResult.Location, ESplineCoordinateSpace::World);

			const FVector Direction{ FMath::GetReflectionVector(PredictionLineInstigator->GetActorForwardVector(), HitResult.ImpactNormal) };
			const double DistanceRemain{ FVector::Distance(TargetLocation.GetValue() , HitResult.Location) };
			TargetLocation = HitResult.Location + Direction * DistanceRemain;
			PredictionStartLocation = HitResult.Location;
		} while (i++ < IterationNum);

	}
}

void AGrenadeTargetActor::UpdatePredictionLine()
{
	for (int32 i = 0; i < SplineComponent->GetNumberOfSplinePoints(); i++)
	{
		SplineComponent->SetSplinePointType(i, ESplinePointType::Linear, false);
	}
	SplineComponent->UpdateSpline();

	PredictionLineMesh.LoadAsync(FLoadSoftObjectPathAsyncDelegate::CreateWeakLambda(this, [this](const FSoftObjectPath& Path, UObject* InMesh) {
		UStaticMesh* Mesh{ Cast<UStaticMesh>(InMesh) };
		if (!Mesh) { return; }

		const int32 NumberOfSplinePoints{ SplineComponent->GetNumberOfSplinePoints() };
		for (int32 i = 0; i < SplineMeshComponents.Num(); i++)
		{
			USplineMeshComponent* SplineMeshComponent{ SplineMeshComponents[i] };
			if (!ensure(SplineMeshComponent)) { return; }

			const bool bIsShow{ i < NumberOfSplinePoints - 1 };
			SplineMeshComponent->SetVisibility(bIsShow);
			SplineMeshComponent->SetHiddenInGame(!bIsShow);

			if (!bIsShow) { /*SplineMeshComponent->SetStartAndEnd(FVector::ZeroVector, FVector::ZeroVector, FVector::ZeroVector, FVector::ZeroVector);*/ }
			else
			{
				SplineMeshComponent->SetStaticMesh(Mesh);

				FVector StartLocation;
				FVector StartTangent;
				FVector EndLocation;
				FVector EndTangent;

				SplineComponent->GetLocationAndTangentAtSplinePoint(i, StartLocation, StartTangent, ESplineCoordinateSpace::World);
				SplineComponent->GetLocationAndTangentAtSplinePoint(i + 1, EndLocation, EndTangent, ESplineCoordinateSpace::World);

				SplineMeshComponent->SetStartAndEnd(StartLocation, StartTangent, EndLocation, EndTangent);
			}
		}
		}));
}

FCollisionObjectQueryParams AGrenadeTargetActor::GetBulletCollisionObjectQueryParams() const
{
	const ABulletBase* Bullet = BulletClass.GetDefaultObject();
	if (!Bullet) { return {}; }

	UPrimitiveComponent* BulletRootComponent = Cast<UPrimitiveComponent>(Bullet->GetRootComponent());
	if (!BulletRootComponent) { return {}; }

	const FCollisionResponseContainer& ResponseContainer = BulletRootComponent->GetCollisionResponseToChannels();

	FCollisionObjectQueryParams ObjectQueryParams{};

	// 3. 核心：遍历虚幻引擎中所有的核心物体通道 (共32个位)
	for (int32 ChannelIndex = 0; ChannelIndex < ECC_MAX; ++ChannelIndex)
	{
		ECollisionChannel TestChannel = static_cast<ECollisionChannel>(ChannelIndex);

		// 只要这个通道的响应是 Block（阻挡），就动态把它塞进查询列表
		if (ResponseContainer.GetResponse(TestChannel) == ECR_Block)
		{
			UE_LOG(LogTemp, Error, TEXT("AGrenadeActorBase::CalculatePredictionLine %d"), TestChannel);
			ObjectQueryParams.AddObjectTypesToQuery(TestChannel);
		}
	}

	return ObjectQueryParams;
}