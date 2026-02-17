#pragma once

#include "CoreMinimal.h"
#include "Dialogue.h"
#include "DialogueRuntimeTypes.h"
#include "UObject/NoExportTypes.h"
#include "DialogueViewModel.generated.h"

class UDialogueProviderComponent;

UCLASS()
class DIALOGUESYSTEM_API UDialogueViewModel : public UObject
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueOpened);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueClosed, bool, bCompleted);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpeakerDialogueChanged, const FDialogueNode&, Node, AActor*, SpeakerActor);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerOptionsChanged, const TArray<FDialogueNode>&, Options);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueEnded);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpeakerChanged, const FDialogueSpeakerPayload&, Payload);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRuntimeOptionsUpdated, const TArray<FDialogueRuntimeOption>&, Options);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHistoryAdded, const FDialogueHistoryEntryEx&, Entry);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCinematicCueRequested, const FDialogueCinematicCue&, Cue);

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnDialogueOpened OnDialogueOpened;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnDialogueClosed OnDialogueClosed;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnSpeakerDialogueChanged OnSpeakerDialogueChanged;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnPlayerOptionsChanged OnPlayerOptionsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnDialogueEnded OnDialogueEnded;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnSpeakerChanged OnSpeakerChanged;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnRuntimeOptionsUpdated OnRuntimeOptionsUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnHistoryAdded OnHistoryAdded;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnCinematicCueRequested OnCinematicCueRequested;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	TObjectPtr<UDialogue> CurrentDialogue = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	FDialogueNode CurrentNode;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	TObjectPtr<AActor> CurrentSpeaker = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	TArray<FDialogueNode> CurrentPlayerOptions;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	TArray<FDialogueRuntimeOption> CurrentRuntimeOptions;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	bool bIsInDialogue = false;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	TArray<FDialogueHistoryEntryEx> DialogueHistory;

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartDialogue(UDialogue* Dialogue, AActor* SpeakerActor);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartDialogueExtended(UDialogue* Dialogue, AActor* SpeakerActor, UDialogueProviderComponent* Provider, APlayerController* InOwningPlayer);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void EndDialogue(bool bCompleted = false);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void ShowSpeakerNode(const FDialogueNode& Node);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void ShowPlayerOptions(const TArray<FDialogueNode>& Options);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void SelectOption(int32 NodeId);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void SelectOptionByIndex(int32 OptionIndex);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void AdvanceToNode(int32 NodeId);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	TArray<FDialogueNode> GetValidNextNodes(const FDialogueNode& FromNode, APlayerController* PC);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void AddToHistory(const FDialogueNode& Node, bool bWasPlayerChoice);

private:
	UPROPERTY()
	TObjectPtr<UDialogueProviderComponent> CurrentProvider = nullptr;

	UPROPERTY()
	TObjectPtr<APlayerController> OwningPlayer = nullptr;

	void ProcessNode(const FDialogueNode& Node);
	void BuildRuntimeOptions(const TArray<FDialogueNode>& CandidateNodes, TArray<FDialogueRuntimeOption>& OutOptions) const;
	bool AreNodeConditionsMet(const FDialogueNode& Node, APlayerController* PC) const;
	void TryEmitCinematicCue(int32 NodeId) const;
};
