// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/Components/QuestManagerComponent.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Event/GlobalEventHandler.h"
#include "Event/RPGEventBase.h"
#include "GameFramework/Pawn.h"
#include "Player/RPGPlayerController.h"
#include "Quest/RPGQuest.h"
#include "Quest/QuestEventMediator.h"
#include "Quest/Data/RPGQuestData.h"
#include "Quest/Data/QuestSpecialEventData.h"
#include "Quest/Data/Objectives/QuestObjectiveBase.h"
#include "Quest/Data/Requirements/QuestRequirementBase.h"
#include "Quest/Data/Requirements/QuestRequirement_SpecialEvent.h"

// Sets default values for this component's properties
UQuestManagerComponent::UQuestManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UQuestManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->OnWorldBeginPlay.AddUObject(this, &UQuestManagerComponent::LateBeginPlay);

	if (UGlobalEventHandler* EventHandler = UGlobalEventHandler::Get(this))
	{
		QuestEventMediator = NewObject<UQuestEventMediator>(this);
		QuestEventMediator->Initialize(EventHandler);
	}

	InitializeSpecialEventTracking();
}

void UQuestManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGlobalEventHandler* EventHandler = UGlobalEventHandler::Get(this))
	{
		for (const FGameplayTag& TrackedTag : TrackedSpecialEventTags)
		{
			EventHandler->UnbindAllGlobalEventsByGameplayTag(this, TrackedTag);
		}
	}
	TrackedSpecialEventTags.Reset();

	Super::EndPlay(EndPlayReason);
}

void UQuestManagerComponent::LateBeginPlay()
{
	// Reserved for runtime hookup after world begin.
}

void UQuestManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FAssetData> QuestDataAssets;

	const FTopLevelAssetPath AssetPath = FTopLevelAssetPath(TEXT("/Script/RPGSystem"), TEXT("RPGQuestData"));
	AssetRegistry.GetAssetsByClass(AssetPath, QuestDataAssets, true);

	for (const FAssetData& Asset : QuestDataAssets)
	{
		URPGQuestData* DataAsset = Cast<URPGQuestData>(Asset.GetAsset());
		if (!DataAsset)
		{
			continue;
		}

		URPGQuest* NewQuestInstance = NewObject<URPGQuest>(this, URPGQuest::StaticClass());
		NewQuestInstance->InitializeFromData(DataAsset);

		const int32 QuestID = DataAsset->QuestID;
		QuestDataCenter.Add(QuestID, NewQuestInstance);
	}
}

URPGQuest* UQuestManagerComponent::QueryQuest(int QuestID)
{
	if (QuestDataCenter.Find(QuestID))
	{
		return QuestDataCenter[QuestID];
	}

	return nullptr;
}

bool UQuestManagerComponent::IsQuestCompleted(int32 QuestID) const
{
	if (const URPGQuest* Quest = QuestDataCenter.FindRef(QuestID))
	{
		return Quest->QuestState == EQuestState::Archive;
	}
	return false;
}

bool UQuestManagerComponent::IsQuestActive(int32 QuestID) const
{
	if (const URPGQuest* Quest = QuestDataCenter.FindRef(QuestID))
	{
		return Quest->QuestState == EQuestState::Active;
	}
	return false;
}

bool UQuestManagerComponent::CanAcceptQuest(const URPGQuestData* QuestData, ARPGPlayerController* Player)
{
	if (!QuestData)
	{
		return false;
	}

	ARPGPlayerController* EffectivePlayer = Player;
	if (!EffectivePlayer)
	{
		if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
		{
			EffectivePlayer = Cast<ARPGPlayerController>(OwnerPawn->GetController());
		}
	}
	if (!EffectivePlayer)
	{
		return false;
	}

	if (IsQuestActive(QuestData->QuestID) || IsQuestCompleted(QuestData->QuestID))
	{
		return false;
	}

	for (const UQuestRequirementBase* Req : QuestData->Requirements)
	{
		if (Req && !Req->IsMet(this, EffectivePlayer))
		{
			return false;
		}
	}

	return true;
}

bool UQuestManagerComponent::AcceptQuestByID(int32 QuestID, ARPGPlayerController* Player)
{
	URPGQuest* Quest = QueryQuest(QuestID);
	if (!Quest || !Quest->QuestData)
	{
		return false;
	}

	if (Quest->QuestState == EQuestState::Active || Quest->QuestState == EQuestState::Valid || Quest->QuestState == EQuestState::Archive)
	{
		return false;
	}

	if (!CanAcceptQuest(Quest->QuestData, Player))
	{
		return false;
	}

	Quest->EnableQuest();
	RegisterQuestObjectives(Quest);
	OnQuestAccepted.Broadcast(Quest);
	return true;
}

bool UQuestManagerComponent::CompleteQuestByID(int32 QuestID)
{
	URPGQuest* Quest = QueryQuest(QuestID);
	if (!Quest || Quest->QuestState != EQuestState::Valid)
	{
		return false;
	}

	UnregisterQuestObjectives(Quest);
	Quest->CompleteQuest();
	OnQuestCompleted.Broadcast(Quest);
	return true;
}

