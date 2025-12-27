// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BTT_BaseActionTask.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_WaitForAction.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UBTT_WaitForAction : public UBTT_BaseActionTask
{
	GENERATED_BODY()
public:
	UBTT_WaitForAction(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(EditAnywhere, Category = "Action")
	FGameplayTag ActionTag;

	UPROPERTY(EditAnywhere, Category = "Action")
	float Timeout = 5.0f;   // 안전용 타임아웃

	virtual EBTNodeResult::Type ExecuteActionTask(
		UBehaviorTreeComponent& OwnerComp,
		UActionComponent* ActionComp,
		APawn* ControlledPawn) override;

	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	// 이 Task 인스턴스에서만 쓸 런타임 상태
	float ElapsedTime = 0.0f;
};
