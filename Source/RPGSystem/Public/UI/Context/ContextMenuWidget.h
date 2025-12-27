// ContextMenuWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ContextMenuDefinitions.h"
#include "ContextMenuWidget.generated.h"

class UVerticalBox;
class UButton;

UCLASS()
class RPGSYSTEM_API UContextMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 메뉴 항목 추가
	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void AddMenuItem(const FContextMenuItemData& ItemData);

	// 화면에 표시 (마우스 위치 기준)
	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void ShowAtMousePosition();

	// 메뉴 닫기
	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void CloseMenu();

protected:
	virtual void NativeConstruct() override;
	
	// 배경 클릭 시 닫기 위한 투명 버튼 핸들러
	UFUNCTION()
	void OnBackgroundClicked();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VB_MenuList;

	// 전체 화면을 덮는 투명 버튼 (외부 클릭 감지용)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_BackgroundClose;

	// 실제 메뉴가 담긴 Border나 CanvasSlot (위치 이동용)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> MenuContainer;

	UPROPERTY(EditDefaultsOnly, Category = "Context Menu")
	TSubclassOf<class UContextMenuEntryWidget> EntryWidgetClass;
};