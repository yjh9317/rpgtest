// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Dialogue.h"
#include "UObject/NoExportTypes.h"
#include "DialogueViewModel.generated.h"

USTRUCT(BlueprintType)
struct FDialogueHistoryEntry
{
	GENERATED_BODY()
        
	UPROPERTY(BlueprintReadOnly)
	FDialogueNode Node;
        
	UPROPERTY(BlueprintReadOnly)
	bool bWasPlayerChoice;
        
	UPROPERTY(BlueprintReadOnly)
	FDateTime Timestamp;
};

UCLASS()
class DIALOGUESYSTEM_API UDialogueViewModel : public UObject
{
	GENERATED_BODY()
public:
    
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpeakerDialogueChanged, const FDialogueNode&, Node, AActor*, SpeakerActor);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerOptionsChanged, const TArray<FDialogueNode>&, Options);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueEnded);
    
    UPROPERTY(BlueprintAssignable, Category = "Dialogue")
    FOnSpeakerDialogueChanged OnSpeakerDialogueChanged;
    
    UPROPERTY(BlueprintAssignable, Category = "Dialogue")
    FOnPlayerOptionsChanged OnPlayerOptionsChanged;
    
    UPROPERTY(BlueprintAssignable, Category = "Dialogue")
    FOnDialogueEnded OnDialogueEnded;
        
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    UDialogue* CurrentDialogue;
    
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FDialogueNode CurrentNode;
    
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    AActor* CurrentSpeaker;
    
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    TArray<FDialogueNode> CurrentPlayerOptions;
    
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    bool bIsInDialogue = false;
    
    
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    TArray<FDialogueHistoryEntry> DialogueHistory;
    
    // ========== 메서드 ==========
    
    /** 대화 시작 */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StartDialogue(UDialogue* Dialogue, AActor* SpeakerActor);
    
    /** 대화 종료 */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();
    
    /** Speaker 노드 표시 */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void ShowSpeakerNode(const FDialogueNode& Node);
    
    /** 플레이어 선택지 표시 */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void ShowPlayerOptions(const TArray<FDialogueNode>& Options);
    
    /** 플레이어가 선택 */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SelectOption(int32 NodeId);
    
    /** 다음 노드로 진행 */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AdvanceToNode(int32 NodeId);
    
    /** 조건 검사된 다음 노드들 가져오기 */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FDialogueNode> GetValidNextNodes(const FDialogueNode& FromNode, APlayerController* PC);
    
    /** 히스토리에 추가 */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AddToHistory(const FDialogueNode& Node, bool bWasPlayerChoice);

private:
    void ProcessNode(const FDialogueNode& Node);
};
