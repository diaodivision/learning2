#pragma once

#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"
#include "PlayerResponseGameplayTags.generated.h"

namespace PlayerResponseTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Debuff_Stun);      // Ñ£ÔÎ (ÉÁ¹âµ¯/Õð±¬µ¯)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Debuff_Blind);     // ÖÂÃ¤
}

UENUM(BlueprintType)
enum class ETagCountChangeType : uint8
{
	Increase,
	Decrease,
	NoChange
};