// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Item/Data/Fragment/ItemFragment.h"
#include "ItemFragment_Questable.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UItemFragment_Questable : public UItemFragment
{
	GENERATED_BODY()
public:
	/** * 이 아이템과 관련된 퀘스트 ID 목록 (int32로 변경)
	 *
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest")
	TArray<int32> RelatedQuestIDs; 
    
	/** 퀘스트 아이템은 버릴 수 없음 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest")
	bool bCannotDrop = true;
    
	/** 퀘스트 아이템은 판매 불가 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest")
	bool bCannotSell = true;
    
	/** 자동으로 제거될지 (퀘스트 완료 시) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest")
	bool bAutoRemoveOnQuestComplete = false;

public:
	/** 특정 퀘스트와 관련있는지 확인 (int32로 변경) */
	UFUNCTION(BlueprintPure, Category = "Quest")
	bool IsRelatedToQuest(int32 QuestID) const;
};