bool UQuestManagerComponent::FailQuestByID(int32 QuestID)
{
	URPGQuest* Quest = QueryQuest(QuestID);
	if (!Quest || (Quest->QuestState != EQuestState::Active && Quest->QuestState != EQuestState::Valid))
	{
		return false;
	}

	UnregisterQuestObjectives(Quest);
	Quest->FailQuest();
	OnQuestFailed.Broadcast(Quest);
	return true;
}

void UQuestManagerComponent::GetAllQuests(TArray<URPGQuest*>& OutQuests) const
{
	QuestDataCenter.GenerateValueArray(OutQuests);
}

void UQuestManagerComponent::RecordQuestEventTag(FGameplayTag EventTag)
{
	if (EventTag.IsValid())
	{
		TriggeredEventTags.Add(EventTag);
	}
}

void UQuestManagerComponent::RecordQuestSpecialEvent(const UQuestSpecialEventData* EventData)
{
	if (!IsValid(EventData))
	{
		return;
	}

	TriggeredSpecialEventAssets.Add(FName(*EventData->GetPathName()));
	RecordQuestEventTag(EventData->EventTag);
}

bool UQuestManagerComponent::HasEventTagOccurred(FGameplayTag EventTag) const
{
	return EventTag.IsValid() && TriggeredEventTags.Contains(EventTag);
}

bool UQuestManagerComponent::HasSpecialEventOccurred(const UQuestSpecialEventData* EventData) const
{
	if (!IsValid(EventData))
	{
		return false;
	}

	if (TriggeredSpecialEventAssets.Contains(FName(*EventData->GetPathName())))
	{
		return true;
	}

	return EventData->EventTag.IsValid() && TriggeredEventTags.Contains(EventData->EventTag);
}

const URPGQuestData* UQuestManagerComponent::GetQuestDataByID(int32 QuestID) const
{
	if (const URPGQuest* QuestInstance = QuestDataCenter.FindRef(QuestID))
	{
		return QuestInstance->QuestData;
	}

	return nullptr;
}

void UQuestManagerComponent::RegisterQuestObjectives(URPGQuest* Quest)
{
	if (!QuestEventMediator || !Quest)
	{
		return;
	}

	for (UQuestObjectiveBase* Objective : Quest->RuntimeObjectives)
	{
		QuestEventMediator->RegisterListener(Objective);
	}
}

void UQuestManagerComponent::UnregisterQuestObjectives(URPGQuest* Quest)
{
	if (QuestEventMediator && Quest)
	{
		QuestEventMediator->UnregisterAllListenersForQuest(Quest);
	}
}

void UQuestManagerComponent::InitializeSpecialEventTracking()
{
	UGlobalEventHandler* EventHandler = UGlobalEventHandler::Get(this);
	if (!EventHandler)
	{
		return;
	}

	TrackedSpecialEventTags.Reset();
	for (const TPair<int, URPGQuest*>& Pair : QuestDataCenter)
	{
		const URPGQuest* Quest = Pair.Value;
		if (!Quest || !Quest->QuestData)
		{
			continue;
		}

		for (const UQuestRequirementBase* Requirement : Quest->QuestData->Requirements)
		{
			const UQuestRequirement_SpecialEvent* SpecialRequirement = Cast<UQuestRequirement_SpecialEvent>(Requirement);
			if (!SpecialRequirement)
			{
				continue;
			}

			if (SpecialRequirement->TargetEventTag.IsValid())
			{
				TrackedSpecialEventTags.Add(SpecialRequirement->TargetEventTag);
			}

			if (SpecialRequirement->TargetEventAsset && SpecialRequirement->TargetEventAsset->EventTag.IsValid())
			{
				TrackedSpecialEventTags.Add(SpecialRequirement->TargetEventAsset->EventTag);
			}
		}
	}

	for (const FGameplayTag& TrackedTag : TrackedSpecialEventTags)
	{
		FRPGOnEventCalledSingle Delegate;
		Delegate.BindDynamic(this, &UQuestManagerComponent::HandleTrackedSpecialEvent);
		EventHandler->BindGlobalEventByGameplayTag(TrackedTag, Delegate);
	}
}

void UQuestManagerComponent::HandleTrackedSpecialEvent(UObject* Publisher, UObject* Payload, const TArray<FString>& Metadata)
{
	FGameplayTag EventTag;
	if (const URPGEventBase* EventBase = Cast<URPGEventBase>(Publisher))
	{
		EventTag = EventBase->EventTag;
	}

	if (!EventTag.IsValid())
	{
		for (const FString& Entry : Metadata)
		{
			if (Entry.StartsWith(TEXT("EventTag=")))
			{
				const FString TagString = Entry.RightChop(9);
				EventTag = FGameplayTag::RequestGameplayTag(FName(*TagString), false);
				break;
			}
		}
	}

	RecordQuestEventTag(EventTag);

	for (const FString& Entry : Metadata)
	{
		if (Entry.StartsWith(TEXT("EventAsset=")))
		{
			TriggeredSpecialEventAssets.Add(FName(*Entry.RightChop(11)));
		}
	}
}
