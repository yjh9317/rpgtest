// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/RPGSystemMenuWidget.h"

#include "Components/WidgetSwitcher.h"
#include "UI/Common/RPGMenuContentWidget.h"

void URPGSystemMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	InitializeMenuPages();
}

void URPGSystemMenuWidget::InitializeMenuPages()
{
	if (!WS_MenuSwitcher) return;

	MenuIndexMap.Empty();
	TArray<FText> AllCategoryTitles;

	// 1. Switcher의 자식 개수 파악
	int32 ChildCount = WS_MenuSwitcher->GetChildrenCount();

	// 2. 자식들을 순회하며 데이터 구축
	for (int32 i = 0; i < ChildCount; ++i)
	{
		// 모든 메뉴 윈도우(인벤토리 등)는 URPGMenuContentWidget을 상속받아야 함
		URPGMenuContentWidget* MenuContent = Cast<URPGMenuContentWidget>(WS_MenuSwitcher->GetWidgetAtIndex(i));
		
		if (MenuContent)
		{
			// 자식에게 자신의 정체성(Enum)을 물어봄
			EMainMenuTab MenuType = MenuContent->GetMenuTabType();

			// 매핑 저장 (Enum -> Index)
			MenuIndexMap.Add(MenuType, i);

			// 헤더에 표시할 타이틀 텍스트 찾기
			if (MenuCategoryTitles.Contains(MenuType))
			{
				AllCategoryTitles.Add(MenuCategoryTitles[MenuType]);
			}
			else
			{
				// 이름이 없으면 Enum 이름을 대충 변환해서라도 넣음 (디버깅용)
				FString EnumName = UEnum::GetValueAsString(MenuType);
				AllCategoryTitles.Add(FText::FromString(EnumName));
			}
		}
		else
		{
			// RPGMenuContentWidget이 아닌 자식이 있다면 빈 텍스트 처리
			AllCategoryTitles.Add(FText::GetEmpty());
		}
	}

	// 3. 다시 순회하며 자식들에게 초기화 명령 전달
	for (int32 i = 0; i < ChildCount; ++i)
	{
		URPGMenuContentWidget* MenuContent = Cast<URPGMenuContentWidget>(WS_MenuSwitcher->GetWidgetAtIndex(i));
		if (MenuContent)
		{
			// "너는 i번째 탭이고, 전체 탭 목록은 이거야"라고 알려줌
			MenuContent->InitializeMenu(i, AllCategoryTitles);

			// 자식이 "탭 바꿔줘"라고 요청하면 HandleSwitchRequest 실행
			MenuContent->OnRequestSwitchTab.RemoveDynamic(this, &URPGSystemMenuWidget::HandleSwitchRequest);
			MenuContent->OnRequestSwitchTab.AddDynamic(this, &URPGSystemMenuWidget::HandleSwitchRequest);
		}
	}
}

void URPGSystemMenuWidget::OpenMenuTab(EMainMenuTab TabToOpen)
{
	if (!WS_MenuSwitcher) return;

	if (MenuIndexMap.Contains(TabToOpen))
	{
		int32 TargetIndex = MenuIndexMap[TabToOpen];
		HandleSwitchRequest(TargetIndex);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("RPGSystemMenu: Requested tab not found in switcher!"));
	}
}

void URPGSystemMenuWidget::HandleSwitchRequest(int32 NewIndex)
{
	if (WS_MenuSwitcher)
	{
		// 1. 실제 페이지 전환
		WS_MenuSwitcher->SetActiveWidgetIndex(NewIndex);

		// 2. 바뀐 페이지의 헤더(Selector)도 동기화 (선택 사항)
		// URPGMenuContentWidget::InitializeMenu에서 이미 자신의 인덱스를 설정했지만,
		// 메뉴를 껐다 켰을 때나 확실한 동기화를 위해 현재 활성 위젯에게 "너 활성화됐어"라고 알릴 수도 있음.
		if (auto* ActiveMenu = Cast<URPGMenuContentWidget>(WS_MenuSwitcher->GetWidgetAtIndex(NewIndex)))
		{
			// 필요하다면 Refresh 함수 호출
		}
	}
}