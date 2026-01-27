// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUD/MenuEnum.h"
#include "RPGMenuContentWidget.generated.h"

class UPlayerInventoryWidget;
class UCategorySelectorWidget;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMenuRequestSwitch, int32, NewIndex);

UCLASS()
class RPGSYSTEM_API URPGMenuContentWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	// 초기화 함수 (HUD가 호출)
	void InitializeMenu(int32 InMyIndex, const TArray<FText>& InCategoryNames);
	EMainMenuTab GetMenuTabType() const { return MenuTabEnum; }
	
public:
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMenuRequestSwitch OnRequestSwitchTab;

	UPROPERTY(EditDefaultsOnly)
	EMainMenuTab MenuTabEnum;
protected:
	int32 MyIndex = 0;

private:
	UFUNCTION()
	void HandleCategoryChanged(int32 NewIndex);
};
