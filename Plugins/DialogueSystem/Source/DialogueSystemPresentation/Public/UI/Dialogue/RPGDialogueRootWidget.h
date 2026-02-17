#pragma once

#include "CoreMinimal.h"
#include "UI/DialogueUserWidget.h"
#include "DialogueRuntimeTypes.h"
#include "RPGDialogueRootWidget.generated.h"

class UImage;
class URichTextBlock;
class UScrollBox;
class UTextBlock;
class UVerticalBox;
class URPGDialogueOptionEntryWidget;

UCLASS()
class DIALOGUESYSTEMPRESENTATION_API URPGDialogueRootWidget : public UDialogueUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void BP_OnDialogueOpened();

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void BP_OnDialogueClosed(bool bCompleted);

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void BP_OnSpeakerChanged(const FDialogueSpeakerPayload& Payload);

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void BP_OnOptionsUpdated(const TArray<FDialogueRuntimeOption>& Options);

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void BP_OnHistoryAdded(const FDialogueHistoryEntryEx& Entry);

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void BP_OnCinematicCue(const FDialogueCinematicCue& Cue);

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Txt_SpeakerName;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<URichTextBlock> RTB_CurrentDialogue;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Img_Portrait;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> VB_Options;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UScrollBox> SB_History;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dialogue")
	TSubclassOf<URPGDialogueOptionEntryWidget> OptionEntryClass;

private:
	bool bBound = false;

	void BindViewModel();
	void UnbindViewModel();

	UFUNCTION()
	void HandleDialogueOpened();

	UFUNCTION()
	void HandleDialogueClosed(bool bCompleted);

	UFUNCTION()
	void HandleSpeakerChanged(const FDialogueSpeakerPayload& Payload);

	UFUNCTION()
	void HandleOptionsUpdated(const TArray<FDialogueRuntimeOption>& Options);

	UFUNCTION()
	void HandleHistoryAdded(const FDialogueHistoryEntryEx& Entry);

	UFUNCTION()
	void HandleCinematicCue(const FDialogueCinematicCue& Cue);
};

