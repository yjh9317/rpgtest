#include "Quest/World/QuestLocationTrigger.h"

#include "Components/BoxComponent.h"
#include "Event/GlobalEventHandler.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Quest/Components/QuestManagerComponent.h"
#include "RPGSystemGameplayTags.h"

namespace
{
	UQuestManagerComponent* ResolveQuestManager(AActor* SourceActor)
	{
		if (!SourceActor)
		{
			return nullptr;
		}

		if (UQuestManagerComponent* QuestManager = SourceActor->FindComponentByClass<UQuestManagerComponent>())
		{
			return QuestManager;
		}

		if (APawn* Pawn = Cast<APawn>(SourceActor))
		{
			if (AController* Controller = Pawn->GetController())
			{
				if (UQuestManagerComponent* QuestManager = Controller->FindComponentByClass<UQuestManagerComponent>())
				{
					return QuestManager;
				}
			}
		}
		else if (AController* Controller = Cast<AController>(SourceActor))
		{
			if (APawn* ControlledPawn = Controller->GetPawn())
			{
				if (UQuestManagerComponent* QuestManager = ControlledPawn->FindComponentByClass<UQuestManagerComponent>())
				{
					return QuestManager;
				}
			}
		}

		return nullptr;
	}
}

AQuestLocationTrigger::AQuestLocationTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	SetRootComponent(TriggerVolume);

	TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerVolume->SetGenerateOverlapEvents(true);
}

void AQuestLocationTrigger::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerVolume)
	{
		TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AQuestLocationTrigger::HandleOverlap);
	}
}

void AQuestLocationTrigger::HandleOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	const FGameplayTag LocationEventTag = RPGGameplayTags::Event_World_LocationEntered.GetTag();
	if (!LocationTag.IsValid() || !LocationEventTag.IsValid())
	{
		return;
	}

	if (bTriggerOncePerActor && TriggeredActors.Contains(OtherActor))
	{
		return;
	}

	if (!ResolveQuestManager(OtherActor))
	{
		return;
	}

	UGlobalEventHandler* EventHandler = UGlobalEventHandler::Get(this);
	if (!EventHandler)
	{
		return;
	}

	TArray<FString> Metadata;
	Metadata.Add(FString::Printf(TEXT("EventTag=%s"), *LocationEventTag.ToString()));
	Metadata.Add(FString::Printf(TEXT("LocationTag=%s"), *LocationTag.ToString()));
	Metadata.Add(FString::Printf(TEXT("Trigger=%s"), *GetPathName()));

	EventHandler->CallGlobalEventByGameplayTag(this, LocationEventTag, OtherActor, Metadata);
	TriggeredActors.Add(OtherActor);

	if (bDisableAfterFirstValidTrigger && TriggerVolume)
	{
		TriggerVolume->SetGenerateOverlapEvents(false);
		TriggerVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}
