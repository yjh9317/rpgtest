// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MenuTabInterface.generated.h"

UINTERFACE(MinimalAPI)
class UMenuTabInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RPGSYSTEM_API IMenuTabInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Menu Tab")
	void OnTabOpened();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Menu Tab")
	void OnTabClosed();
};
