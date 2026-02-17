#include "Component/DialogueManagerComponent.h"

#include "Blueprint/UserWidget.h"
#include "Component/DialogueProviderComponent.h"
#include "GameFramework/PlayerController.h"
#include "Interface/DialogueControllerInterface.h"
#include "Kismet/GameplayStatics.h"
#include "UI/DialogueUserWidget.h"
#include "UI/DialogueViewModel.h"

UDialogueManagerComponent::UDialogueManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDialogueManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	DialogueViewModel = NewObject<UDialogueViewModel>(this, TEXT("DialogueViewModel"));
	if (DialogueViewModel)
	{
		DialogueViewModel->OnDialogueClosed.AddDynamic(this, &UDialogueManagerComponent::OnViewModelDialogueClosed);
		DialogueViewModel->OnCinematicCueRequested.AddDynamic(this, &UDialogueManagerComponent::OnViewModelCinematicCue);
	}

	UE_LOG(LogTemp, Log, TEXT("DialogueManagerComponent initialized"));
}

void UDialogueManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bIsInDialogue)
	{
		EndDialogueInternal(false, true);
	}

	Super::EndPlay(EndPlayReason);
}

bool UDialogueManagerComponent::StartDialogue(UDialogueProviderComponent* Provider)
{
	if (!Provider || bIsInDialogue || !DialogueViewModel)
	{
		return false;
	}

	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC)
	{
		return false;
	}

	if (!Provider->CanStartDialogue(PC))
	{
		return false;
	}

	UDialogue* DialogueToStart = Provider->GetDialogueToProvide(PC);
	if (!DialogueToStart)
	{
		return false;
	}

	CurrentProvider = Provider;
	bIsInDialogue = true;

	Provider->OnDialogueStarted();

	CreateDialogueWidget();
	SetupInputMode(true);

	if (PC->GetClass()->ImplementsInterface(UDialogueControllerInterface::StaticClass()))
	{
		IDialogueControllerInterface::Execute_SetCharacterMovementEnabled(PC, false);
		IDialogueControllerInterface::Execute_SwitchToDialogueCamera(PC, Provider->GetOwner());
	}

	if (bPauseGameDuringDialogue)
	{
		UGameplayStatics::SetGamePaused(GetWorld(), true);
	}

	DialogueViewModel->StartDialogueExtended(DialogueToStart, Provider->GetOwner(), Provider, PC);

	OnDialogueStarted.Broadcast(Provider);
	return true;
}

void UDialogueManagerComponent::EndDialogue(bool bCompleted)
{
	EndDialogueInternal(bCompleted, true);
}

void UDialogueManagerComponent::EndDialogueInternal(bool bCompleted, bool bNotifyViewModel)
{
	if (!bIsInDialogue)
	{
		return;
	}

	bIsInDialogue = false;

	UDialogueProviderComponent* FinishedProvider = CurrentProvider;
	CurrentProvider = nullptr;

	if (bNotifyViewModel && DialogueViewModel && DialogueViewModel->bIsInDialogue)
	{
		DialogueViewModel->EndDialogue(bCompleted);
	}

	RemoveDialogueWidget();
	SetupInputMode(false);

	if (bPauseGameDuringDialogue)
	{
		UGameplayStatics::SetGamePaused(GetWorld(), false);
	}

	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (PC && PC->GetClass()->ImplementsInterface(UDialogueControllerInterface::StaticClass()))
	{
		IDialogueControllerInterface::Execute_RestoreGameplayCamera(PC);
		IDialogueControllerInterface::Execute_SetCharacterMovementEnabled(PC, true);
	}

	if (FinishedProvider)
	{
		FinishedProvider->OnDialogueFinished(bCompleted);
	}

	OnDialogueEnded.Broadcast(bCompleted);
}

void UDialogueManagerComponent::CreateDialogueWidget()
{
	if (!DialogueWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("DialogueManager: DialogueWidgetClass not set"));
		return;
	}

	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC)
	{
		return;
	}

	DialogueWidget = CreateWidget<UDialogueUserWidget>(PC, DialogueWidgetClass);
	if (DialogueWidget)
	{
		DialogueWidget->NPCActor = CurrentProvider ? CurrentProvider->GetOwner() : nullptr;
		DialogueWidget->ViewModel = DialogueViewModel;
		DialogueWidget->AddToViewport(100);
	}
}

void UDialogueManagerComponent::RemoveDialogueWidget()
{
	if (DialogueWidget)
	{
		DialogueWidget->RemoveFromParent();
		DialogueWidget = nullptr;
	}
}

void UDialogueManagerComponent::SetupInputMode(bool bDialogueMode)
{
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC)
	{
		return;
	}

	if (bDialogueMode)
	{
		if (bBlockInputDuringDialogue)
		{
			FInputModeUIOnly InputMode;
			if (DialogueWidget)
			{
				InputMode.SetWidgetToFocus(DialogueWidget->TakeWidget());
			}
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = bShowMouseCursor;
		}
	}
	else
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}
}

void UDialogueManagerComponent::OnViewModelDialogueClosed(bool bCompleted)
{
	EndDialogueInternal(bCompleted, false);
}

void UDialogueManagerComponent::OnViewModelCinematicCue(const FDialogueCinematicCue& Cue)
{
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC || !PC->GetClass()->ImplementsInterface(UDialogueControllerInterface::StaticClass()))
	{
		return;
	}

	IDialogueControllerInterface::Execute_ApplyDialogueCinematicCue(
		PC,
		Cue,
		CurrentProvider ? CurrentProvider->GetOwner() : nullptr
	);
}
