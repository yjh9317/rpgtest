// ItemDefinition.cpp
#include "Item/Data/ItemDefinition.h"
#include "Item/Data/ItemInstance.h"
#include "Item/Data/Fragment/ItemFragment.h"
#include "UObject/ObjectSaveContext.h"

UItemDefinition::UItemDefinition()
{
}

UItemInstance* UItemDefinition::CreateInstance(UObject* Outer) const
{
	if (!Outer)
	{
		return nullptr;
	}
	
	// 1. 새 Instance 생성
	UItemInstance* NewInstance = NewObject<UItemInstance>(Outer);
	
	// 2. Definition 참조 설정 (asset 자체를 참조)
	NewInstance->ItemDef = this;
	
	// 3. Fragment들을 복제하여 Instance에 추가
	for (UItemFragment* FragmentTemplate : Fragments)
	{
		if (FragmentTemplate)
		{
			// DuplicateObject로 Fragment 복제 (Outer는 NewInstance)
			UItemFragment* InstancedFragment = DuplicateObject(FragmentTemplate, NewInstance);
			
			// 복제된 Fragment 초기화 (런타임 변수 설정)
			InstancedFragment->OnInstanced(NewInstance);
			
			// Instance의 Fragment 리스트에 추가
			NewInstance->AddFragment(InstancedFragment);
		}
	}
	
	return NewInstance;
}

#if WITH_EDITOR

void UItemDefinition::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	// ItemTags가 변경되면 자동 검증
	if (PropertyChangedEvent.Property && 
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UItemDefinition, ItemTags))
	{
		ValidateInternal(false);  // 조용히 검증 (에러만 로그)
	}
}

void UItemDefinition::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);
	
	// 저장 전 검증
	ValidateInternal(false);
}

void UItemDefinition::ValidateItemDefinition()
{
	// 수동 검증 (결과 표시)
	ValidateInternal(true);
}

bool UItemDefinition::ValidateInternal(bool bShowNotification)
{
	// 에디터 모듈의 검증 클래스 사용
	// 주의: 런타임 모듈에서 에디터 모듈 의존성을 직접 가질 수 없으므로
	// 여기서는 기본적인 검증만 수행하거나, 
	// Data Validation 플러그인을 사용하는 것이 좋습니다
	
	bool bIsValid = true;
	TArray<FText> Errors;
	TArray<FText> Warnings;
	
	// ========================================
	// 기본 검증
	// ========================================
	
	// ItemName 필수
	if (ItemName.IsEmpty())
	{
		Errors.Add(FText::FromString("Item name is required"));
		bIsValid = false;
	}
	
	// Weight 범위 검증
	if (Weight < 0.0f)
	{
		Errors.Add(FText::FromString("Weight cannot be negative"));
		bIsValid = false;
	}
	
	// Stackable 검증
	if (bStackable && MaxStackSize < 1)
	{
		Errors.Add(FText::FromString("Stackable items must have MaxStackSize >= 1"));
		bIsValid = false;
	}
	
	// ========================================
	// Tag 검증 (간단 버전)
	// ========================================
	
	// Type 태그 확인
	static const FGameplayTag TypeParentTag = 
		FGameplayTag::RequestGameplayTag(FName("Item.Type"));
	
	FGameplayTagContainer TypeTags = ItemTags.Filter(
		FGameplayTagContainer(TypeParentTag)
	);
	
	// if (TypeTags.Num() == 0)
	// {
	// 	Errors.Add(FText::FromString("Item must have exactly one Item.Type.* tag"));
	// 	bIsValid = false;
	// }
	// else if (TypeTags.Num() > 1)
	// {
	// 	Errors.Add(FText::FromString("Item must have only one Item.Type.* tag"));
	// 	bIsValid = false;
	// }
	
	// Rarity 태그 확인
	static const FGameplayTag RarityParentTag = 
		FGameplayTag::RequestGameplayTag(FName("Item.Rarity"));
	
	FGameplayTagContainer RarityTags = ItemTags.Filter(
		FGameplayTagContainer(RarityParentTag)
	);
	
	// if (RarityTags.Num() == 0)
	// {
	// 	Errors.Add(FText::FromString("Item must have exactly one Item.Rarity.* tag"));
	// 	bIsValid = false;
	// }
	// else if (RarityTags.Num() > 1)
	// {
	// 	Errors.Add(FText::FromString("Item must have only one Item.Rarity.* tag"));
	// 	bIsValid = false;
	// }
	
	// ========================================
	// 결과 표시
	// ========================================
	
	FMessageLog DataValidationLog("AssetCheck");
	
	for (const FText& Error : Errors)
	{
		DataValidationLog.Error()
			->AddToken(FTextToken::Create(FText::FromString(GetName())))
			->AddToken(FTextToken::Create(FText::FromString(": ")))
			->AddToken(FTextToken::Create(Error));
	}
	
	for (const FText& Warning : Warnings)
	{
		DataValidationLog.Warning()
			->AddToken(FTextToken::Create(FText::FromString(GetName())))
			->AddToken(FTextToken::Create(FText::FromString(": ")))
			->AddToken(FTextToken::Create(Warning));
	}
	
	if (bShowNotification)
	{
		if (bIsValid && Warnings.Num() == 0)
		{
			FMessageDialog::Open(EAppMsgType::Ok, 
				FText::FromString("Validation passed! No issues found."));
		}
		else
		{
			FString Message = "Validation completed with issues:\n\n";
			for (const FText& Error : Errors)
			{
				Message += "ERROR: " + Error.ToString() + "\n";
			}
			for (const FText& Warning : Warnings)
			{
				Message += "WARNING: " + Warning.ToString() + "\n";
			}
			
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Message));
		}
		
		DataValidationLog.Notify();
	}
	
	return bIsValid;
}

#endif // WITH_EDITOR