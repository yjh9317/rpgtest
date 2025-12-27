// Fill out your copyright notice in the Description page of Project Settings.

#include "Quest/Data/Objectives/QuestObjective_Kill.h"
#include "Event/GlobalEventHandler.h" // 글로벌 이벤트 핸들러 가정

void UQuestObjective_Kill::ActivateObjective(URPGQuest* OwnerQuest)
{
	Super::ActivateObjective(OwnerQuest);

	UGlobalEventHandler* Handler = UGlobalEventHandler::Get(this);
	if (Handler)
	{
		// [중요] 부모 클래스의 함수 이름(&UQuestObjectiveBase::OnGlobalEvent)으로 바인딩해도
		// 실제로는 오버라이드된 자식의 함수(Kill::OnGlobalEvent)가 호출됩니다.
		FRPGOnEventCalledSingle Delegate;
		Delegate.BindDynamic(this, &UQuestObjectiveBase::OnGlobalEvent);

		FGameplayTag KillTag = FGameplayTag::RequestGameplayTag(FName("Event.Combat.Kill"));
		Handler->BindGlobalEventByGameplayTag(KillTag, Delegate);
	}
}

void UQuestObjective_Kill::DeactivateObjective()
{
	UWorld* World = GetWorld();
	if (World)
	{
		// UGlobalEventHandler::Get(World)->OnActorKilled.RemoveDynamic(this, &UQuestObjective_Kill::OnEnemyKilled);
	}
	Super::DeactivateObjective();
}


FString UQuestObjective_Kill::GetProgressString() const
{
	return FString::Printf(TEXT("%d / %d"), CurrentAmount, TargetAmount);
}

void UQuestObjective_Kill::OnGlobalEvent(UObject* Publisher, UObject* Payload, const TArray<FString>& Metadata)
{
	Super::OnGlobalEvent(Publisher, Payload, Metadata);
	
	AActor* DeadActor = Cast<AActor>(Payload);
	if (!DeadActor) return;
	
	// if (bIsCompleted) return;
	//
	// // 적 태그가 일치하는지 확인 (자식 태그 포함)
	// if (EnemyTag.MatchesTag(TargetEnemyTag))
	// {
	// 	CurrentAmount += Amount;
	// 	if (OnProgressChanged.IsBound()) OnProgressChanged.Broadcast(this);
	//
	// 	if (CurrentAmount >= TargetAmount)
	// 	{
	// 		CurrentAmount = TargetAmount;
	// 		FinishObjective();
	// 	}
	// }
}
