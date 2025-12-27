// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/Components/QuestManagerComponent.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Player/RPGPlayerController.h"
#include "Quest/RPGQuest.h"
#include "Quest/Data/RPGQuestData.h"
#include "Quest/Data/Requirements/QuestRequirementBase.h"

// Sets default values for this component's properties
UQuestManagerComponent::UQuestManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UQuestManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	GetWorld()->OnWorldBeginPlay.AddUObject(this, &UQuestManagerComponent::LateBeginPlay);
}

void UQuestManagerComponent::LateBeginPlay()
{
	// /* Get the Player Channels component */
	// UAQ_PlayerChannels* playerChannels = GetOwner()->FindComponentByClass<UAQ_PlayerChannels>();
	// if (!playerChannels)
	// 	return;
	//
	// /* Generate an array with all the quests in the QuestDataCenter */
	// TArray<URPGQuest*> temporaryQuests;
	// QuestDataCenter.GenerateValueArray(temporaryQuests);
	//
	// /* Call On Quest Created for each quests */
	// for (auto quests : temporaryQuests)
	// {
	// 	/* According to the state of each quests, the Player Channels will update
	// 	the Quest channel and its book quest, and Functions will be bind to delegates */
	// 	playerChannels->OnQuestCreated(quests);
	// }
}

void UQuestManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();
	
	/* Asset Registry 로직 (주석 처리된 부분 해제 및 경로 수정 필요) */
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FAssetData> QuestDataAssets;
    
	FTopLevelAssetPath assetPath = FTopLevelAssetPath(TEXT("/Script/RPGSystem"), TEXT("RPGQuestData"));
	AssetRegistry.GetAssetsByClass(assetPath, QuestDataAssets, true);

	for (const FAssetData& Asset : QuestDataAssets)
	{
		URPGQuestData* DataAsset = Cast<URPGQuestData>(Asset.GetAsset());
	
		if (!DataAsset) continue;
		
		URPGQuest* NewQuestInstance = NewObject<URPGQuest>(this, URPGQuest::StaticClass());
		NewQuestInstance->InitializeFromData(DataAsset); 
		// NewQuestInstance->QuestData = DataAsset; 
		
		int32 QuestID = DataAsset->QuestID;
		QuestDataCenter.Add(QuestID, NewQuestInstance);
	}
}


URPGQuest* UQuestManagerComponent::QueryQuest(int QuestID)
{
	/* Find the Quest corresponding to the ID */
	if (QuestDataCenter.Find(QuestID))
		return QuestDataCenter[QuestID];

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

bool UQuestManagerComponent::CanAcceptQuest(const URPGQuestData* QuestData, class ARPGPlayerController* Player)
{
	if (!QuestData || !Player) return false;

	// 이미 퀘스트를 가지고 있거나 완료했는지 체크 (기존 로직 활용)
	if (IsQuestActive(QuestData->QuestID) || IsQuestCompleted(QuestData->QuestID)) return false;

	// Requirements 배열 순회
	for (const UQuestRequirementBase* Req : QuestData->Requirements)
	{
		if (Req && !Req->IsMet(this, Player))
		{
			return false;
		}
	}

	return true;
}

const URPGQuestData* UQuestManagerComponent::GetQuestDataByID(int32 QuestID) const
{
	if (const URPGQuest* QuestInstance = QuestDataCenter.FindRef(QuestID))
	{
		return QuestInstance->QuestData;
	}
    
	// 만약 인스턴스가 없다면 nullptr 반환
	return nullptr;
}



