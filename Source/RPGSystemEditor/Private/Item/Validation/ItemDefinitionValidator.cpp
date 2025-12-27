// Source/RPGSystemEditor/Private/Item/Validation/ItemDefinitionValidator.cpp
#include "Item/Validation/ItemDefinitionValidator.h"
#include "Item/Validation/ItemDefinitionValidation.h"
#include "Item/Data/ItemDefinition.h"

UItemDefinitionValidator::UItemDefinitionValidator()
{
	bIsEnabled = true;
}

bool UItemDefinitionValidator::CanValidateAsset_Implementation(UObject* InAsset) const
{
	// ItemDefinition만 검증
	return InAsset && InAsset->IsA<UItemDefinition>();
}

EDataValidationResult UItemDefinitionValidator::ValidateLoadedAsset_Implementation(
	UObject* InAsset, 
	TArray<FText>& ValidationErrors)
{
	UItemDefinition* ItemDef = Cast<UItemDefinition>(InAsset);
	if (!ItemDef)
	{
		return EDataValidationResult::NotValidated;
	}
	
	return ValidateItemDefinition(ItemDef, ValidationErrors);
}

EDataValidationResult UItemDefinitionValidator::ValidateItemDefinition(
	UItemDefinition* ItemDef, 
	TArray<FText>& ValidationErrors)
{
	if (!ItemDef)
	{
		return EDataValidationResult::NotValidated;
	}
	
	// ✅ ItemDefinitionValidation 클래스 사용
	FItemDefinitionValidation::FValidationResult Result = 
		FItemDefinitionValidation::ValidateItemTags(ItemDef->ItemTags);
	
	// ========================================
	// 추가 검증
	// ========================================
	
	// ItemName 필수
	if (ItemDef->ItemName.IsEmpty())
	{
		Result.AddError(FText::FromString("Item name is required"));
	}
	
	// Weight 범위
	if (ItemDef->Weight < 0.0f)
	{
		Result.AddError(FText::FromString("Weight cannot be negative"));
	}
	
	// Stackable 검증
	if (ItemDef->bStackable && ItemDef->MaxStackSize < 1)
	{
		Result.AddError(FText::FromString("Stackable items must have MaxStackSize >= 1"));
	}
	
	// Fragment 검증
	if (ItemDef->Fragments.Num() == 0)
	{
		Result.AddWarning(FText::FromString("Item has no fragments - functionality may be limited"));
	}
	
	// ========================================
	// 결과 수집
	// ========================================
	
	ValidationErrors.Append(Result.Errors);
	ValidationErrors.Append(Result.Warnings);
	
	if (!Result.bIsValid)
	{
		return EDataValidationResult::Invalid;
	}
	
	if (Result.Warnings.Num() > 0)
	{
		return EDataValidationResult::Valid;  // Warning은 통과
	}
	
	return EDataValidationResult::Valid;
}