// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QuestListWidget.generated.h"

class UQuestEntryWidget;
class UScrollBox;
class URPGQuestData;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestSelected, URPGQuestData*, SelectedQuest);
/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UQuestListWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> QuestListContainer;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UQuestEntryWidget> QuestEntryClass;

	// 부모가 구독할 이벤트
	FOnQuestSelected OnQuestSelected; 

	// 데이터 받아서 목록 갱신
	void RefreshList(const TArray<URPGQuestData*>& Quests);
};
