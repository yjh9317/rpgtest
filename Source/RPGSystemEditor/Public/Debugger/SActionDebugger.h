// Source/RPGSystemEditor/Public/Debugger/SActionDebugger.h
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "GameplayTagContainer.h"

class UActionComponent;
class UBaseAction;
class SVerticalBox;

/**
 * 선택된 액터의 ActionComponent 상태와 태그 정보를 보여주는 디버거
 */
class RPGSYSTEMEDITOR_API SActionDebugger : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SActionDebugger) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	/** 선택된 액터 및 컴포넌트 갱신 */
	void CheckSelection();

	/** UI 전체 리빌드 */
	void RebuildContent();

	/** 태그 목록 UI 생성 */
	void DrawGameplayTags(const FGameplayTagContainer& Tags, TSharedPtr<SVerticalBox> Container);

	/** 액션 목록 UI 생성 */
	void DrawActionsList(UActionComponent* Comp, TSharedPtr<SVerticalBox> Container);

	/** 개별 액션의 상세 프로퍼티 위젯 생성 (리플렉션) */
	void AddActionDetails(UBaseAction* Action, TSharedPtr<SVerticalBox> Container);

	/** 프로퍼티 값을 위젯으로 변환 (SItemDebugger 로직 재사용) */
	TSharedRef<SWidget> MakePropertyWidget(FProperty* Prop, UObject* Object);

	/** UI 헬퍼 */
	TSharedRef<SWidget> MakeSectionHeader(const FText& HeaderText, FLinearColor Color);
	TSharedRef<SWidget> MakeDebugRow(const FText& Label, const FString& ValueStr);

private:
	TSharedPtr<SVerticalBox> ContentBox;

	/** 현재 추적 중인 컴포넌트 (약한 참조) */
	TWeakObjectPtr<UActionComponent> CachedActionComp;
	
	/** 마지막 업데이트 시점의 태그/액션 수 (변경 감지용) */
	int32 LastTagCount = -1;
	int32 LastActionCount = -1;
};