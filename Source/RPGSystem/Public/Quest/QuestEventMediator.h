// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"
#include "QuestEventMediator.generated.h"

class UGlobalEventHandler;
class UQuestObjectiveBase;
class URPGQuest;
/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UQuestEventMediator : public UObject
{
	GENERATED_BODY()
public:
	void Initialize(UGlobalEventHandler* InGlobalEventHandler);
	
	// UFUNCTION()
	// void RegisterListener(IQuestEventListener* Listener);
	// UFUNCTION()
	// void UnregisterListener(IQuestEventListener* Listener);
	// UFUNCTION()
	// void UnregisterAllListenersForQuest(URPGQuest* Quest);

protected:
	/**
	 * GlobalEventHandler로부터 이벤트 수신 (내부 콜백)
	 */
	// UFUNCTION()
	// void HandleGlobalEvent(UObject* Publisher, UObject* Payload, const TArray<FString>& Metadata);

	/**
	 * 태그별로 분류된 리스너 맵
	 * Key: EventTag, Value: 해당 태그를 듣는 리스너들
	 */
	// UPROPERTY()
	// TMap<FGameplayTag, TArray<TObjectPtr<UQuestEventListener>>> ListenersByTag;
	//
	// /**
	//  * GlobalEventHandler 참조 (약한 참조)
	//  */
	// UPROPERTY()
	// TWeakObjectPtr<UGlobalEventHandler> GlobalEventHandler;
	//
	// /**
	//  * 이미 바인딩된 태그들 추적 (중복 바인딩 방지)
	//  */
	// TSet<FGameplayTag> BoundTags;
	//
	// /**
	//  * 특정 태그에 리스너 등록 (내부 함수)
	//  */
	// void BindTagIfNeeded(const FGameplayTag& Tag);
};
