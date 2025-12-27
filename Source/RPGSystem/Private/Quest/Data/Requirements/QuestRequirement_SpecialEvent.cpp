// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/Data/Requirements/QuestRequirement_SpecialEvent.h"

bool UQuestRequirement_SpecialEvent::IsMet(const UQuestManagerComponent* Manager,
	const ARPGPlayerController* Player) const
{
	if (!Manager) return false;

	// 우선순위 1: 에셋으로 체크
	if (TargetEventAsset)
	{
		// Manager나 Subsystem에 "이 에셋 ID가 기록되었나?" 확인
		// return Manager->HasEventHappened(TargetEventAsset);
            
		// 또는 에셋 내부의 태그를 이용
		if (TargetEventAsset->EventTag.IsValid())
		{
			// return Manager->HasTagEvents(TargetEventAsset->EventTag);
		}
	}

	// 우선순위 2: 태그로 직접 체크
	if (TargetEventTag.IsValid())
	{
		// return Manager->HasTagEvents(TargetEventTag);
	}

	return false;
}
