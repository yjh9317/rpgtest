// Source/RPGSystemEditor/Public/Debugger/SQuestDebugger.h
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class UQuestManagerComponent;
class SVerticalBox;

class RPGSYSTEMEDITOR_API SQuestDebugger : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SQuestDebugger) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	void CheckSelection();
	void RebuildContent();

	/** 퀘스트 리스트 표시 (Active / Completed) */
	void DrawQuestList(const FString& Title, const FName& PropertyName, FLinearColor TitleColor);

	TSharedRef<SWidget> MakeSectionHeader(const FText& HeaderText, FLinearColor Color);
	TSharedRef<SWidget> MakeDebugRow(const FText& Label, const FString& ValueStr);

private:
	TSharedPtr<SVerticalBox> ContentBox;
	TWeakObjectPtr<UQuestManagerComponent> CachedQuestComp;
	
	// 변경 감지용 (단순 틱 카운터 혹은 데이터 해시)
	int32 UpdateCounter = 0;
};