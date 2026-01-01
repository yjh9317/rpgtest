// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/DialogueReplyWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UDialogueReplyWidget::Setup(FText InText, int32 InNodeId, UDialogueUserWidget* InParent)
{
	if (ReplyText) ReplyText->SetText(InText);
	TargetNodeId = InNodeId;
	ParentDialogueWidget = InParent;
    
	if (ReplyButton)
	{
		ReplyButton->OnClicked.AddDynamic(this, &UDialogueReplyWidget::OnReplyClicked);
	}
}

void UDialogueReplyWidget::OnReplyClicked()
{
	if (ParentDialogueWidget)
	{
		// 부모 위젯에 구현할 대화 진행 함수 호출 (예: SelectOption)
		// ParentDialogueWidget->SelectOption(TargetNodeId);
	}
}