// Fill out your copyright notice in the Description page of Project Settings.


#include "GrenadeActorBase.h"
#include "Bullet/Base/GrenadeBulletBase.h"
#include "BlueprintFunctionLibrary/WeaponActorBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Bullet/Base/BulletBaseTypes.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Ability/TargetActor/GrenadeTargetActor.h"

AGrenadeActorBase::AGrenadeActorBase() : Super()
{
	PrimaryActorTick.bCanEverTick = true;

	CreateSplineComponent();
}

void AGrenadeActorBase::BeginPlay()
{
	Super::BeginPlay();

	{
		FActorSpawnParameters ActorSpawnParameters;
		ActorSpawnParameters.Owner = this;
		ActorSpawnParameters.Instigator = Cast<APawn>(GetOwner());
		GrenadeTargetActor = GetWorld()->SpawnActor<AGrenadeTargetActor>(GrenadeTargetActorClass, FTransform{ GetOwner()->GetActorRotation(), GetActorLocation() }, ActorSpawnParameters);
		GrenadeTargetActor->SetPredictionLineMesh(PredictionLineMesh);
		GrenadeTargetActor->SetIterationNum(IterationNum);
		GrenadeTargetActor->SetBulletClass(BulletClass);
		//GrenadeTargetActor->SetActorRotation(GetOwner()->GetActorRotation());
		const FAttachmentTransformRules AttachRules{
		EAttachmentRule::KeepWorld, EAttachmentRule::KeepRelative, EAttachmentRule::KeepWorld, false
		};
		GrenadeTargetActor->AttachToActor(GetOwner(), AttachRules);

		GrenadeTargetActor->OnStartLocationUpdatedDelegate.BindUObject(this, &AGrenadeActorBase::OnStartLocationUpdated);
		if (const AGrenadeBulletBase * BulletCDO{ Cast<AGrenadeBulletBase>(BulletClass.GetDefaultObject()) })
		{
			FGrenadeBulletAttributeData Data;
			if (UWeaponActorBlueprintLibrary::GetGrenadeBulletAttributeFromDataTable(Data, BulletCDO))
			{
				GrenadeTargetActor->SetRadius(Data.EffectiveRange);
			}
		}
	}

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

	InitialzeDelegates();
}

void AGrenadeActorBase::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	DeinitialzeDelegates();
}

void AGrenadeActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//ShowPredictionLine();
}

void AGrenadeActorBase::OnControl_Implementation(UObject* InOwner)
{
	Super::OnControl_Implementation(InOwner);

	ShowPredictionLine({ GetWeaponBulletClass(), Cast<APawn>(InOwner) });
}

void AGrenadeActorBase::OnControlReleased_Implementation()
{
	Super::OnControlReleased_Implementation();
	HidePredictionLine();
	APlayerController* PlayerController{ GetWorld()->GetFirstPlayerController() };
}

//void AGrenadeActorBase::BeginPlay()
//{
//
//}

