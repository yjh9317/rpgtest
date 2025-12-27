// Source/RPGSystemEditor/Public/Item/Validation/ItemDefinitionValidation.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * ItemDefinition 검증 유틸리티
 * - Detail Customization과 Data Validation에서 공유하는 검증 로직
 * - 검증 로직을 한 곳에서 중앙 관리
 */
class RPGSYSTEMEDITOR_API FItemDefinitionValidation
{
public:
	// ========================================
	// 검증 결과 구조체
	// ========================================
	
	struct FValidationResult
	{
		bool bIsValid = true;
		TArray<FText> Errors;
		TArray<FText> Warnings;
		
		void AddError(const FText& Error)
		{
			Errors.Add(Error);
			bIsValid = false;
		}
		
		void AddWarning(const FText& Warning)
		{
			Warnings.Add(Warning);
		}
		
		bool HasIssues() const
		{
			return !bIsValid || Warnings.Num() > 0;
		}
	};

	// ========================================
	// Tag 부모 캐싱
	// ========================================
	
	static const FGameplayTag& GetItemTypeParentTag();
	static const FGameplayTag& GetItemRarityParentTag();
	static const FGameplayTag& GetItemSlotParentTag();

	// ========================================
	// 검증 함수
	// ========================================

	/**
	 * Type 태그 검증
	 * @return 유효한 Type 태그 (없으면 Invalid)
	 */
	static FGameplayTag ValidateTypeTag(
		const FGameplayTagContainer& ItemTags, 
		FValidationResult& OutResult
	);

	/**
	 * Rarity 태그 검증
	 * @return 유효한 Rarity 태그 (없으면 Invalid)
	 */
	static FGameplayTag ValidateRarityTag(
		const FGameplayTagContainer& ItemTags, 
		FValidationResult& OutResult
	);

	/**
	 * 전체 태그 검증
	 */
	static FValidationResult ValidateItemTags(const FGameplayTagContainer& ItemTags);

	// ========================================
	// UI 헬퍼
	// ========================================

	/** 사용 가능한 Type 태그 목록 */
	static TArray<FGameplayTag> GetAvailableTypeTags();
	
	/** 사용 가능한 Rarity 태그 목록 */
	static TArray<FGameplayTag> GetAvailableRarityTags();
};