// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/DialogueReplyWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "UI/DialogueUserWidget.h"
#include "UI/DialogueViewModel.h"

void UDialogueReplyWidget::NativeConstruct()
{
	Super::NativeConstruct();
    
	if (ReplyButton)
	{
		// 버튼 이벤트 바인딩
		ReplyButton->OnClicked.AddDynamic(this, &UDialogueReplyWidget::OnReplyClicked);
		ReplyButton->OnHovered.AddDynamic(this, &UDialogueReplyWidget::OnReplyHovered);
		ReplyButton->OnUnhovered.AddDynamic(this, &UDialogueReplyWidget::OnReplyUnhovered);
        
		// 초기 스타일 적용
		ApplyButtonStyle(false);
	}
}


void UDialogueReplyWidget::Setup(const FText& InText, int32 InNodeId, UDialogueUserWidget* InParent, int32 OptionIndex)
{
	TargetNodeId = InNodeId;
	ParentDialogueWidget = InParent;
    
	// 텍스트 설정
	if (ReplyText)
	{
		ReplyText->SetText(InText);
		ReplyText->SetColorAndOpacity(FSlateColor(TextNormalColor));
	}
    
	// 번호 설정
	if (OptionNumberText && OptionIndex > 0)
	{
		OptionNumberText->SetText(FText::AsNumber(OptionIndex));
	}
    
	// 초기 스타일
	ApplyButtonStyle(false);
}

void UDialogueReplyWidget::OnReplyClicked()
{
	if (ParentDialogueWidget && ParentDialogueWidget->ViewModel)
	{
		ParentDialogueWidget->ViewModel->SelectOption(TargetNodeId);
        
		// 클릭 사운드 재생 (선택적)
		// UGameplayStatics::PlaySound2D(this, ClickSound);
	}
}

void UDialogueReplyWidget::OnReplyHovered()
{
	bIsHovered = true;
	ApplyButtonStyle(true);
    
	// 호버 사운드 재생 (선택적)
	// UGameplayStatics::PlaySound2D(this, HoverSound);
}

void UDialogueReplyWidget::OnReplyUnhovered()
{
	bIsHovered = false;
	ApplyButtonStyle(false);
}

void UDialogueReplyWidget::ApplyButtonStyle(bool bHovered)
{
	if (!ReplyButton) return;
    
	// 버튼 배경 색상 변경
	FButtonStyle ButtonStyle = ReplyButton->GetStyle();
    
	if (bHovered)
	{
		ButtonStyle.Normal.TintColor = FSlateColor(HoveredColor);
		ButtonStyle.Hovered.TintColor = FSlateColor(HoveredColor);
        
		// 텍스트 색상 변경
		if (ReplyText)
		{
			ReplyText->SetColorAndOpacity(FSlateColor(TextHoveredColor));
		}
	}
	else
	{
		ButtonStyle.Normal.TintColor = FSlateColor(NormalColor);
		ButtonStyle.Hovered.TintColor = FSlateColor(HoveredColor);
        
		// 텍스트 색상 변경
		if (ReplyText)
		{
			ReplyText->SetColorAndOpacity(FSlateColor(TextNormalColor));
		}
	}
    
	ReplyButton->SetStyle(ButtonStyle);
}
