// ContextMenuDefinitions.h
#pragma once

#include "CoreMinimal.h"
#include "ContextMenuDefinitions.generated.h"

// 메뉴 항목 타입
UENUM(BlueprintType)
enum class EContextMenuItemType : uint8
{
	Button      UMETA(DisplayName = "Button"),
	Checkbox    UMETA(DisplayName = "Checkbox"),
	Separator   UMETA(DisplayName = "Separator"),
	Header      UMETA(DisplayName = "Header")
};

// 메뉴 클릭 시 실행할 델리게이트
DECLARE_DYNAMIC_DELEGATE(FOnContextItemClicked);

// 메뉴 항목 데이터 구조체
USTRUCT(BlueprintType)
struct FContextMenuItemData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EContextMenuItemType Type = EContextMenuItemType::Button;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Text;

	// 체크박스일 경우 현재 상태
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsChecked = false;

	// 클릭 시 실행될 함수
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FOnContextItemClicked OnClicked;
};