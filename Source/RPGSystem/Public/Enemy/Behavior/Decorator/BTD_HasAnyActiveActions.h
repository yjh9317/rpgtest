// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTD_HasAnyActiveActions.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UBTD_HasAnyActiveActions : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UBTD_HasAnyActiveActions(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(EditAnywhere, Category = "Action")
	bool bInvert = false;    // true면 "액션이 없을 때"를 조건으로 사용

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
