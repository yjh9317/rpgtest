// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/Action/BaseAction.h"
#include "RollAction.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API URollAction : public UBaseAction
{
	GENERATED_BODY()

protected:
	virtual void OnInitialized_Implementation() override;
	virtual void OnExecute_Implementation() override;
};
