#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "RPGActorTagComponent.generated.h"

UCLASS(ClassGroup=(Tags), meta=(BlueprintSpawnableComponent))
class RPGSYSTEM_API URPGActorTagComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URPGActorTagComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tags")
	FGameplayTagContainer OwnedTags;

	UFUNCTION(BlueprintCallable, Category = "Tags")
	void AddTag(FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category = "Tags")
	void RemoveTag(FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category = "Tags")
	void ClearTags();

	UFUNCTION(BlueprintPure, Category = "Tags")
	bool HasTag(FGameplayTag Tag) const;

	UFUNCTION(BlueprintPure, Category = "Tags")
	bool HasAllTags(const FGameplayTagContainer& Tags) const;

	UFUNCTION(BlueprintPure, Category = "Tags")
	bool HasAnyTags(const FGameplayTagContainer& Tags) const;

	void GetOwnedTags(FGameplayTagContainer& OutTags) const;
};

