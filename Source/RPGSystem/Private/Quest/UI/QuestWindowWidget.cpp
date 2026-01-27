// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/UI/QuestWindowWidget.h"
#include "Quest/UI/QuestDetailPanelWidget.h"
#include "Quest/UI/QuestListWidget.h"

void UQuestWindowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 1. 리스트 위젯의 이벤트를 구독
	if (WBP_QuestList)
	{
		WBP_QuestList->OnQuestSelected.AddDynamic(this, &UQuestWindowWidget::HandleQuestSelection);
	}
    
	// 2. 초기 데이터 로드 (예시: 플레이어의 퀘스트 매니저에서 가져옴)
	// TArray<URPGQuestData*> MyQuests = Player->GetQuestManager()->GetActiveQuests();
	// WBP_QuestList->RefreshList(MyQuests);
}

void UQuestWindowWidget::HandleQuestSelection(URPGQuestData* SelectedQuest)
{
	if (!SelectedQuest) return;

	// 중간 패널 갱신 (목표)
	if (WBP_Objectives)
	{
		// WBP_Objectives->UpdateObjectives(SelectedQuest->Objectives); // 별도 함수 구현 필요
	}

	// 오른쪽 패널 갱신 (설명)
	if (WBP_Description)
	{
		WBP_Description->UpdateView(SelectedQuest);
	}
}