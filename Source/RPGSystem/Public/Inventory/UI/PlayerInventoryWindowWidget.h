// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Common/RPGMenuContentWidget.h"
#include "PlayerInventoryWindowWidget.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UPlayerInventoryWindowWidget : public URPGMenuContentWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPlayerInventoryWidget> PlayerInventoryWidget;
};
