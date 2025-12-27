// Source/RPGSystemEditor/Public/Debugger/SInventoryDebugger.h
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class UInventoryCoreComponent;
class SVerticalBox;

class RPGSYSTEMEDITOR_API SInventoryDebugger : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SInventoryDebugger) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	void CheckSelection();
	void RebuildContent();
	
	// 개별 슬롯 UI 생성
	TSharedRef<SWidget> MakeSlotRow(int32 SlotIdx, const FString& ItemName, int32 Quantity);

	TSharedRef<SWidget> MakeSectionHeader(const FText& HeaderText, FLinearColor Color);

private:
	TSharedPtr<SVerticalBox> ContentBox;
	TWeakObjectPtr<UInventoryCoreComponent> CachedInvComp;
	int32 UpdateCounter = 0;
};