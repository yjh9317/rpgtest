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
class DIALOGUESYSTEMPRESENTATION_API UDialogueReplyWidget : public UUserWidget
{
	GENERATED_BODY()
public:
    /** ë©”ì¸ ë²„íŠ¼ (?„ì²´ ?ì—­) */
    UPROPERTY(meta = (BindWidget))
    class UButton* ReplyButton;
    
    /** ë²„íŠ¼ ?´ë? ì»¨í…Œ?´ë„ˆ */
    UPROPERTY(meta = (BindWidget))
    class UHorizontalBox* ReplyContentBox;
    
    /** ? íƒì§€ ë²ˆí˜¸ */
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* OptionNumberText;
    
    /** ?€???ìŠ¤??*/
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* ReplyText;
    
    /** ì¡°ê±´ ?„ì´ì½?(? íƒ?? */
    UPROPERTY(meta = (BindWidget))
    class UImage* ConditionIcon;
    
    /** ?¤í‚¬ ì²´í¬ ?„ì´ì½?(? íƒ?? BG3 ?¤í??? */
    UPROPERTY(meta = (BindWidget))
    class UImage* SkillCheckIcon;
    
    // ========== ë²„íŠ¼ ?¤í???==========
    
    /** ?¼ë°˜ ?íƒœ ?‰ìƒ */
    UPROPERTY(EditDefaultsOnly, Category = "Style")
    FLinearColor NormalColor = FLinearColor(0.02f, 0.02f, 0.03f, 0.95f);
    
    /** ?¸ë²„ ?íƒœ ?‰ìƒ */
    UPROPERTY(EditDefaultsOnly, Category = "Style")
    FLinearColor HoveredColor = FLinearColor(0.1f, 0.1f, 0.15f, 0.95f);
    
    /** ? íƒ???íƒœ ?‰ìƒ */
    UPROPERTY(EditDefaultsOnly, Category = "Style")
    FLinearColor SelectedColor = FLinearColor(0.15f, 0.15f, 0.2f, 0.95f);
    
    /** ?ìŠ¤???¼ë°˜ ?‰ìƒ */
    UPROPERTY(EditDefaultsOnly, Category = "Style")
    FLinearColor TextNormalColor = FLinearColor(0.8f, 0.8f, 0.8f, 1.0f);
    
    /** ?ìŠ¤???¸ë²„ ?‰ìƒ */
    UPROPERTY(EditDefaultsOnly, Category = "Style")
    FLinearColor TextHoveredColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
    
    // ========== ë©”ì„œ??==========
    
    virtual void NativeConstruct() override;
    
    UFUNCTION()
    void OnReplyClicked();
    
    UFUNCTION()
    void OnReplyHovered();
    
    UFUNCTION()
    void OnReplyUnhovered();
    
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void Setup(const FText& InText, int32 InNodeId, UDialogueUserWidget* InParent, int32 OptionIndex = 0);
    
    /** ë²„íŠ¼ ?¤í????ìš© */
    UFUNCTION(BlueprintCallable, Category = "Style")
    void ApplyButtonStyle(bool bHovered);

protected:
    int32 TargetNodeId;
    UDialogueUserWidget* ParentDialogueWidget;
    bool bIsHovered = false;
};

