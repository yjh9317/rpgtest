// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/UI/QuestEntryWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Quest/Data/RPGQuestData.h"
#include "UI/Base/RPGButtonWidget.h"

void UQuestEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Select)
	{
		Btn_Select->OnClicked.AddUniqueDynamic(this, &UQuestEntryWidget::HandleButtonClicked);
	}
}

void UQuestEntryWidget::SetupEntry(URPGQuestData* InQuestData)
{
	MyQuestData = InQuestData;
	if (Txt_QuestTitle)
	{
		Txt_QuestTitle->SetText(MyQuestData ? FText::FromString(MyQuestData->QuestTitle) : FText::GetEmpty());
	}
}

void UQuestEntryWidget::SetIsSelected(bool bSelected)
{
	if (Btn_Select && Btn_Select->Img_Background)
	{
		Btn_Select->Img_Background->SetRenderOpacity(bSelected ? 1.0f : 0.6f);
	}
}

void UQuestEntryWidget::HandleButtonClicked()
{
	if (MyQuestData)
	{
		OnEntryClicked.Broadcast(MyQuestData);
	}
}
