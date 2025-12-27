// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/CombatData.h"
#include "UObject/Interface.h"
#include "ItemFragmentInterface.generated.h"

class UItemInstance;
// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UItemFragmentInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RPGSYSTEM_API IItemFragmentInterface
{
	GENERATED_BODY()

public:
	virtual void OnEquipped(UItemInstance* Instance, AActor* Equipper) = 0;
	virtual void OnUnequipped(UItemInstance* Instance, AActor* Equipper) = 0;
};
