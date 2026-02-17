#include "UI/DialogueViewModel.h"

#include "Component/DialogueProviderComponent.h"

void UDialogueViewModel::StartDialogue(UDialogue* Dialogue, AActor* SpeakerActor)
{
	StartDialogueExtended(Dialogue, SpeakerActor, nullptr, GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr);
}

void UDialogueViewModel::StartDialogueExtended(UDialogue* Dialogue, AActor* SpeakerActor,
	UDialogueProviderComponent* Provider, APlayerController* InOwningPlayer)
{
	if (!Dialogue || !SpeakerActor)
	{
		return;
	}

	CurrentDialogue = Dialogue;
	CurrentSpeaker = SpeakerActor;
	CurrentProvider = Provider;
	OwningPlayer = InOwningPlayer;
	bIsInDialogue = true;
	DialogueHistory.Empty();
	CurrentPlayerOptions.Empty();
	CurrentRuntimeOptions.Empty();

	OnDialogueOpened.Broadcast();

	const FDialogueNode FirstNode = Dialogue->GetFirstNode();
	if (FirstNode.id != -1)
	{
		ProcessNode(FirstNode);
	}
	else
	{
		EndDialogue(false);
	}
}

void UDialogueViewModel::EndDialogue(bool bCompleted)
{
	if (!bIsInDialogue)
	{
		return;
	}

	bIsInDialogue = false;
	CurrentDialogue = nullptr;
	CurrentSpeaker = nullptr;
	CurrentProvider = nullptr;
	OwningPlayer = nullptr;
	CurrentPlayerOptions.Empty();
	CurrentRuntimeOptions.Empty();

	OnDialogueClosed.Broadcast(bCompleted);
	OnDialogueEnded.Broadcast();
}

void UDialogueViewModel::ProcessNode(const FDialogueNode& Node)
{
	CurrentNode = Node;

	if (Node.isPlayer)
	{
		AddToHistory(Node, true);

		const TArray<FDialogueNode> NextNodes = CurrentDialogue->GetNextNodes(Node);
		if (NextNodes.Num() > 0)
		{
			ProcessNode(NextNodes[0]);
		}
		else
		{
			EndDialogue(true);
		}
		return;
	}

	ShowSpeakerNode(Node);
	AddToHistory(Node, false);
	TryEmitCinematicCue(Node.id);

	TArray<FDialogueRuntimeOption> RuntimeOptions;
	BuildRuntimeOptions(CurrentDialogue->GetNextNodes(Node), RuntimeOptions);
	CurrentRuntimeOptions = RuntimeOptions;

	CurrentPlayerOptions.Empty();
	for (const FDialogueRuntimeOption& Option : CurrentRuntimeOptions)
	{
		CurrentPlayerOptions.Add(Option.Node);
	}

	OnPlayerOptionsChanged.Broadcast(CurrentPlayerOptions);
	OnRuntimeOptionsUpdated.Broadcast(CurrentRuntimeOptions);

	if (CurrentRuntimeOptions.Num() == 0)
	{
		EndDialogue(true);
	}
}

void UDialogueViewModel::ShowSpeakerNode(const FDialogueNode& Node)
{
	OnSpeakerDialogueChanged.Broadcast(Node, CurrentSpeaker.Get());

	FDialogueSpeakerPayload Payload;
	Payload.Node = Node;
	Payload.SpeakerActor = CurrentSpeaker;
	Payload.SpeakerName = CurrentProvider ? CurrentProvider->ResolveSpeakerName() : FText::GetEmpty();
	Payload.Portrait = CurrentProvider ? CurrentProvider->ResolveSpeakerPortrait() : nullptr;
	OnSpeakerChanged.Broadcast(Payload);
}

void UDialogueViewModel::ShowPlayerOptions(const TArray<FDialogueNode>& Options)
{
	CurrentPlayerOptions = Options;
	OnPlayerOptionsChanged.Broadcast(Options);
}

void UDialogueViewModel::SelectOption(int32 NodeId)
{
	for (const FDialogueRuntimeOption& Option : CurrentRuntimeOptions)
	{
		if (Option.Node.id == NodeId)
		{
			if (!Option.bSelectable)
			{
				return;
			}

			ProcessNode(Option.Node);
			return;
		}
	}

	for (const FDialogueNode& Option : CurrentPlayerOptions)
	{
		if (Option.id == NodeId)
		{
			ProcessNode(Option);
			return;
		}
	}
}

