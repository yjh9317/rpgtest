#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "QuestLocationTrigger.generated.h"

class UBoxComponent;
class UPrimitiveComponent;
struct FHitResult;

UCLASS(Blueprintable)
class RPGSYSTEM_API AQuestLocationTrigger : public AActor
{
	GENERATED_BODY()

public:
	AQuestLocationTrigger();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
	TObjectPtr<UBoxComponent> TriggerVolume;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FGameplayTag LocationTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	bool bTriggerOncePerActor = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	bool bDisableAfterFirstValidTrigger = false;

private:
	UPROPERTY(Transient)
	TSet<TWeakObjectPtr<AActor>> TriggeredActors;
};
