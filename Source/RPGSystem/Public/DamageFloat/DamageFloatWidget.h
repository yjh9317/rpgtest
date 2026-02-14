// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DamageFloatWidget.generated.h"

class UTextBlock;
class UCanvasPanel;
/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UDamageFloatWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	// UI 바인딩
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_Damage;
    
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* FloatUpAnimation;
    
	// 데미지 설정 함수
	void SetDamage(float Damage, bool bIsCritical);
    
	// 애니메이션 끝나면 호출
	// UFUNCTION()
	// void OnAnimationFinished();
};
