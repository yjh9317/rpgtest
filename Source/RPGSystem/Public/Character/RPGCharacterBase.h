#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagAssetInterface.h"
#include "RPGCharacterBase.generated.h"

class URPGActorTagComponent;

UCLASS(Abstract)
class RPGSYSTEM_API ARPGCharacterBase : public ACharacter, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	ARPGCharacterBase();

	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;

	UFUNCTION(BlueprintPure, Category = "Tags")
	URPGActorTagComponent* GetActorTagComponent() const { return ActorTagComponent; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tags")
	TObjectPtr<URPGActorTagComponent> ActorTagComponent;
};

