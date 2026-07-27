//#pragma once
//
//#include "CoreMinimal.h"
//#include "GameplayAbilitySpecHandle.h"
//#include "AbilityCombiner.generated.h"
//
//UENUM(BlueprintType)
//enum class ECombineType : uint8
//{
//	OnStarted,
//	OnEnded
//};
//
///**
// * 技能组合器
// */
//USTRUCT(BlueprintType)
//struct FAbilityCombiner
//{
//	GENERATED_BODY()
//
//public:
//	FAbilityCombiner() = defalut;
//
//	FAbilityCombiner(FGameplayAbilitySpecHandle CombinedAbilityHandle, FGameplayAbilitySpecHandle AbilityHandle, ECombineType CombineType)
//		:CombinedAbilityHandle(CombinedAbilityHandle)
//		, AbilityHandle(AbilityHandle)
//		, CombineType(CombineType)
//	{
//	}
//
//	
//
//public:
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ability)
//	FGameplayAbilitySpecHandle CombinedAbilityHandle;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ability)
//	FGameplayAbilitySpecHandle AbilityHandle;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ability)
//	ECombineType CombineType{ ECombineType::OnStarted };
//};