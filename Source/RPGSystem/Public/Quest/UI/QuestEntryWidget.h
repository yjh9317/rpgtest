// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QuestEntryWidget.generated.h"

class URPGButtonWidget;
class UImage;
class UTextBlock;
class UButton;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEntrySelected, URPGQuestData*, QuestData);

class URPGQuestData;

UCLASS()
class RPGSYSTEM_API UQuestEntryWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	// --- UI 컴포넌트 ---
	// 전체를 감싸는 투명 버튼 (클릭 감지용)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URPGButtonWidget> Btn_Select;

	// 퀘스트 제목
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_QuestTitle;

	// 퀘스트 타입 아이콘 (메인/서브 구분)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_QuestTypeIcon;

	// --- 이벤트 ---	
	FOnEntrySelected OnEntryClicked;

	// --- 데이터 ---
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	TObjectPtr<URPGQuestData> MyQuestData;

public:
	virtual void NativeConstruct() override;

	// 초기화 함수: 데이터를 받아서 UI에 뿌림
	void SetupEntry(URPGQuestData* InQuestData);
	// 선택 상태 시각화 (List 위젯이 호출함)
	void SetIsSelected(bool bSelected);

private:
	// 버튼 클릭 핸들러
	UFUNCTION()
	void HandleButtonClicked();
};
