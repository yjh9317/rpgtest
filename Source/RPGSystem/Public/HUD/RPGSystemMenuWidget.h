// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MenuEnum.h"
#include "Blueprint/UserWidget.h"
#include "RPGSystemMenuWidget.generated.h"

class UWidgetSwitcher;
class URPGMenuContentWidget;
/**
 * 
 */
UCLASS()
class RPGSYSTEM_API URPGSystemMenuWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;

public:
	// [UI 바인딩] 메뉴들을 교체해서 보여줄 스위처
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> WS_MenuSwitcher;

	// [설정] 각 메뉴 탭의 표시 이름 (순서는 Switcher에 배치된 순서를 따름)
	// 예: Inventory -> "INVENTORY", Quest -> "QUESTS"
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TMap<EMainMenuTab, FText> MenuCategoryTitles;

protected:
	TMap<EMainMenuTab, int32> MenuIndexMap;

public:
	// 외부(PlayerController)에서 특정 탭을 열 때 호출
	UFUNCTION(BlueprintCallable, Category = "Menu Control")
	void OpenMenuTab(EMainMenuTab TabToOpen);

private:
	// 초기화: 자식 위젯들을 스캔하고 헤더 설정을 주입함
	void InitializeMenuPages();

	// 자식(MenuContent)이나 헤더에서 탭 변경 요청이 왔을 때 처리
	UFUNCTION()
	void HandleSwitchRequest(int32 NewIndex);
};