void UDialogueViewModel::SelectOptionByIndex(int32 OptionIndex)
{
	for (const FDialogueRuntimeOption& Option : CurrentRuntimeOptions)
	{
		if (Option.OptionIndex == OptionIndex)
		{
			SelectOption(Option.Node.id);
			return;
		}
	}
}

void UDialogueViewModel::AdvanceToNode(int32 NodeId)
{
	if (!CurrentDialogue)
	{
		return;
	}

	bool bFound = false;
	const FDialogueNode NextNode = CurrentDialogue->GetNodeById(NodeId, bFound);
	if (bFound)
	{
		ProcessNode(NextNode);
	}
}

bool UDialogueViewModel::AreNodeConditionsMet(const FDialogueNode& Node, APlayerController* PC) const
{
	for (UDialogueConditions* Condition : Node.Conditions)
	{
		if (IsValid(Condition) && !Condition->IsConditionMet(PC, CurrentSpeaker.Get()))
		{
			return false;
		}
	}

	return true;
}

void UDialogueViewModel::BuildRuntimeOptions(const TArray<FDialogueNode>& CandidateNodes,
	TArray<FDialogueRuntimeOption>& OutOptions) const
{
	int32 RunningIndex = 1;
	for (const FDialogueNode& Node : CandidateNodes)
	{
		const bool bConditionsMet = AreNodeConditionsMet(Node, OwningPlayer.Get());

		FDialogueOptionMeta Meta;
		if (CurrentProvider)
		{
			CurrentProvider->GetOptionMetadata(Node.id, Meta);
		}

		FDialogueRuntimeOption Runtime;
		Runtime.Node = Node;
		Runtime.Meta = Meta;
		Runtime.OptionIndex = RunningIndex++;
		Runtime.bAvailable = bConditionsMet;
		Runtime.bSelectable = bConditionsMet || Meta.bSelectableWhenUnavailable;
		Runtime.bVisible = bConditionsMet || !Meta.bHideIfUnavailable;
		Runtime.DisplayText = Node.Text;

		if (!bConditionsMet)
		{
			Runtime.UnavailableReason = Meta.UnavailableReason.IsEmpty()
				? NSLOCTEXT("Dialogue", "DefaultUnavailableReason", "Requirements are not met.")
				: Meta.UnavailableReason;
		}

		if (Meta.bHasSkillCheck)
		{
			FFormatNamedArguments Args;
			Args.Add(TEXT("Skill"), FText::FromName(Meta.SkillCheckId));
			Args.Add(TEXT("DC"), FText::AsNumber(Meta.SkillCheckDC));
			Runtime.DecoratedPrefix = FText::Format(
				NSLOCTEXT("Dialogue", "SkillCheckPrefix", "[{Skill} DC {DC}] "),
				Args);
		}

		if (Runtime.bVisible)
		{
			OutOptions.Add(Runtime);
		}
	}
}

void UDialogueViewModel::TryEmitCinematicCue(int32 NodeId) const
{
	if (!CurrentProvider)
	{
		return;
	}

	FDialogueCinematicCue Cue;
	if (CurrentProvider->GetCinematicCue(NodeId, Cue))
	{
		OnCinematicCueRequested.Broadcast(Cue);
	}
}

TArray<FDialogueNode> UDialogueViewModel::GetValidNextNodes(const FDialogueNode& FromNode, APlayerController* PC)
{
	TArray<FDialogueNode> ValidNodes;
	if (!CurrentDialogue)
	{
		return ValidNodes;
	}

	const TArray<FDialogueNode> AllNextNodes = CurrentDialogue->GetNextNodes(FromNode);
	for (const FDialogueNode& Node : AllNextNodes)
	{
		if (AreNodeConditionsMet(Node, PC))
		{
			ValidNodes.Add(Node);
		}
	}

	return ValidNodes;
}

void UDialogueViewModel::AddToHistory(const FDialogueNode& Node, bool bWasPlayerChoice)
{
	FDialogueHistoryEntryEx Entry;
	Entry.Node = Node;
	Entry.bPlayerChoice = bWasPlayerChoice;
	Entry.DisplayText = Node.Text;
	Entry.Timestamp = FDateTime::Now();

	DialogueHistory.Add(Entry);
	OnHistoryAdded.Broadcast(Entry);
}
