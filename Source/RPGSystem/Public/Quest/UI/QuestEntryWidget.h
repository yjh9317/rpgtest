// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QuestEntryWidget.generated.h"

class URPGButtonWidget;
class UImage;
class UTextBlock;
class URPGQuestData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEntrySelected, URPGQuestData*, QuestData);

UCLASS()
class RPGSYSTEM_API UQuestEntryWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URPGButtonWidget> Btn_Select;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_QuestTitle;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_QuestTypeIcon;

	FOnEntrySelected OnEntryClicked;

	UPROPERTY(BlueprintReadOnly, Category = "Data")
	TObjectPtr<URPGQuestData> MyQuestData;

public:
	virtual void NativeConstruct() override;

	void SetupEntry(URPGQuestData* InQuestData);
	void SetIsSelected(bool bSelected);

private:
	UFUNCTION()
	void HandleButtonClicked();
};
