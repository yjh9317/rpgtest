// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActionComponent.h"
#include "UObject/Interface.h"
#include "ActionOwner.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UActionOwner : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RPGSYSTEM_API IActionOwner
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual UActionComponent* GetActionComponent() const = 0;

	// 편의 함수들 (원하면 생략 가능)
	virtual bool ExecuteActionByTag(const FGameplayTag& ActionTag)
	{
		if (UActionComponent* AC = GetActionComponent())
		{
			return AC->ExecuteAction(ActionTag);
		}
		return false;
	}

	virtual UBaseAction* GetActionByTag(const FGameplayTag& ActionTag) const
	{
		if (const UActionComponent* AC = GetActionComponent())
		{
			return AC->GetAction(ActionTag);
		}
		return nullptr;
	}
};
