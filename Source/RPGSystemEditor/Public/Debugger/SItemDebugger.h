// Source/RPGSystemEditor/Public/Debugger/SItemDebugger.h
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class AWorldItem;
class UItemInstance;
class UItemFragment;
class SVerticalBox;

/**
 * 에디터에서 선택된 AWorldItem의 상세 정보(Instance, Fragments)를 보여주는 디버거
 */
class RPGSYSTEMEDITOR_API SItemDebugger : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SItemDebugger) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	/** 선택된 액터가 변경되었는지 확인하고 UI를 갱신합니다. */
	void CheckSelectionAndRefresh();

	/** UI 전체를 다시 그립니다. */
	void RebuildContent();

	/** 프래그먼트 섹션을 생성합니다. */
	void AddFragmentSection(UItemFragment* Fragment, TSharedPtr<SVerticalBox> Container);

	/** UI 헬퍼 함수들 */
	TSharedRef<SWidget> MakeSectionHeader(const FText& HeaderText, FLinearColor Color);
	TSharedRef<SWidget> MakeDebugRow(const FText& Label, const FString& ValueStr);

	/** 리플렉션을 사용하여 프로퍼티 값을 문자열로 변환합니다. */
	TSharedRef<SWidget> MakePropertyWidget(FProperty* Prop, UObject* Object);

private:
	/** 아이템 정보가 담길 메인 컨테이너 */
	TSharedPtr<SVerticalBox> ContentBox;

	/** 현재 추적 중인 아이템 (약한 참조) */
	TWeakObjectPtr<AWorldItem> CachedTargetItem;
    
	/** 마지막으로 UI를 갱신했을 때의 ItemInstance ID (변경 감지용) */
	TWeakObjectPtr<UItemInstance> LastInstanceID;
};