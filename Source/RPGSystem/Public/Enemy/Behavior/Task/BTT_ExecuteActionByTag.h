// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BTT_BaseActionTask.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_ExecuteActionByTag.generated.h"

class UActionComponent;
/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UBTT_ExecuteActionByTag : public UBTT_BaseActionTask
{
	GENERATED_BODY()
public:
	UBTT_ExecuteActionByTag(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(EditAnywhere, Category = "Action")
	FGameplayTag ActionTag;

	virtual EBTNodeResult::Type ExecuteActionTask(
		UBehaviorTreeComponent& OwnerComp,
		UActionComponent* ActionComp,
		APawn* ControlledPawn) override;
};
