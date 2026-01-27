// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/UI/QuestListWidget.h"
#include "Components/ScrollBox.h" // 필수!
#include "Quest/UI/QuestEntryWidget.h"

void UQuestListWidget::RefreshList(const TArray<URPGQuestData*>& Quests)
{
	if (!QuestListContainer || !QuestEntryClass) return;

	// 1. 기존 목록 싹 비우기
	QuestListContainer->ClearChildren();

	// 2. 새로운 퀘스트 목록 채우기
	for (URPGQuestData* QuestData : Quests)
	{
		if (QuestData)
		{
			// 위젯 생성
			UUserWidget* NewWidget = CreateWidget<UUserWidget>(this, QuestEntryClass);
            
			// 캐스팅해서 데이터 꽂아주기
			if (UQuestEntryWidget* EntryWidget = Cast<UQuestEntryWidget>(NewWidget))
			{
				// EntryWidget->SetupEntry(QuestData);
				// 항목 클릭 시 내(ListWidget) 이벤트가 발동되도록 연결
				// EntryWidget->OnClicked.AddDynamic(this, ...); 
			}

			// 스크롤 박스에 자식으로 추가
			QuestListContainer->AddChild(NewWidget);
		}
	}
}
