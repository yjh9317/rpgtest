// DialogueProviderComponent.cpp
#include "Component/DialogueProviderComponent.h"
#include "Engine/GameInstance.h"

UDialogueProviderComponent::UDialogueProviderComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UDialogueProviderComponent::BeginPlay()
{
    Super::BeginPlay();

    UGameInstance* GI = GetWorld()->GetGameInstance();
    if (!GI)
    {
        return;
    }

    auto AssignOuter = [GI](UDialogue* Dlg)
    {
        if (Dlg)
        {
            Dlg->AssignPersistentOuter(GI);
        }
    };

    AssignOuter(Dialogue);
    AssignOuter(FirstDialogue);
    AssignOuter(RepeatDialogue);
    AssignOuter(DefaultDialogue);

    for (FConditionalDialogue& ConditionalDlg : ConditionalDialogues)
    {
        AssignOuter(ConditionalDlg.Dialogue);
    }
}

bool UDialogueProviderComponent::CanStartDialogue(APlayerController* Player) const
{
    // 비활성화 체크
    if (bDisableAfterDialogue && bHasSpoken)
    {
        return false;
    }

    // 한 번만 가능 체크
    if (bOneTimeOnly && bHasSpoken)
    {
        return false;
    }

    // 제공할 대화가 있는지 체크
    UDialogue* DialogueToProvide = const_cast<UDialogueProviderComponent*>(this)->GetDialogueToProvide(Player);
    if (!DialogueToProvide)
    {
        return false;
    }

    // 시작 조건 체크
    for (UDialogueConditions* Condition : StartConditions)
    {
        if (IsValid(Condition))
        {
            if (!Condition->IsConditionMet(Player, GetOwner()))
            {
                return false;
            }
        }
    }

    return true;
}

UDialogue* UDialogueProviderComponent::GetDialogueToProvide(APlayerController* Player)
{
    switch (ProvideMode)
    {
        case EDialogueProvideMode::Single:
            return Dialogue;

        case EDialogueProvideMode::FirstAndRepeat:
            // 처음이면 FirstDialogue, 아니면 RepeatDialogue
            if (!bHasSpoken && FirstDialogue)
            {
                return FirstDialogue;
            }
            return RepeatDialogue;

        case EDialogueProvideMode::Conditional:
            return SelectConditionalDialogue(Player);

        default:
            return nullptr;
    }
}

UDialogue* UDialogueProviderComponent::SelectConditionalDialogue(APlayerController* Player)
{
    // 우선순위 순으로 정렬
    TArray<FConditionalDialogue> SortedDialogues = ConditionalDialogues;
    SortedDialogues.Sort([](const FConditionalDialogue& A, const FConditionalDialogue& B)
    {
        return A.Priority > B.Priority;
    });

    // 조건 만족하는 첫 번째 대화 반환
    for (const FConditionalDialogue& ConditionalDlg : SortedDialogues)
    {
        if (!ConditionalDlg.Dialogue)
        {
            continue;
        }

        bool bAllConditionsMet = true;
        for (UDialogueConditions* Condition : ConditionalDlg.Conditions)
        {
            if (IsValid(Condition))
            {
                if (!Condition->IsConditionMet(Player, GetOwner()))
                {
                    bAllConditionsMet = false;
                    break;
                }
            }
        }

        if (bAllConditionsMet)
        {
            return ConditionalDlg.Dialogue;
        }
    }

    // 조건 만족하는 대화 없으면 기본 대화
    return DefaultDialogue;
}

void UDialogueProviderComponent::OnDialogueStarted()
{
    UE_LOG(LogTemp, Log, TEXT("Dialogue started with %s (Count: %d)"), 
        *GetOwner()->GetName(), DialogueCount + 1);
}

void UDialogueProviderComponent::OnDialogueFinished(bool bCompleted)
{
    if (bCompleted)
    {
        bHasSpoken = true;
        DialogueCount++;

        UE_LOG(LogTemp, Log, TEXT("Dialogue finished with %s (Total: %d times)"), 
            *GetOwner()->GetName(), DialogueCount);
    }
}

void UDialogueProviderComponent::ResetDialogueState()
{
    bHasSpoken = false;
    DialogueCount = 0;
}

bool UDialogueProviderComponent::GetOptionMetadata(int32 NodeId, FDialogueOptionMeta& OutMeta) const
{
	if (const FDialogueOptionMeta* Found = NodeOptionMetadata.Find(NodeId))
	{
		OutMeta = *Found;
		return true;
	}

	return false;
}

bool UDialogueProviderComponent::GetCinematicCue(int32 NodeId, FDialogueCinematicCue& OutCue) const
{
	if (const FDialogueCinematicCue* Found = NodeCinematicCues.Find(NodeId))
	{
		OutCue = *Found;
		return true;
	}

	return false;
}

FText UDialogueProviderComponent::ResolveSpeakerName() const
{
	if (!NPCName.IsEmpty())
	{
		return NPCName;
	}

	return GetOwner() ? FText::FromString(GetOwner()->GetName()) : FText::GetEmpty();
}

UTexture2D* UDialogueProviderComponent::ResolveSpeakerPortrait() const
{
	return DefaultPortrait;
}
