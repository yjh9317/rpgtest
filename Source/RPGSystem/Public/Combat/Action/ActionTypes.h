

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ActionTypes.generated.h"

UENUM(BlueprintType)
enum class EActionFailure : uint8
{
	None,
	IsRunning,
	IsBlocked,
	NotEnoughResource,
	InCooldown
};

USTRUCT(BlueprintType)
struct FActionCost
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag CostStatTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float CostValue = 0.f;
};



// UENUM(BlueprintType)
// enum class EActionState : uint8
// {
// 	Ready       UMETA(DisplayName = "Ready"),        // 액션을 시작할 수 있는 준비된 상태
// 	Active      UMETA(DisplayName = "Active"),       // 액션이 현재 활성화(실행)되어 있는 상태
// 	Cooldown    UMETA(DisplayName = "Cooldown"),     // 액션이 끝나고 재사용 대기시간에 있는 상태
// 	Cancelled   UMETA(DisplayName = "Cancelled"),    // 액션이 강제로 중단된 상태
// 	Failed      UMETA(DisplayName = "Failed")        // 액션 실행에 실패한 상태
// };
//
// UENUM(BlueprintType)
// enum class EActionType : uint8
// {
// 	Active      UMETA(DisplayName = "Active"),       // 직접 발동형 액션
// 	Passive     UMETA(DisplayName = "Passive"),      // 지속 효과형 액션
// 	Toggle      UMETA(DisplayName = "Toggle"),       // 토글형 액션
// 	Instant     UMETA(DisplayName = "Instant")       // 즉시 실행형 액션
// };
//
// UENUM(BlueprintType)
// enum class EActionPriority : uint8
// {
// 	Low         = 0,
// 	Normal      = 1,
// 	High        = 2,
// 	Critical    = 3     // 다른 모든 액션을 중단시킬 수 있음
// };
