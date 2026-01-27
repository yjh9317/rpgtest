// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Base/RPGButtonWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetBlueprintLibrary.h" // 드래그 취소 등에 유용
#include "Components/SizeBox.h"

void URPGButtonWidget::SetTextLabel()
{
	if (Txt_Label)
	{
		Txt_Label->SetText(ButtonText);
		
		if (ButtonFont.HasValidFont())
		{
			Txt_Label->SetFont(ButtonFont);
		}

		Txt_Label->SetColorAndOpacity(TextColor);
	}
}


void URPGButtonWidget::UpdateWidgetLayout()
{
	SetTextLabel();
	SetButtonIcon(ButtonIcon);
	UpdateButtonStyle();
}

void URPGButtonWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	UpdateWidgetLayout();
}

void URPGButtonWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	UpdateWidgetLayout();
}

void URPGButtonWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	bIsHovered = true;
	UpdateButtonStyle();

	// 호버 사운드 재생
	if (HoverSound)
	{
		UGameplayStatics::PlaySound2D(this, HoverSound);
	}

	if (OnHovered.IsBound())
	{
		OnHovered.Broadcast();
	}
}

void URPGButtonWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	bIsHovered = false;
	
	// 마우스가 밖으로 나가면 눌림 상태도 해제 (클릭 취소)
	if (bIsPressed)
	{
		bIsPressed = false;
	}
	
	UpdateButtonStyle();

	if (OnUnhovered.IsBound())
	{
		OnUnhovered.Broadcast();
	}
}

FReply URPGButtonWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 왼쪽 마우스 버튼만 처리
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsPressed = true;
		UpdateButtonStyle();

		// [핵심] 1. 마우스 캡처 (밖으로 나가도 추적)
		// [핵심] 2. 드래그 감지 시작 (DetectDrag) -> 이게 없으면 OnDragDetected가 안 불림
		return FReply::Handled()
			.CaptureMouse(TakeWidget())
			.DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply URPGButtonWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (bIsPressed)
		{
			bIsPressed = false;
			UpdateButtonStyle();

			// 마우스를 뗄 때, 커서가 여전히 버튼 위에 있는지 확인
			// (드래그 중이 아니어야 클릭으로 인정됨)
			bool bIsCursorInside = InGeometry.IsUnderLocation(InMouseEvent.GetScreenSpacePosition());

			if (bIsCursorInside)
			{
				// 클릭 성공!
				if (ClickSound)
				{
					UGameplayStatics::PlaySound2D(this, ClickSound);
				}

				if (OnClicked.IsBound())
				{
					OnClicked.Broadcast();
				}
			}
			
			// 캡처 해제
			return FReply::Handled().ReleaseMouseCapture();
		}
	}

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void URPGButtonWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	// 드래그가 시작되었으므로 '눌림' 상태는 해제 (단순 클릭이 아님)
	bIsPressed = false;
	UpdateButtonStyle();

	// 블루프린트나 상속받은 클래스에서 정의한 드래그 오퍼레이션 생성
	OutOperation = CreateDragDropOperation();

	// 드래그가 시작되면 보통 위젯은 더 이상 마우스 캡처를 유지할 필요가 없음
	// (DragDropOperation이 제어권을 가져감)
}


void URPGButtonWidget::SetButtonText(FText InText)
{
	ButtonText = InText;
	if (Txt_Label)
	{
		Txt_Label->SetText(ButtonText);
	}
}

void URPGButtonWidget::SetButtonIcon(UTexture2D* InIcon)
{
	if (Img_Icon)
	{
		if (InIcon)
		{
			Img_Icon->SetBrushFromTexture(InIcon);
			Img_Icon->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			Img_Icon->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void URPGButtonWidget::SetBackgroundTexture(UTexture2D* InIcon)
{
	if (Img_Background)
	{
		if (InIcon)
		{
			Img_Icon->SetBrushFromTexture(BackgroundTexture);
			Img_Icon->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			Img_Icon->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void URPGButtonWidget::UpdateButtonStyle()
{
	if (!Img_Background) return;

	FLinearColor TargetColor = NormalColor;

	if (bIsPressed)
	{
		TargetColor = PressedColor;
	}
	else if (bIsHovered)
	{
		TargetColor = HoveredColor;
	}

	Img_Background->SetColorAndOpacity(TargetColor);
}
