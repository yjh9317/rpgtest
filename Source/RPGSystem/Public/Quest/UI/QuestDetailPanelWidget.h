// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QuestDetailPanelWidget.generated.h"

class UTextBlock;
class URPGQuestData;
/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UQuestDetailPanelWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	// UPROPERTY(meta = (BindWidget))
	// UTextBlock* Txt_Title;
    
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_Description;

	void UpdateView(URPGQuestData* QuestData);
};
