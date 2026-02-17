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
	
	/** ListenerObject는 IQuestEventListener를 구현한 UObject 여야 함 */
	void RegisterListener(UObject* ListenerObject);
	void UnregisterListener(UObject* ListenerObject);
	void UnregisterAllListenersForQuest(URPGQuest* Quest);
	
protected:
	/** GlobalEventHandler로부터 이벤트 수신 (내부 콜백) */
	UFUNCTION()
	void HandleGlobalEvent(UObject* Publisher, UObject* Payload, const TArray<FString>& Metadata);

	/** 특정 태그에 대한 GlobalEvent 바인딩 보장 */
	void BindTagIfNeeded(const FGameplayTag& Tag);
	
private:
	TMap<FGameplayTag, TArray<TWeakObjectPtr<UObject>>> ListenersByTag;
	TWeakObjectPtr<UGlobalEventHandler> GlobalEventHandler;
	TSet<FGameplayTag> BoundTags;
};
