// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Common/RPGMenuContentWidget.h"
#include "QuestWindowWidget.generated.h"

class URPGQuestData;
class UQuestListWidget;
class UQuestDetailPanelWidget;
class UQuestObjectivePanelWidget;
/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UQuestWindowWidget : public URPGMenuContentWidget
{
	GENERATED_BODY()
public:
	// --- 3단 분할 위젯들 ---
	UPROPERTY(meta = (BindWidget))
	UQuestListWidget* WBP_QuestList; 

	UPROPERTY(meta = (BindWidget))
	UQuestDetailPanelWidget* WBP_Description;

	UPROPERTY(meta = (BindWidget))
	UQuestObjectivePanelWidget* WBP_Objectives;
	
protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void HandleQuestSelection(URPGQuestData* SelectedQuest);
};
