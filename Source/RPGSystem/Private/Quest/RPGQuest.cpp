// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/RPGQuest.h"
#include "Quest/Data/RPGQuestData.h"
#include "Quest/Data/Objectives/QuestObjectiveBase.h"

URPGQuest::URPGQuest()
{
}

void URPGQuest::InitializeFromData(URPGQuestData* Data)
{
	if (!Data) return;

	QuestData = Data;
	RuntimeObjectives.Empty();

	// [중요] DataAsset에 있는 템플릿 목표들을 복제(Deep Copy)하여 독립적인 인스턴스로 만듦
	for (UQuestObjectiveBase* TemplateObj : QuestData->Objectives)
	{
		if (TemplateObj)
		{
			UQuestObjectiveBase* NewObj = DuplicateObject<UQuestObjectiveBase>(TemplateObj, this);
			RuntimeObjectives.Add(NewObj);
		}
	}
}

void URPGQuest::EnableQuest()
{
	if (QuestState == EQuestState::Active) return;

	UpdateQuestState(EQuestState::Active);

	// 모든 목표 활성화 (이벤트 리스닝 시작)
	for (UQuestObjectiveBase* Obj : RuntimeObjectives)
	{
		if (Obj)
		{
			// 진행도 변화와 완료 이벤트를 구독
			if (!Obj->OnCompleted.IsBound())
			{
				Obj->OnCompleted.AddDynamic(this, &URPGQuest::OnObjectiveCompleted);
			}
			if (!Obj->OnProgressChanged.IsBound())
			{
				Obj->OnProgressChanged.AddDynamic(this, &URPGQuest::OnObjectiveProgress);
			}

			// 목표 가동!
			Obj->ActivateObjective(this);
		}
	}

	// 시작하자마자 완료된 목표가 있는지 체크 (예: 이미 아이템을 가지고 있는 수집 퀘스트)
	if (CheckAllObjectivesCompleted())
	{
		UpdateQuestState(EQuestState::Valid);
	}
}

void URPGQuest::CompleteQuest()
{
	// 목표들 비활성화 (이벤트 리스닝 중지)
	for (UQuestObjectiveBase* Obj : RuntimeObjectives)
	{
		if (Obj) Obj->DeactivateObjective();
	}

	UpdateQuestState(EQuestState::Archive);
}

void URPGQuest::FailQuest()
{
	for (UQuestObjectiveBase* Obj : RuntimeObjectives)
	{
		if (Obj) Obj->DeactivateObjective();
	}

	UpdateQuestState(EQuestState::Failed);
}

void URPGQuest::OnObjectiveProgress(UQuestObjectiveBase* Objective)
{
	// UI 업데이트 등을 위해 전파
	if (OnQuestProgressUpdated.IsBound())
	{
		OnQuestProgressUpdated.Broadcast(this);
	}
}

void URPGQuest::OnObjectiveCompleted(UQuestObjectiveBase* Objective)
{
	// 목표 하나가 완료될 때마다 전체 완료 여부 체크
	if (CheckAllObjectivesCompleted())
	{
		// 모든 목표 달성! -> 보상 받을 준비 완료 (Valid)
		UpdateQuestState(EQuestState::Valid);
	}
	else
	{
		// 아직 남음 -> 업데이트 알림
		OnObjectiveProgress(Objective);
	}
}

bool URPGQuest::CheckAllObjectivesCompleted() const
{
	if (RuntimeObjectives.Num() == 0) return true; // 목표가 없으면 자동 성공? (기획에 따라 다름)

	for (const UQuestObjectiveBase* Obj : RuntimeObjectives)
	{
		// 하나라도 완료되지 않았다면 false
		if (Obj && !Obj->bIsCompleted)
		{
			return false;
		}
	}
	return true;
}

void URPGQuest::UpdateQuestState(EQuestState NewState)
{
	QuestState = NewState;
	if (OnQuestStateChanged.IsBound())
	{
		OnQuestStateChanged.Broadcast(this, QuestState);
	}
}
