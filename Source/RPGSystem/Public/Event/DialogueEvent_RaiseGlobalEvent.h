// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Dialogue.h"
#include "GameplayTagContainer.h"
#include "DialogueEvent_RaiseGlobalEvent.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UDialogueEvent_RaiseGlobalEvent : public UDialogueEvents
{
	GENERATED_BODY()
public:
	// 에디터에서 선택할 이벤트 방식 (이름 또는 태그)
	UPROPERTY(EditAnywhere, Category = "Global Event")
	bool bUseGameplayTag = true;

	// 호출할 게임플레이 태그
	UPROPERTY(EditAnywhere, Category = "Global Event", meta = (EditCondition = "bUseGameplayTag"))
	FGameplayTag EventTag;

	// 호출할 이벤트 이름
	UPROPERTY(EditAnywhere, Category = "Global Event", meta = (EditCondition = "!bUseGameplayTag"))
	FName EventName;

	// 선택 사항: 메타데이터나 추가 페이로드가 필요하다면 변수 추가 가능
	// UPROPERTY(EditAnywhere, Category = "Global Event")
	// TArray<FString> Metadata;

	// UDialogueEvents의 가상 함수 오버라이드
	virtual void RecieveEventTriggered_Implementation(APlayerController* ConsideringPlayer, AActor* NPCActor) override;
};
