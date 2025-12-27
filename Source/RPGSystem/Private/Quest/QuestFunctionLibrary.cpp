// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/QuestFunctionLibrary.h"
#include "Quest/Components/QuestManagerComponent.h"
#include "GameFramework/Actor.h"
#include "Player/RPGPlayerController.h"

UQuestManagerComponent* UQuestFunctionLibrary::GetQuestManager(AActor* TargetActor)
{
	if (!IsValid(TargetActor)) return nullptr;
    
	// Actor 본체 혹은 Controller 등을 통해서 컴포넌트 검색
	// (Pawn인 경우 Controller를 통해 찾는 로직 추가 가능)
	return TargetActor->FindComponentByClass<UQuestManagerComponent>();
}

bool UQuestFunctionLibrary::IsQuestCompleted(AActor* TargetActor, int32 QuestID)
{
	if (UQuestManagerComponent* Manager = GetQuestManager(TargetActor))
	{
		return Manager->IsQuestCompleted(QuestID);
	}
	return false;
}

bool UQuestFunctionLibrary::IsQuestActive(AActor* TargetActor, int32 QuestID)
{
	if (UQuestManagerComponent* Manager = GetQuestManager(TargetActor))
	{
		return Manager->IsQuestActive(QuestID);
	}
	return false;
}

bool UQuestFunctionLibrary::CanAcceptQuest(AActor* TargetActor, const URPGQuestData* QuestData)
{
	if (!QuestData) return false;

	// Pawn -> Controller 변환이 필요할 수 있음
	ARPGPlayerController* PC = nullptr;
	if (APawn* Pawn = Cast<APawn>(TargetActor))
	{
		PC = Cast<ARPGPlayerController>(Pawn->GetController());
	}
	else
	{
		PC = Cast<ARPGPlayerController>(TargetActor);
	}

	if (UQuestManagerComponent* Manager = GetQuestManager(TargetActor))
	{
		// Manager에 이전에 만든 CanAcceptQuest 호출
		return Manager->CanAcceptQuest(QuestData, PC);
	}
	return false;
}