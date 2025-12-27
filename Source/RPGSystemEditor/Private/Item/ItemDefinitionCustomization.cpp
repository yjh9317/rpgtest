// ItemDefinitionCustomization.cpp
#include "Item/ItemDefinitionCustomization.h"
#include "Item/Validation/ItemDefinitionValidation.h"
#include "Item/Data/ItemDefinition.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "GameplayTagsManager.h"
#include "ScopedTransaction.h"

TSharedRef<IDetailCustomization> FItemDefinitionCustomization::MakeInstance()
{
	return MakeShareable(new FItemDefinitionCustomization);
}

void FItemDefinitionCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	CustomizeTagsCategory(DetailBuilder);
}

void FItemDefinitionCustomization::CustomizeTagsCategory(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& TagsCategory = DetailBuilder.EditCategory("Tags", 
		FText::FromString("Tags"), ECategoryPriority::Important);
	
	// ItemTags 속성 가져오기
	TSharedRef<IPropertyHandle> TagsProperty = 
		DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UItemDefinition, ItemTags));
	
	// 원본 ItemTags 속성 숨기기
	TagsProperty->MarkHiddenByCustomization();
	
	// ========================================
	// Item Type (필수)
	// ========================================
	
	TagsCategory.AddCustomRow(FText::FromString("Item Type"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(FText::FromString("Item Type"))
			.Font(IDetailLayoutBuilder::GetDetailFontBold())
			.ToolTipText(FText::FromString("Required: Select one item type"))
		]
		.ValueContent()
		[
			CreateTypeTagWidget(TagsProperty)
		];
	
	// ========================================
	// Item Rarity (필수)
	// ========================================
	
	TagsCategory.AddCustomRow(FText::FromString("Item Rarity"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(FText::FromString("Item Rarity"))
			.Font(IDetailLayoutBuilder::GetDetailFontBold())
			.ToolTipText(FText::FromString("Required: Select one item rarity"))
		]
		.ValueContent()
		[
			CreateRarityTagWidget(TagsProperty)
		];
	
	// ========================================
	// Additional Tags (고급)
	// ========================================
	
	TagsCategory.AddProperty(TagsProperty)
		.DisplayName(FText::FromString("Additional Tags"))
		.ToolTip(FText::FromString("Optional: Add extra tags for filtering and gameplay"));
}

TSharedRef<SWidget> FItemDefinitionCustomization::CreateTypeTagWidget(
	TSharedRef<IPropertyHandle> TagsProperty)
{
	return SNew(SComboButton)
		.OnGetMenuContent_Lambda([this, TagsProperty]()
		{
			FMenuBuilder MenuBuilder(true, nullptr);
			
			TArray<FGameplayTag> AvailableTags = 
				FItemDefinitionValidation::GetAvailableTypeTags();
			
			for (const FGameplayTag& Tag : AvailableTags)
			{
				MenuBuilder.AddMenuEntry(
					FText::FromName(Tag.GetTagName()),
					FText::FromString(Tag.ToString()),
					FSlateIcon(),
					FUIAction(
						FExecuteAction::CreateSP(
							this, 
							&FItemDefinitionCustomization::OnTypeTagSelected, 
							Tag, 
							TagsProperty
						)
					)
				);
			}
			
			return MenuBuilder.MakeWidget();
		})
		.ButtonContent()
		[
			SNew(STextBlock)
			.Text_Lambda([this, TagsProperty]()
			{
				return GetTypeTagText(TagsProperty);
			})
		];
}

TSharedRef<SWidget> FItemDefinitionCustomization::CreateRarityTagWidget(
	TSharedRef<IPropertyHandle> TagsProperty)
{
	return SNew(SComboButton)
		.OnGetMenuContent_Lambda([this, TagsProperty]()
		{
			FMenuBuilder MenuBuilder(true, nullptr);
			
			TArray<FGameplayTag> AvailableTags = 
				FItemDefinitionValidation::GetAvailableRarityTags();
			
			for (const FGameplayTag& Tag : AvailableTags)
			{
				MenuBuilder.AddMenuEntry(
					FText::FromName(Tag.GetTagName()),
					FText::FromString(Tag.ToString()),
					FSlateIcon(),
					FUIAction(
						FExecuteAction::CreateSP(
							this, 
							&FItemDefinitionCustomization::OnRarityTagSelected, 
							Tag, 
							TagsProperty
						)
					)
				);
			}
			
			return MenuBuilder.MakeWidget();
		})
		.ButtonContent()
		[
			SNew(STextBlock)
			.Text_Lambda([this, TagsProperty]()
			{
				return GetRarityTagText(TagsProperty);
			})
		];
}

// ========================================
// Tag 선택 핸들러 (공통 로직 사용)
// ========================================

void FItemDefinitionCustomization::OnTypeTagSelected(
	FGameplayTag SelectedTag, 
	TSharedRef<IPropertyHandle> TagsProperty)
{
	ReplaceTagInCategory(
		SelectedTag, 
		TagsProperty, 
		FItemDefinitionValidation::GetItemTypeParentTag(),
		NSLOCTEXT("ItemDefinitionCustomization", "ChangeTypeTag", "아이템 타입 변경")
	);
}

void FItemDefinitionCustomization::OnRarityTagSelected(
	FGameplayTag SelectedTag, 
	TSharedRef<IPropertyHandle> TagsProperty)
{
	ReplaceTagInCategory(
		SelectedTag, 
		TagsProperty, 
		FItemDefinitionValidation::GetItemRarityParentTag(),
		NSLOCTEXT("ItemDefinitionCustomization", "ChangeRarityTag", "아이템 희귀도 변경")
	);
}

// ========================================
// 헬퍼 함수: Tag 교체 로직
// ========================================

void FItemDefinitionCustomization::ReplaceTagInCategory(
	const FGameplayTag& NewTag,
	TSharedRef<IPropertyHandle> TagsProperty,
	const FGameplayTag& CategoryParentTag,
	const FText& TransactionDescription)
{
	// ✅ OuterObjects 올바르게 가져오기
	TArray<UObject*> OuterObjects;
	TagsProperty->GetOuterObjects(OuterObjects);
	
	if (OuterObjects.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("ItemDefinitionCustomization: No outer objects found"));
		return;
	}
	
	// ✅ Undo/Redo 지원을 위한 트랜잭션 시작
	FScopedTransaction Transaction(TransactionDescription);
	
	// ✅ 여러 오브젝트 동시 편집 지원 (Multi-Edit)
	for (UObject* Object : OuterObjects)
	{
		UItemDefinition* ItemDef = Cast<UItemDefinition>(Object);
		if (!ItemDef)
		{
			continue;
		}
		
		// ✅ Undo/Redo를 위해 Modify 호출
		ItemDef->Modify();
		
		// 기존 카테고리 태그 제거
		FGameplayTagContainer CategoryTags = ItemDef->ItemTags.Filter(
			FGameplayTagContainer(CategoryParentTag));
		
		for (const FGameplayTag& OldTag : CategoryTags)
		{
			ItemDef->ItemTags.RemoveTag(OldTag);
		}
		
		// 새 태그 추가
		ItemDef->ItemTags.AddTag(NewTag);
	}
	
	// ✅ UI 업데이트 알림
	TagsProperty->NotifyPostChange(EPropertyChangeType::ValueSet);
}

