// Source/RPGSystemEditor/Public/Item/Validation/ItemDefinitionValidator.h
#pragma once

#include "CoreMinimal.h"
#include "EditorValidatorBase.h"
#include "ItemDefinitionValidator.generated.h"

/**
 * Data Validation 플러그인용 ItemDefinition 검증자
 * - 에디터의 Validate Data 기능과 통합
 * - CI/CD 파이프라인에서 자동 검증 가능
 */
UCLASS()
class RPGSYSTEMEDITOR_API UItemDefinitionValidator : public UEditorValidatorBase
{
	GENERATED_BODY()

public:
	UItemDefinitionValidator();

protected:
	//~UEditorValidatorBase interface
	virtual bool CanValidateAsset_Implementation(UObject* InAsset) const override;
	virtual EDataValidationResult ValidateLoadedAsset_Implementation(UObject* InAsset, TArray<FText>& ValidationErrors) override;
	//~End of UEditorValidatorBase interface

private:
	/** ItemDefinition 검증 로직 */
	EDataValidationResult ValidateItemDefinition(
		class UItemDefinition* ItemDef, 
		TArray<FText>& ValidationErrors
	);
};