#include "Quest/Data/Requirements/QuestRequirement_Level.h"
#include "Player/RPGPlayerController.h"
#include "Player/PlayerState/RPGPlayerState.h" 
// 또는 StatsComponent 헤더 포함

bool UQuestRequirement_Level::IsMet(const UQuestManagerComponent* Manager, const ARPGPlayerController* Player) const
{
	if (!Player) return false;

	// 예시: PlayerState를 통해 레벨을 가져오는 경우
	// const ARPGPlayerState* PS = Player->GetPlayerState<ARPGPlayerState>();
	// if (PS) return PS->GetLevel() >= RequiredLevel;

	// 예시: Controller나 Character에 직접 함수가 있는 경우
	// return Player->GetCharacterLevel() >= RequiredLevel;
    
	return true; 
}

FText UQuestRequirement_Level::GetFailReason() const
{
	return FText::Format(NSLOCTEXT("Quest", "FailLevel", "레벨 {0} 이상이어야 합니다."), RequiredLevel);
}