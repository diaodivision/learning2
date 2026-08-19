// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable/HighLightInterface.h"
#include "Interactable/InteractableTargetInterface.h"
#include "Interactable/InteractionOptionsBuilder.h"
#include "Misc/Optional.h"
#include "Interactive/ActorWidgetControllableInterface.h"
#include "Interface/FreezableInterface.h"
#include <type_traits>
#include "GameplayAbilities/Public/AbilitySystemInterface.h"
#include "InteractableActorBase.generated.h"

class UStaticMeshComponent;
class UGameplayAbility;
class UMyGameplayAbilityBase;
class UActorWidgetComponent;
class UBoxComponent;
class UMyAbilitySystemComponent;
class UUserWidget;

UCLASS(BlueprintType, Blueprintable, Abstract)
class LEARNING2_API AInteractableActorBase :
	public AActor,
	public IHighLightInterface,
	public IInteractableTargetInterface,
	public IActorWidgetControllableInterface,
	public IFreezableInterface,
	public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AInteractableActorBase();

	virtual void GatherInteractionOptions_Implementation(const FInteractionQuery& InteractQuery);
	virtual void ShowOptions_Implementation(const FInteractionQuery& InteractQuery);
	virtual void HideOptions_Implementation();
	virtual bool IsShow_Implementation() const;

	//virtual bool CanActivateAbility(UMyGameplayAbilityBase* GA, UMyGameplayAbilityBase* OtherGA, UAbilitySystemComponent* InstigatorASC, UAbilitySystemComponent* TargetASC) const;
	UFUNCTION(BlueprintCallable, Category = "Interactable Option", meta = (DisplayName = "Activate Option"))
	bool K2_ActivateOption(int32 Index, FGameplayTag TriggerTag, FGameplayEventData Payload);
	virtual bool ActivateOption(int32 Index);

	virtual void Freeze_Implementation() override;
	virtual void Unfreeze_Implementation() override;
	virtual FORCEINLINE bool IsFreezing_Implementation() override { return bIsFreezing; };

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpec& Spec) const;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PostRegisterAllComponents() override;
	virtual void PostUnregisterAllComponents() override;

	void InitAbilities();

	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	void OnGiveAbility(const FGameplayAbilitySpec& AbilitySpec, const UMyAbilitySystemComponent* InAbilitySystemComponent);
	virtual void OnGiveAbility_Implementation(const FGameplayAbilitySpec& AbilitySpec, const UMyAbilitySystemComponent* InAbilitySystemComponent);

	UFUNCTION(BlueprintNativeEvent)
	void OnRemoveAbility(const FGameplayAbilitySpec& AbilitySpec, const UMyAbilitySystemComponent* InAbilitySystemComponent);
	virtual void OnRemoveAbility_Implementation(const FGameplayAbilitySpec& AbilitySpec, const UMyAbilitySystemComponent* InAbilitySystemComponent);

	void InitWidget();

	UFUNCTION(BlueprintPure, Category = "Interaction Option")
	UInteractionOptionBase* GetOptionByIndex(int32 Index) { return OptionsBuilder.GetOption(Index); };

	UFUNCTION(BlueprintImplementableEvent, Category = "Collision")
	bool IsOverlappingActorByCollisionComponent(const AActor* Other) const;

	virtual void InitializeDelegates() {};
	virtual void DeinitializeDelegates() {};

	virtual void BindAbility(UMyGameplayAbilityBase& GA1, UAbilitySystemComponent& ASC1, UMyGameplayAbilityBase& GA2, UAbilitySystemComponent& ASC2, const FInteractionQuery& InteractQuery);

	virtual void OnInteractionOptionsUpdated();

	UFUNCTION()
	virtual void OnAbilityStateChanged(EActionState OldState, EActionState NewState);

	//virtual void OnUIInitialized_Implementation(UUserWidget* UserWidget) override;

	virtual void PostAbilityOptionRecorded(UInteractionOptionBase* Option, const FRecordedDataObjectHandle Handle);

protected:
	InteractionOptionTypes::OptionGroupIDType GetOptionGroupIDByAbilityInstance(const UGameplayAbility* AbilityInstance) const;

	FORCEINLINE void ClearInvalidDataInOptionToAbilityMap()
	{
		for (auto It{ OptionToAbilityIndexMap.CreateIterator() }; It; ++It)
		{
			if (!It.Key().IsValid()) { It.RemoveCurrent(); }
		}
	}

	FORCEINLINE void RemoveOptionIfSameGroupActivating()
	{
		using GroupIDType = InteractionOptionTypes::OptionGroupIDType;
		TMap<GroupIDType, const UInteractionOptionBase*> GroupIDToActivatingOptionMap;

		for (auto It{ OptionsBuilder.CreateConstIterator() }; It; ++It)
		{
			const TStrongObjectPtr<UInteractionOptionBase>& Option{ (*It) };
			if (Option.IsValid() && Option->IsActivating())
			{
				if (!ensureAlways(GroupIDToActivatingOptionMap.Contains(Option->GetGroupID()))) { return; }
				GroupIDToActivatingOptionMap.Add(Option->GetGroupID(), Option.Get());
			}
		}

		for (auto It{ GroupIDToActivatingOptionMap.CreateConstIterator() }; It; ++It)
		{
			OptionsBuilder.RemoveAllOptionByGroupID(It.Key(), It.Value());
		}

		OptionsBuilder.Sort([this](const TStrongObjectPtr<UInteractionOptionBase>& A, const TStrongObjectPtr<UInteractionOptionBase>& B)
			{return OptionToAbilityIndexMap[A.Get()] < OptionToAbilityIndexMap[B.Get()]; });


	}

protected:
	UPROPERTY(BlueprintReadWrite, Category = "Interaction Options")
	FInteractionOptionsBuilder OptionsBuilder;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Options")
	TArray<FOptionInfo> OptionClasses;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI"/*, meta = (AllowPrivateAccess = "true")*/)
	UActorWidgetComponent* WidgetComponent;
	//TObjectPtr<UWidgetComponent> WidgetComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<UBoxComponent> BoxComponent;

	UPROPERTY(BlueprintReadOnly, Category = Actor)
	TWeakObjectPtr<AActor> RequestingAvatar;

	TOptional<FInteractionQuery> CachedInteractionQuery;

	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> GivenAbilityHandles;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	//TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay Abilities")
	TObjectPtr<UMyAbilitySystemComponent> AbilitySystemComponent;

	TMap<TWeakObjectPtr<UGameplayAbility>, TWeakObjectPtr<UInteractionOptionBase>> AbilityToOptionMap;

	TMap<TWeakObjectPtr<UInteractionOptionBase>, int32> OptionToAbilityIndexMap;

	bool bIsFreezing{ false };
};
