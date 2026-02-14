// DialogueViewModel.cpp
#include "UI/DialogueViewModel.h"

void UDialogueViewModel::StartDialogue(UDialogue* Dialogue, AActor* SpeakerActor)
{
    if (!Dialogue || !SpeakerActor) return;
    
    CurrentDialogue = Dialogue;
    CurrentSpeaker = SpeakerActor;
    bIsInDialogue = true;
    DialogueHistory.Empty();
   
    FDialogueNode FirstNode = Dialogue->GetFirstNode();
    if (FirstNode.id != -1)
    {
        ProcessNode(FirstNode);
    }
}

void UDialogueViewModel::EndDialogue()
{
    bIsInDialogue = false;
    CurrentDialogue = nullptr;
    CurrentSpeaker = nullptr;
    CurrentPlayerOptions.Empty();
    
    OnDialogueEnded.Broadcast();
}

void UDialogueViewModel::ProcessNode(const FDialogueNode& Node)
{
    CurrentNode = Node;
    
    if (Node.isPlayer)
    {
        // 플레이어 노드면 자동으로 다음으로
        AddToHistory(Node, true);
        
        TArray<FDialogueNode> NextNodes = CurrentDialogue->GetNextNodes(Node);
        if (NextNodes.Num() > 0)
        {
            ProcessNode(NextNodes[0]);
        }
        else
        {
            EndDialogue();
        }
    }
    else
    {
        // Speaker 노드
        ShowSpeakerNode(Node);
        AddToHistory(Node, false);
        
        // 다음 선택지 준비
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        TArray<FDialogueNode> ValidOptions = GetValidNextNodes(Node, PC);
        
        if (ValidOptions.Num() > 0)
        {
            ShowPlayerOptions(ValidOptions);
        }
        else
        {
            EndDialogue();
        }
    }
}

void UDialogueViewModel::ShowSpeakerNode(const FDialogueNode& Node)
{
    OnSpeakerDialogueChanged.Broadcast(Node, CurrentSpeaker);
}

void UDialogueViewModel::ShowPlayerOptions(const TArray<FDialogueNode>& Options)
{
    CurrentPlayerOptions = Options;
    OnPlayerOptionsChanged.Broadcast(Options);
}

void UDialogueViewModel::SelectOption(int32 NodeId)
{
    for (const FDialogueNode& Option : CurrentPlayerOptions)
    {
        if (Option.id == NodeId)
        {
            ProcessNode(Option);
            return;
        }
    }
}

void UDialogueViewModel::AdvanceToNode(int32 NodeId)
{
}

TArray<FDialogueNode> UDialogueViewModel::GetValidNextNodes(const FDialogueNode& FromNode, APlayerController* PC)
{
    TArray<FDialogueNode> ValidNodes;
    TArray<FDialogueNode> AllNextNodes = CurrentDialogue->GetNextNodes(FromNode);
    
    for (const FDialogueNode& Node : AllNextNodes)
    {
        // 조건 검사
        bool bConditionsMet = true;
        for (UDialogueConditions* Condition : Node.Conditions)
        {
            if (IsValid(Condition))
            {
                if (!Condition->IsConditionMet(PC, CurrentSpeaker))
                {
                    bConditionsMet = false;
                    break;
                }
            }
        }
        
        if (bConditionsMet)
        {
            ValidNodes.Add(Node);
        }
    }
    
    return ValidNodes;
}

void UDialogueViewModel::AddToHistory(const FDialogueNode& Node, bool bWasPlayerChoice)
{
    FDialogueHistoryEntry Entry;
    Entry.Node = Node;
    Entry.bWasPlayerChoice = bWasPlayerChoice;
    Entry.Timestamp = FDateTime::Now();
    
    DialogueHistory.Add(Entry);
}