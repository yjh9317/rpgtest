// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "QuestFunctionLibrary.generated.h"

class URPGQuestData;
class UQuestManagerComponent;
/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UQuestFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// Actor(Player/NPC)로부터 QuestManager를 안전하게 가져오는 함수
	UFUNCTION(BlueprintPure, Category = "Quest | Helper")
	static UQuestManagerComponent* GetQuestManager(AActor* TargetActor);

	// 특정 퀘스트가 완료되었는지 확인 (ID 기반)
	UFUNCTION(BlueprintPure, Category = "Quest | Helper")
	static bool IsQuestCompleted(AActor* TargetActor, int32 QuestID);

	// 특정 퀘스트가 진행 중인지 확인
	UFUNCTION(BlueprintPure, Category = "Quest | Helper")
	static bool IsQuestActive(AActor* TargetActor, int32 QuestID);

	// 퀘스트 수락 가능 여부 (Requirement 클래스들을 순회하며 체크)
	UFUNCTION(BlueprintCallable, Category = "Quest | Helper")
	static bool CanAcceptQuest(AActor* TargetActor, const URPGQuestData* QuestData);
};
