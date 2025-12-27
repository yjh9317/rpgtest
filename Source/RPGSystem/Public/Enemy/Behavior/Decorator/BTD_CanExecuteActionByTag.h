// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTD_CanExecuteActionByTag.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UBTD_CanExecuteActionByTag : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UBTD_CanExecuteActionByTag(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(EditAnywhere, Category = "Action")
	FGameplayTag ActionTag;

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
