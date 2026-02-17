#include "Character/RPGCharacterBase.h"

#include "Tags/RPGActorTagComponent.h"

ARPGCharacterBase::ARPGCharacterBase()
{
	ActorTagComponent = CreateDefaultSubobject<URPGActorTagComponent>(TEXT("ActorTagComponent"));
}

void ARPGCharacterBase::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (ActorTagComponent)
	{
		ActorTagComponent->GetOwnedTags(TagContainer);
		return;
	}

	TagContainer.Reset();
}

bool ARPGCharacterBase::HasMatchingGameplayTag(FGameplayTag TagToCheck) const
{
	return ActorTagComponent && ActorTagComponent->HasTag(TagToCheck);
}

bool ARPGCharacterBase::HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (!ActorTagComponent)
	{
		return TagContainer.IsEmpty();
	}

	return ActorTagComponent->HasAllTags(TagContainer);
}

bool ARPGCharacterBase::HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	return ActorTagComponent && ActorTagComponent->HasAnyTags(TagContainer);
}

