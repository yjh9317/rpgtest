#include "NPC/NPCCharacter.h"

#include "Component/DialogueManagerComponent.h"
#include "Component/DialogueProviderComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Pawn.h"
#include "Interaction/InteractableComponent.h"
#include "Player/RPGPlayerController.h"
#include "Quest/Components/QuestGiverComponent.h"
#include "RPGSystemCollisionChannels.h"

ANPCCharacter::ANPCCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	InteractionCollision = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionCollision"));
	InteractionCollision->SetupAttachment(GetRootComponent());
	InteractionCollision->SetSphereRadius(160.0f);
	InteractionCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionCollision->SetCollisionObjectType(ECO_Interactable);
	InteractionCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	InteractionWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionWidget"));
	InteractionWidget->SetupAttachment(GetRootComponent());
	InteractionWidget->SetWidgetSpace(EWidgetSpace::Screen);
	InteractionWidget->SetDrawAtDesiredSize(true);
	InteractionWidget->SetVisibility(false);

	InteractableComponent = CreateDefaultSubobject<UInteractableComponent>(TEXT("InteractableComponent"));
	QuestGiverComponent = CreateDefaultSubobject<UQuestGiverComponent>(TEXT("QuestGiverComponent"));
	DialogueProviderComponent = CreateDefaultSubobject<UDialogueProviderComponent>(TEXT("DialogueProviderComponent"));

	if (InteractableComponent)
	{
		InteractableComponent->CustomDepthStencilValue = StencilValue;
	}

	if (USkeletalMeshComponent* CharacterMesh = GetMesh())
	{
		CharacterMesh->SetCustomDepthStencilValue(StencilValue);
	}
}

void ANPCCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (InteractableComponent)
	{
		InteractableComponent->CustomDepthStencilValue = StencilValue;

		TSet<UPrimitiveComponent*> HighlightableObjects;
		if (USkeletalMeshComponent* CharacterMesh = GetMesh())
		{
			CharacterMesh->SetCustomDepthStencilValue(StencilValue);
			HighlightableObjects.Add(CharacterMesh);
		}

		InteractableComponent->SetupInteractableReferences(InteractionCollision, InteractionWidget, HighlightableObjects);
	}
}

void ANPCCharacter::Initialize()
{
	if (InteractionWidget)
	{
		InteractionWidget->SetVisibility(false);
	}
}

void ANPCCharacter::EndInteraction(AActor* Interactor)
{
}

void ANPCCharacter::Interaction(AActor* Interactor)
{
	ARPGPlayerController* PlayerController = nullptr;
	if (APawn* InteractorPawn = Cast<APawn>(Interactor))
	{
		PlayerController = Cast<ARPGPlayerController>(InteractorPawn->GetController());
	}
	else
	{
		PlayerController = Cast<ARPGPlayerController>(Interactor);
	}

	bool bDialogueStarted = false;
	if (PlayerController && DialogueProviderComponent)
	{
		if (UDialogueManagerComponent* DialogueManager = PlayerController->FindComponentByClass<UDialogueManagerComponent>())
		{
			if (DialogueProviderComponent->CanStartDialogue(PlayerController))
			{
				bDialogueStarted = DialogueManager->StartDialogue(DialogueProviderComponent);
			}
		}
	}

	if (!bDialogueStarted && QuestGiverComponent)
	{
		QuestGiverComponent->OnQuestInteracted(Interactor);
	}
}

void ANPCCharacter::RemoveInteraction()
{
	if (InteractionWidget)
	{
		InteractionWidget->SetVisibility(false);
	}
}

bool ANPCCharacter::CanBeInteractedWith()
{
	return true;
}

void ANPCCharacter::ClientBeginInteraction(AActor* Interactor)
{
	if (InteractionWidget)
	{
		InteractionWidget->SetVisibility(true);
	}
}

void ANPCCharacter::ClientEndInteraction(AActor* Interactor)
{
	if (InteractionWidget)
	{
		InteractionWidget->SetVisibility(false);
	}
}