void AGrenadeActorBase::CalculatePredictionLine()
{
	SplineComponent->ClearSplinePoints();

	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character) { return; }

	const APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
	if (!PlayerController) { return; }

	const ABulletBase* Bullet = BulletClass.GetDefaultObject();
	if (!Bullet) { return; }

	const UPrimitiveComponent* BulletRootComponent = Cast<UPrimitiveComponent>(Bullet->GetRootComponent());
	{
		FVector StartLocation{ Character->GetActorLocation() + BulletSpawnLocationOffset };
		FVector TargetLocation;
		if (!UWeaponActorBlueprintLibrary::GetLocationUnderCursorOnGround(TargetLocation, PlayerController) || TargetLocation == StartLocation) { return; }
		TargetLocation.Z = StartLocation.Z;

		SplineComponent->AddSplinePoint(StartLocation, ESplineCoordinateSpace::Local);

		const FCollisionObjectQueryParams BulletCollisionObjectQueryParams = GetBulletCollisionObjectQueryParams();
		const FCollisionShape BulletCollisionShape{ BulletRootComponent->GetCollisionShape() };
		FCollisionQueryParams CollisionQueryParams;
		CollisionQueryParams.AddIgnoredActor(GetOwner());

		int32 i{ 0 };
		do
		{
			TArray<FHitResult> HitResults;
			const bool bHit{ GetWorld()->SweepMultiByObjectType(HitResults, StartLocation, TargetLocation, GetActorRotation().Quaternion(), BulletCollisionObjectQueryParams, BulletCollisionShape, CollisionQueryParams) };

			//bool bHasValidHit{ false };
			FHitResult HitResult;
			for (const FHitResult& Result : HitResults)
			{
				if (!HitResult.HasValidHitObjectHandle() && Result.GetActor() && !Result.GetActor()->IsHidden())
				{
					HitResult = Result;
					CollisionQueryParams.AddIgnoredActor(HitResult.GetActor());
					break;
				}
				//else
				//{
				//	if (Result.GetActor()) { CollisionQueryParams.AddIgnoredActor(HitResult.GetActor()); }
				//}
			}

			if (!bHit || HitResult.Location == TargetLocation)
			{
				SplineComponent->AddSplinePoint(TargetLocation, ESplineCoordinateSpace::Local);
				break;
			}

			SplineComponent->AddSplinePoint(HitResult.Location, ESplineCoordinateSpace::Local);
			//{
			//	const FVector TraceDirection = (TargetLocation - StartLocation).GetSafeNormal();
			//	//����켣�������Ϸ���Z�ᣩ���ɵ�ƽ��ķ���
			//	const FVector PlaneNormal = FVector::CrossProduct(TraceDirection, FVector::UpVector).GetSafeNormal();
			//	//��ԭʼ������ͶӰ����ƽ���ϣ���ȥ��ƽ�淨�߷���ķ�����
			//	HitResult.ImpactNormal = HitResult.ImpactNormal - FVector::DotProduct(HitResult.ImpactNormal, PlaneNormal) * PlaneNormal;
			//}
			const FVector Direction{ FMath::GetReflectionVector(Character->GetActorForwardVector(), HitResult.ImpactNormal) };
			const double DistanceRemain{ FVector::Distance(TargetLocation , HitResult.Location) };
			TargetLocation = HitResult.Location + Direction * DistanceRemain;
			StartLocation = HitResult.Location;
		} while (i++ < IterationNum);

	}

	//{
	//	bool bHit{ GetWorld()->SweepSingleByObjectType(HitResult, GetActorLocation(), Location, GetActorRotation().Quaternion(), BulletCollisionObjectQueryParams,BulletCollisionShape) };
	//	if (!bHit || HitResult.Location == Location)
	//	{
	//		SplineComponent->AddSplinePoint(Location, ESplineCoordinateSpace::World);
	//	}
	//	else
	//	{
	//		SplineComponent->AddSplinePoint(HitResult.Location, ESplineCoordinateSpace::World);

	//		const FVector Direction{ FMath::GetReflectionVector(GetActorForwardVector(), HitResult.ImpactNormal) };
	//		const double DistanceRemain{ FVector::Distance(Location , HitResult.Location) };
	//		Location = HitResult.Location + Direction * DistanceRemain;

	//		bHit = GetWorld()->SweepSingleByObjectType(HitResult, HitResult.Location, Location, GetActorRotation().Quaternion(), BulletCollisionObjectQueryParams, BulletCollisionShape);
	//		if (!bHit || HitResult.Location == Location)
	//		{
	//			SplineComponent->AddSplinePoint(Location, ESplineCoordinateSpace::World);
	//		}
	//		else
	//		{
	//			SplineComponent->AddSplinePoint(HitResult.Location, ESplineCoordinateSpace::World);
	//		}
	//	}
	//}
}

void AGrenadeActorBase::ShowPredictionLine(const FPredictionLineParams& Params)
{
	GrenadeTargetActor->ShowPredictionLine(Params);
	if (GrenadeTargetActor) { return; }

	if (!IsActorTickEnabled()) { SetActorTickEnabled(true); }
	if (GrenadeTargetActor->IsHidden()) { GrenadeTargetActor->SetActorHiddenInGame(false); }

	CalculatePredictionLine();
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

				SplineComponent->GetLocationAndTangentAtSplinePoint(i, StartLocation, StartTangent, ESplineCoordinateSpace::Local);
				SplineComponent->GetLocationAndTangentAtSplinePoint(i + 1, EndLocation, EndTangent, ESplineCoordinateSpace::Local);

				SplineMeshComponent->SetStartAndEnd(StartLocation, StartTangent, EndLocation, EndTangent);
			}
		}
		}));
}

void AGrenadeActorBase::HidePredictionLine()
{
	GrenadeTargetActor->HidePredictionLine();
	if (GrenadeTargetActor) { return; }

	SetActorTickEnabled(false);
	GrenadeTargetActor->SetActorHiddenInGame(true);

	SplineComponent->ClearSplinePoints();
	for (const TObjectPtr<USplineMeshComponent>& SplineMeshComponentPtr : SplineMeshComponents)
	{
		SplineMeshComponentPtr->SetStaticMesh(nullptr);
		SplineMeshComponentPtr->SetVisibility(false);
		SplineMeshComponentPtr->bHiddenInGame = true;
	}
}

bool AGrenadeActorBase::IsPredictionLineVisible() const
{
	return SplineComponent && SplineComponent->GetNumberOfSplinePoints() > 1;
}

