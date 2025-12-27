// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_BaseActionTask.generated.h"

class UActionComponent;
/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UBTT_BaseActionTask : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTT_BaseActionTask(const FObjectInitializer& ObjectInitializer);

protected:
	// 자식에서 오버라이드해서 실제 로직만 구현
	virtual EBTNodeResult::Type ExecuteActionTask(UBehaviorTreeComponent& OwnerComp, UActionComponent* ActionComp, APawn* ControlledPawn);

	// UBTTaskNode 인터페이스
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
