// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DialogueReplyWidget.generated.h"

class UDialogueUserWidget;
/**
 * 
 */
UCLASS()
class DIALOGUESYSTEM_API UDialogueReplyWidget : public UUserWidget
{
	GENERATED_BODY()
public:
    /** 메인 버튼 (전체 영역) */
    UPROPERTY(meta = (BindWidget))
    class UButton* ReplyButton;
    
    /** 버튼 내부 컨테이너 */
    UPROPERTY(meta = (BindWidget))
    class UHorizontalBox* ReplyContentBox;
    
    /** 선택지 번호 */
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* OptionNumberText;
    
    /** 대화 텍스트 */
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* ReplyText;
    
    /** 조건 아이콘 (선택적) */
    UPROPERTY(meta = (BindWidget))
    class UImage* ConditionIcon;
    
    /** 스킬 체크 아이콘 (선택적, BG3 스타일) */
    UPROPERTY(meta = (BindWidget))
    class UImage* SkillCheckIcon;
    
    // ========== 버튼 스타일 ==========
    
    /** 일반 상태 색상 */
    UPROPERTY(EditDefaultsOnly, Category = "Style")
    FLinearColor NormalColor = FLinearColor(0.02f, 0.02f, 0.03f, 0.95f);
    
    /** 호버 상태 색상 */
    UPROPERTY(EditDefaultsOnly, Category = "Style")
    FLinearColor HoveredColor = FLinearColor(0.1f, 0.1f, 0.15f, 0.95f);
    
    /** 선택됨 상태 색상 */
    UPROPERTY(EditDefaultsOnly, Category = "Style")
    FLinearColor SelectedColor = FLinearColor(0.15f, 0.15f, 0.2f, 0.95f);
    
    /** 텍스트 일반 색상 */
    UPROPERTY(EditDefaultsOnly, Category = "Style")
    FLinearColor TextNormalColor = FLinearColor(0.8f, 0.8f, 0.8f, 1.0f);
    
    /** 텍스트 호버 색상 */
    UPROPERTY(EditDefaultsOnly, Category = "Style")
    FLinearColor TextHoveredColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
    
    // ========== 메서드 ==========
    
    virtual void NativeConstruct() override;
    
    UFUNCTION()
    void OnReplyClicked();
    
    UFUNCTION()
    void OnReplyHovered();
    
    UFUNCTION()
    void OnReplyUnhovered();
    
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void Setup(const FText& InText, int32 InNodeId, UDialogueUserWidget* InParent, int32 OptionIndex = 0);
    
    /** 버튼 스타일 적용 */
    UFUNCTION(BlueprintCallable, Category = "Style")
    void ApplyButtonStyle(bool bHovered);

protected:
    int32 TargetNodeId;
    UDialogueUserWidget* ParentDialogueWidget;
    bool bIsHovered = false;
};
