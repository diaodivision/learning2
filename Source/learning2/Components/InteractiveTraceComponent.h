// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractiveTraceComponent.generated.h"

class AActor;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class LEARNING2_API UInteractiveTraceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UInteractiveTraceComponent();

	virtual void BeginPlay() override;


protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Actor")
	void K2_OnHoveredActorChanged(AActor* OldActor, AActor* NewActor);
	virtual void OnHoveredActorChanged(AActor* OldActor, AActor* NewActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Actor")
	void K2_OnSelectedActorChanged(AActor* OldActor, AActor* NewActor);
	virtual void OnSelectedActorChanged(AActor* OldActor, AActor* NewActor);

private:
	TWeakObjectPtr<AActor> HoveredActor;

	float HoveredStartSeconds{ 0.f };
	// Called every frame
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