// ========================================
// 현재 선택된 Tag 가져오기 (공통 로직)
// ========================================

FGameplayTag FItemDefinitionCustomization::GetCurrentTagInCategory(
	TSharedRef<IPropertyHandle> TagsProperty,
	const FGameplayTag& CategoryParentTag) const
{
	// ✅ OuterObjects 올바르게 가져오기
	TArray<UObject*> OuterObjects;
	TagsProperty->GetOuterObjects(OuterObjects);
	
	if (OuterObjects.Num() == 0)
	{
		return FGameplayTag();
	}
	
	// 첫 번째 오브젝트에서 태그 가져오기
	UItemDefinition* ItemDef = Cast<UItemDefinition>(OuterObjects[0]);
	if (!ItemDef)
	{
		return FGameplayTag();
	}
	
	// 해당 카테고리의 태그 필터링
	FGameplayTagContainer CategoryTags = ItemDef->ItemTags.Filter(
		FGameplayTagContainer(CategoryParentTag));
	
	if (CategoryTags.Num() > 0)
	{
		return CategoryTags.First();
	}
	
	return FGameplayTag();
}

FGameplayTag FItemDefinitionCustomization::GetCurrentTypeTag(
	TSharedRef<IPropertyHandle> TagsProperty) const
{
	return GetCurrentTagInCategory(
		TagsProperty, 
		FItemDefinitionValidation::GetItemTypeParentTag()
	);
}

FGameplayTag FItemDefinitionCustomization::GetCurrentRarityTag(
	TSharedRef<IPropertyHandle> TagsProperty) const
{
	return GetCurrentTagInCategory(
		TagsProperty, 
		FItemDefinitionValidation::GetItemRarityParentTag()
	);
}

// ========================================
// UI 텍스트 생성
// ========================================

FText FItemDefinitionCustomization::GetTypeTagText(
	TSharedRef<IPropertyHandle> TagsProperty) const
{
	FGameplayTag CurrentTag = GetCurrentTypeTag(TagsProperty);
	
	if (CurrentTag.IsValid())
	{
		return FText::FromName(CurrentTag.GetTagName());
	}
	
	return FText::FromString("Select Type...");
}

FText FItemDefinitionCustomization::GetRarityTagText(
	TSharedRef<IPropertyHandle> TagsProperty) const
{
	FGameplayTag CurrentTag = GetCurrentRarityTag(TagsProperty);
	
	if (CurrentTag.IsValid())
	{
		return FText::FromName(CurrentTag.GetTagName());
	}
	
	return FText::FromString("Select Rarity...");
}