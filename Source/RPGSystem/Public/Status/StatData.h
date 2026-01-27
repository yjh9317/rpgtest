
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StatData.generated.h"

UENUM(BlueprintType)
enum class EModifierSourceType : uint8
{
	Flat,        // +10 Attack
	Percentage,  // +20% Speed
	Override     // = 100 (최종값 강제)
};

USTRUCT(BlueprintType)
struct FStatModifier
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag ModifierTag;  // 고유 ID (아이템 ID, 버프 ID)

	UPROPERTY()
	EModifierSourceType ModifierType = EModifierSourceType::Flat;

	UPROPERTY()
	float ModifierValue = 0.0f;

	UPROPERTY()
	float Duration = -1.0f;  // -1 = 영구

	UPROPERTY()
	float StartTime = 0.0f;

	UPROPERTY()
	int32 Priority = 0;  // 적용 순서 (낮을수록 먼저)

	FStatModifier() = default;

	FStatModifier(const FGameplayTag& InTag, float InValue, 
		EModifierSourceType InType = EModifierSourceType::Flat, float InDuration = -1.0f)
		: ModifierTag(InTag), ModifierType(InType), ModifierValue(InValue), Duration(InDuration)
	{}

	bool IsExpired(float CurrentTime) const
	{
		return Duration > 0.0f && (CurrentTime >= StartTime + Duration);
	}

	float GetRemainingTime(float CurrentTime) const
	{
		if (Duration <= 0.0f) return -1.0f;
		return FMath::Max(0.0f, StartTime + Duration - CurrentTime);
	}
};