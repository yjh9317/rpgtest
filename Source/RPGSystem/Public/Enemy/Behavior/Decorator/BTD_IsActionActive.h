// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTD_IsActionActive.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UBTD_IsActionActive : public UBTDecorator
{
	GENERATED_BODY()
public:
	UBTD_IsActionActive(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(EditAnywhere, Category = "Action")
	FGameplayTag ActionTag;

	UPROPERTY(EditAnywhere, Category = "Action")
	bool bInvert = false;

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
