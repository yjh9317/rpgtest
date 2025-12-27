// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Data/Fragment/ItemFragment.h"
#include "GameplayTagContainer.h"
#include "ItemFragment_Actions.generated.h"

class UBaseAction;

USTRUCT(BlueprintType)
struct FGrantedActionDef
{
	GENERATED_BODY()

	// 1. 어떤 슬롯으로 사용할 것인가? (예: Action_Primary)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "Action.Slot"))
	FGameplayTag SlotTag;

	// 2. 어떤 액션을 실행할 것인가? (예: UBowAttackAction)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UBaseAction> ActionClass;

	// 3. [확장 예시] 나중에 이런 옵션을 추가하기 쉽습니다!
	// UPROPERTY(EditDefaultsOnly)
	// float OverrideCooldown = -1.0f;
};

UCLASS()
class RPGSYSTEM_API UItemFragment_Actions : public UItemFragment
{
	GENERATED_BODY()
	
public:
	// [설정] 이 아이템을 장착하면 등록될 액션들
	// Key: 슬롯 태그 (예: Action_Primary)
	// Value: 실행할 액션 클래스 (예: UMeleeAttackAction)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions")
	TArray<FGrantedActionDef> GrantedActions;
};
