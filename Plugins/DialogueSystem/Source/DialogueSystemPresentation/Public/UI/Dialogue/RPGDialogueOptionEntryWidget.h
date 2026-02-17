#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DialogueRuntimeTypes.h"
#include "RPGDialogueOptionEntryWidget.generated.h"

class UButton;
class UImage;
class URichTextBlock;
class UTextBlock;
class UDialogueViewModel;

UCLASS()
class DIALOGUESYSTEMPRESENTATION_API URPGDialogueOptionEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void SetupOption(const FDialogueRuntimeOption& InOption, UDialogueViewModel* InViewModel);

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void BP_ApplyOptionVisual(const FDialogueRuntimeOption& InOption, bool bHovered);

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_Select;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Txt_Index;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<URichTextBlock> RTB_OptionText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Txt_UnavailableReason;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Img_Locked;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Img_SkillCheck;

private:
	UPROPERTY()
	TObjectPtr<UDialogueViewModel> ViewModel;

	FDialogueRuntimeOption RuntimeOption;

	UFUNCTION()
	void OnClicked();

	UFUNCTION()
	void OnHovered();

	UFUNCTION()
	void OnUnhovered();
};

