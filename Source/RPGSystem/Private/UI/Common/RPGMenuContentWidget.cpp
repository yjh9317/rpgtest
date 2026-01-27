// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Common/RPGMenuContentWidget.h"

void URPGMenuContentWidget::NativeConstruct()
{
	Super::NativeConstruct();


}

void URPGMenuContentWidget::InitializeMenu(int32 InMyIndex, const TArray<FText>& AllTitles)
{
	MyIndex = InMyIndex;
    

}

void URPGMenuContentWidget::HandleCategoryChanged(int32 NewIndex)
{
	// 헤더에서 버튼을 눌러 인덱스가 바뀌면, 이걸 HUD에게 전파합니다.
	// 여기서 NewIndex는 전체 카테고리 중 몇 번째인지입니다.
	if (NewIndex != MyIndex)
	{
		OnRequestSwitchTab.Broadcast(NewIndex);
	}
}

