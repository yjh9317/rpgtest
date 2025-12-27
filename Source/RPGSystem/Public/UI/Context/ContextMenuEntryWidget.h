// ContextMenuEntryWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ContextMenuDefinitions.h"
#include "ContextMenuEntryWidget.generated.h"

class UTextBlock;
class UButton;
class UCheckBox;
class UWidgetSwitcher;

UCLASS()
class RPGSYSTEM_API UContextMenuEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetupEntry(const FContextMenuItemData& InData);

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnBtnClicked();

	UFUNCTION()
	void OnCheckboxChanged(bool bNewState);

protected:
	FContextMenuItemData ItemData;

	// --- Bind Widgets ---
	// 타입을 전환하기 위한 스위처 (Button / Checkbox / Separator / Header)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> Switcher_Type;

	// 1. Button Type
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Action;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_ButtonLabel;

	// 2. Checkbox Type
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> CB_Toggle;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_CheckboxLabel;

	// 3. Header Type
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_HeaderLabel;

	// 부모 메뉴 위젯 (클릭 후 닫기 위해 필요)
	TWeakObjectPtr<class UContextMenuWidget> ParentMenu;
	
	friend class UContextMenuWidget;
};