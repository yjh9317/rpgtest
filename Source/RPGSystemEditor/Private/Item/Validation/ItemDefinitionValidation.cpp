// Source/RPGSystemEditor/Private/Item/Validation/ItemDefinitionValidation.cpp
#include "Item/Validation/ItemDefinitionValidation.h"
#include "GameplayTagsManager.h"

// ========================================
// Tag 부모 캐싱
// ========================================

const FGameplayTag& FItemDefinitionValidation::GetItemTypeParentTag()
{
	static const FGameplayTag Tag = 
		FGameplayTag::RequestGameplayTag(FName("Item.Type"));
	return Tag;
}

const FGameplayTag& FItemDefinitionValidation::GetItemRarityParentTag()
{
	static const FGameplayTag Tag = 
		FGameplayTag::RequestGameplayTag(FName("Item.Rarity"));
	return Tag;
}

const FGameplayTag& FItemDefinitionValidation::GetItemSlotParentTag()
{
	static const FGameplayTag Tag = 
		FGameplayTag::RequestGameplayTag(FName("Item.Slot"));
	return Tag;
}

// ========================================
// 검증 함수
// ========================================

FGameplayTag FItemDefinitionValidation::ValidateTypeTag(
	const FGameplayTagContainer& ItemTags, 
	FValidationResult& OutResult)
{
	FGameplayTagContainer TypeTags = ItemTags.Filter(
		FGameplayTagContainer(GetItemTypeParentTag())
	);
	
	if (TypeTags.Num() == 0)
	{
		OutResult.AddError(FText::FromString(
			"Item must have exactly one Item.Type.* tag"
		));
		return FGameplayTag();
	}
	
	if (TypeTags.Num() > 1)
	{
		OutResult.AddError(FText::FromString(
			"Item must have only one Item.Type.* tag"
		));
		return FGameplayTag();
	}
	
	return TypeTags.First();
}

FGameplayTag FItemDefinitionValidation::ValidateRarityTag(
	const FGameplayTagContainer& ItemTags, 
	FValidationResult& OutResult)
{
	FGameplayTagContainer RarityTags = ItemTags.Filter(
		FGameplayTagContainer(GetItemRarityParentTag())
	);
	
	if (RarityTags.Num() == 0)
	{
		OutResult.AddError(FText::FromString(
			"Item must have exactly one Item.Rarity.* tag"
		));
		return FGameplayTag();
	}
	
	if (RarityTags.Num() > 1)
	{
		OutResult.AddError(FText::FromString(
			"Item must have only one Item.Rarity.* tag"
		));
		return FGameplayTag();
	}
	
	return RarityTags.First();
}

FItemDefinitionValidation::FValidationResult FItemDefinitionValidation::ValidateItemTags(
	const FGameplayTagContainer& ItemTags)
{
	FValidationResult Result;
	
	// Type 검증
	ValidateTypeTag(ItemTags, Result);
	
	// Rarity 검증
	ValidateRarityTag(ItemTags, Result);
	
	// 추가 검증: Equippable 태그가 있으면 Slot도 있어야 함
	static const FGameplayTag EquippableTag = 
		FGameplayTag::RequestGameplayTag(FName("Item.Equippable"));
	
	if (ItemTags.HasTag(EquippableTag))
	{
		FGameplayTagContainer SlotTags = ItemTags.Filter(
			FGameplayTagContainer(GetItemSlotParentTag())
		);
		
		if (SlotTags.Num() == 0)
		{
			Result.AddWarning(FText::FromString(
				"Equippable items should have an Item.Slot.* tag"
			));
		}
	}
	
	return Result;
}

// ========================================
// UI 헬퍼 (버그 수정)
// ========================================

TArray<FGameplayTag> FItemDefinitionValidation::GetAvailableTypeTags()
{
	TArray<FGameplayTag> Tags;
	
	// ✅ 수정: RequestGameplayTagChildren의 반환값을 사용
	FGameplayTagContainer AllTags = UGameplayTagsManager::Get().RequestGameplayTagChildren(
		GetItemTypeParentTag()
	);
	
	AllTags.GetGameplayTagArray(Tags);
	return Tags;
}

TArray<FGameplayTag> FItemDefinitionValidation::GetAvailableRarityTags()
{
	TArray<FGameplayTag> Tags;
	
	// ✅ 수정: RequestGameplayTagChildren의 반환값을 사용
	FGameplayTagContainer AllTags = UGameplayTagsManager::Get().RequestGameplayTagChildren(
		GetItemRarityParentTag()
	);
	
	AllTags.GetGameplayTagArray(Tags);
	return Tags;
}