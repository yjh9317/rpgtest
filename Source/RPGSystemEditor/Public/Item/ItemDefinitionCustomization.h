// ItemDefinitionCustomization.h
#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "GameplayTagContainer.h"

class IDetailLayoutBuilder;
class IPropertyHandle;

/**
 * ItemDefinition의 에디터 UI 커스터마이제이션
 * - 필수 태그를 UI에서 강제
 * - 잘못된 입력을 사전에 차단
 * - Undo/Redo 지원
 * - Multi-Edit 지원
 */
class FItemDefinitionCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	// ========================================
	// UI 구성
	// ========================================
	
	void CustomizeTagsCategory(IDetailLayoutBuilder& DetailBuilder);
	
	// ========================================
	// 위젯 생성
	// ========================================
	
	TSharedRef<SWidget> CreateTypeTagWidget(TSharedRef<IPropertyHandle> TagsProperty);
	TSharedRef<SWidget> CreateRarityTagWidget(TSharedRef<IPropertyHandle> TagsProperty);
	
	// ========================================
	// Tag 선택 핸들러
	// ========================================
	
	void OnTypeTagSelected(FGameplayTag SelectedTag, TSharedRef<IPropertyHandle> TagsProperty);
	void OnRarityTagSelected(FGameplayTag SelectedTag, TSharedRef<IPropertyHandle> TagsProperty);
	
	// ========================================
	// 헬퍼 함수: Tag 교체 로직 (중복 제거)
	// ========================================
	
	/**
	 * 특정 카테고리의 기존 태그를 제거하고 새 태그로 교체
	 * - Multi-Edit 지원
	 * - Undo/Redo 지원
	 * 
	 * @param NewTag 새로 설정할 태그
	 * @param TagsProperty ItemTags 프로퍼티 핸들
	 * @param CategoryParentTag 카테고리 부모 태그 (예: Item.Type)
	 * @param TransactionDescription Undo 히스토리에 표시될 설명
	 */
	void ReplaceTagInCategory(
		const FGameplayTag& NewTag,
		TSharedRef<IPropertyHandle> TagsProperty,
		const FGameplayTag& CategoryParentTag,
		const FText& TransactionDescription
	);
	
	// ========================================
	// 현재 선택된 Tag 가져오기
	// ========================================
	
	/**
	 * 특정 카테고리에서 현재 선택된 태그 가져오기
	 * 
	 * @param TagsProperty ItemTags 프로퍼티 핸들
	 * @param CategoryParentTag 카테고리 부모 태그
	 * @return 현재 설정된 태그 (없으면 Invalid Tag)
	 */
	FGameplayTag GetCurrentTagInCategory(
		TSharedRef<IPropertyHandle> TagsProperty,
		const FGameplayTag& CategoryParentTag
	) const;
	
	FGameplayTag GetCurrentTypeTag(TSharedRef<IPropertyHandle> TagsProperty) const;
	FGameplayTag GetCurrentRarityTag(TSharedRef<IPropertyHandle> TagsProperty) const;
	
	// ========================================
	// UI 업데이트
	// ========================================
	
	FText GetTypeTagText(TSharedRef<IPropertyHandle> TagsProperty) const;
	FText GetRarityTagText(TSharedRef<IPropertyHandle> TagsProperty) const;
};