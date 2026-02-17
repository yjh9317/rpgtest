#include "Tags/RPGActorTagComponent.h"

URPGActorTagComponent::URPGActorTagComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URPGActorTagComponent::AddTag(FGameplayTag Tag)
{
	if (Tag.IsValid())
	{
		OwnedTags.AddTag(Tag);
	}
}

void URPGActorTagComponent::RemoveTag(FGameplayTag Tag)
{
	if (Tag.IsValid())
	{
		OwnedTags.RemoveTag(Tag);
	}
}

void URPGActorTagComponent::ClearTags()
{
	OwnedTags.Reset();
}

bool URPGActorTagComponent::HasTag(FGameplayTag Tag) const
{
	return Tag.IsValid() && OwnedTags.HasTag(Tag);
}

bool URPGActorTagComponent::HasAllTags(const FGameplayTagContainer& Tags) const
{
	return OwnedTags.HasAll(Tags);
}

bool URPGActorTagComponent::HasAnyTags(const FGameplayTagContainer& Tags) const
{
	return OwnedTags.HasAny(Tags);
}

void URPGActorTagComponent::GetOwnedTags(FGameplayTagContainer& OutTags) const
{
	OutTags = OwnedTags;
}

