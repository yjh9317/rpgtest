// CraftingTypes.h
#pragma once
#include "CoreMinimal.h"
#include "CraftingJob.generated.h"

class UCraftingRecipe;

/** 제작 대기열/진행 상태를 관리하기 위한 런타임 데이터 */
USTRUCT(BlueprintType)
struct FCraftingJob
{
	GENERATED_BODY()

	/** 어떤 레시피를 제작 중인가? */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<const UCraftingRecipe> Recipe;

	/** 남은 시간 (초) */
	UPROPERTY(BlueprintReadOnly)
	float TimeRemaining = 0.0f;

	/** 전체 소요 시간 (진행률 계산용) */
	UPROPERTY(BlueprintReadOnly)
	float TotalDuration = 0.0f;

	/** 고유 ID (취소 등에 사용) */
	UPROPERTY(BlueprintReadOnly)
	FGuid JobID;

	FCraftingJob() : JobID(FGuid::NewGuid()) {}
};