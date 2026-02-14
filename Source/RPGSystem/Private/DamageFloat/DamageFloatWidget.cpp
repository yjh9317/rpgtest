// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageFloat/DamageFloatWidget.h"
#include "Components/TextBlock.h"


void UDamageFloatWidget::SetDamage(float Damage, bool bIsCritical)
{
	FText DamageStr = FText::AsNumber(FMath::RoundToInt(Damage));
	Txt_Damage->SetText(DamageStr);
        
	// 크리티컬이면 색상 변경
	if (bIsCritical)
	{
		Txt_Damage->SetColorAndOpacity(FLinearColor::Yellow);
	}
        
	// 애니메이션 재생
	PlayAnimation(FloatUpAnimation);
}
