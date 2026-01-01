// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DialogueReplyWidget.generated.h"

/**
 * 
 */
UCLASS()
class DIALOGUESYSTEM_API UDialogueReplyWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
	class UButton* ReplyButton;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ReplyText;

	int32 TargetNodeId;
	class UDialogueUserWidget* ParentDialogueWidget;

	UFUNCTION()
	void OnReplyClicked();

	void Setup(FText InText, int32 InNodeId, UDialogueUserWidget* InParent);
};
