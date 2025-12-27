// Source/RPGSystemEditor/Public/Debugger/SStatsDebugger.h
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "GameplayTagContainer.h"

class UStatsComponent;
class SVerticalBox;

class RPGSYSTEMEDITOR_API SStatsDebugger : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SStatsDebugger) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	void CheckSelection();
	void RebuildContent();

	/** 개별 스탯(Tag)에 대한 행을 생성 */
	TSharedRef<SWidget> MakeStatRow(const FGameplayTag& StatTag, float BaseValue, float CurrentValue);

	/** UI 헬퍼 */
	TSharedRef<SWidget> MakeSectionHeader(const FText& HeaderText, FLinearColor Color);

private:
	TSharedPtr<SVerticalBox> ContentBox;
	TWeakObjectPtr<UStatsComponent> CachedStatsComp;
	
	// 변경 감지용 (단순하게 마지막으로 확인한 태그 수 등을 저장하거나, 매 틱 갱신)
	int32 LastStatCount = -1;
};