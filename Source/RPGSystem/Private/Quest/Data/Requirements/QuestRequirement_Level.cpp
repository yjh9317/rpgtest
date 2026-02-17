#include "Quest/Data/Requirements/QuestRequirement_Level.h"

#include "GameFramework/Pawn.h"
#include "Player/RPGPlayerController.h"
#include "Status/StatsComponent.h"

bool UQuestRequirement_Level::IsMet(const UQuestManagerComponent* Manager, const ARPGPlayerController* Player) const
{
	if (!Player)
	{
		return false;
	}

	const APawn* Pawn = Player->GetPawn();
	if (!Pawn)
	{
		return false;
	}

	const UStatsComponent* Stats = Pawn->FindComponentByClass<UStatsComponent>();
	if (!Stats)
	{
		return false;
	}

	FGameplayTag EffectiveLevelTag = LevelStatTag;
	if (!EffectiveLevelTag.IsValid())
	{
		EffectiveLevelTag = FGameplayTag::RequestGameplayTag(FName("Attribute.Progression.Level"), false);
	}
	if (!EffectiveLevelTag.IsValid())
	{
		EffectiveLevelTag = FGameplayTag::RequestGameplayTag(FName("Attribute.Secondary.Level"), false);
	}

	if (!EffectiveLevelTag.IsValid() || !Stats->HasStat(EffectiveLevelTag))
	{
		return false;
	}

	return Stats->GetStatValue(EffectiveLevelTag) >= RequiredLevel;
}

FText UQuestRequirement_Level::GetFailReason() const
{
	return FText::Format(NSLOCTEXT("Quest", "FailLevel", "플레이어 레벨 {0} 이상이어야 합니다."), RequiredLevel);
}