AActor* AGrenadeActorBase::SpawnBullet_Implementation() const
{
	APawn* OwnerPawn{ Cast<APawn>(GetOwner()) };
	if (!OwnerPawn) { return nullptr; }

	const APlayerController* PlayerController = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PlayerController) { return nullptr; }

	FVector TargetLocation;
	if (!UWeaponActorBlueprintLibrary::GetLocationUnderCursorOnGround(TargetLocation, PlayerController)) { return nullptr; }

	return SpawnBulletByParameters({ OwnerPawn->GetActorLocation(), TargetLocation });
}

AActor* AGrenadeActorBase::SpawnBulletByParameters_Implementation(const FSpawnGrenadeParameters& SpawnGrenadeParameters) const
{
	APawn* OwnerPawn{ Cast<APawn>(GetOwner()) };
	if (!OwnerPawn) { return nullptr; }

	const FVector& SpawnsLocation{ SpawnGrenadeParameters.GrenadeSpawnsLocation };
	const FVector& TargetLocation{ SpawnGrenadeParameters.GrenadeTargetLocation };
	const FTransform BulletSpawnTransform{ (TargetLocation - SpawnsLocation).Rotation(), SpawnsLocation };
	AGrenadeBulletBase* Bullet{ GetWorld()->SpawnActorDeferred<AGrenadeBulletBase>(BulletClass, BulletSpawnTransform, nullptr, OwnerPawn) };
	if (!Bullet) { return nullptr; }

	const AGrenadeBulletBase* BulletCDO{ Cast<AGrenadeBulletBase>(BulletClass->GetDefaultObject()) };
	if (!BulletCDO || !BulletCDO->GetGrenadeBulletSpeedFloatCurveFromTable()) { return nullptr; }

	UGrenadeBulletBaseInitData* BulletInitData = NewObject<UGrenadeBulletBaseInitData>();
	BulletInitData->Damage = GetDamage();
	BulletInitData->Penetration = GetPenetration();
	BulletInitData->Direction = (TargetLocation - SpawnsLocation).GetSafeNormal();
	BulletInitData->SpeedRate = GetMuzzleSpeed();
	BulletInitData->LifeTime = UWeaponActorBlueprintLibrary::CalculateTimeToTarget(FVector::Distance(SpawnGrenadeParameters.GrenadeTargetLocation, SpawnsLocation), BulletInitData->SpeedRate, BulletCDO->GetGrenadeBulletSpeedFloatCurveFromTable());
	BulletInitData->EffectClass = EffectClass;

	IBulletInterface::Execute_InitializeBulletData(Bullet, BulletInitData);
	Bullet->FinishSpawning(BulletSpawnTransform);

	return Bullet;
}

void AGrenadeActorBase::CreateSplineComponent()
{
	SplineComponent = CreateDefaultSubobject<USplineComponent>(FName("PredictionLine"));
	//SplineComponent->SetupAttachment(RootComponent);

}

void AGrenadeActorBase::InitilizeWeaponAttribute()
{
	if (FWeaponAttributeData Result; ensure(UWeaponActorBlueprintLibrary::GetGrenadeWeaponAttributeFromDataTable(Result, this)))
	{
		WeaponAttribute = Result;
	}
}

void AGrenadeActorBase::InitialzeDelegates()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController) { return; }

	PlayerController->OnPossessedPawnChanged.AddDynamic(this, &AGrenadeActorBase::HandlePossessedPawnChanged);
}

void AGrenadeActorBase::DeinitialzeDelegates()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController) { return; }

	PlayerController->OnPossessedPawnChanged.RemoveAll(this);
}

inline FCollisionObjectQueryParams AGrenadeActorBase::GetBulletCollisionObjectQueryParams() const
{
	const ABulletBase* Bullet = BulletClass.GetDefaultObject();
	if (!Bullet) { return {}; }

	UPrimitiveComponent* BulletRootComponent = Cast<UPrimitiveComponent>(Bullet->GetRootComponent());
	if (!BulletRootComponent) { return {}; }

	const FCollisionResponseContainer& ResponseContainer = BulletRootComponent->GetCollisionResponseToChannels();

	FCollisionObjectQueryParams ObjectQueryParams{};

	// 3. ���ģ�����������������еĺ�������ͨ�� (��32��λ)
	for (int32 ChannelIndex = 0; ChannelIndex < ECC_MAX; ++ChannelIndex)
	{
		ECollisionChannel TestChannel = static_cast<ECollisionChannel>(ChannelIndex);

		// ֻҪ���ͨ������Ӧ�� Block���赲�����Ͷ�̬����������ѯ�б�
		if (ResponseContainer.GetResponse(TestChannel) == ECR_Block)
		{
			ObjectQueryParams.AddObjectTypesToQuery(TestChannel);
		}
	}

	return ObjectQueryParams;
}

void AGrenadeActorBase::HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	if (OldPawn != NewPawn && OldPawn == GetOwner())
	{
		HidePredictionLine();
	}
}