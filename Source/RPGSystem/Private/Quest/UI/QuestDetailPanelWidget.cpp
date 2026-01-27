// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/UI/QuestDetailPanelWidget.h"
#include "Components/TextBlock.h"
#include "Quest/Data/RPGQuestData.h"

void UQuestDetailPanelWidget::UpdateView(URPGQuestData* QuestData)
{
	if (QuestData)
	{
		// RPGQuestData.h 참고
		// if (Txt_Title) Txt_Title->SetText(FText::FromString(QuestData->QuestTitle));
		if (Txt_Description) Txt_Description->SetText(FText::FromString(QuestData->QuestDescription));
        
		// 보상(Rewards) 아이콘 생성 로직 등...
	}
}
