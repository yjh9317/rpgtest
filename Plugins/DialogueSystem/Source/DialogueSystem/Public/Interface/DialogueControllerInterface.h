#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Dialogue.h"
#include "DialogueControllerInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UDialogueControllerInterface : public UInterface
{
    GENERATED_BODY()
};

class DIALOGUESYSTEM_API IDialogueControllerInterface
{
    GENERATED_BODY()

public:
    // ========== 대화 시작/종료 ==========
    
    /** 대화 시작 */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dialogue")
    void StartDialogue(UDialogue* Dialogue, AActor* NPCActor);
    
    /** 대화 종료 */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dialogue")
    void EndDialogue();
    
    // ========== UI 표시 ==========
    
    /** NPC 대사 표시 */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dialogue")
    void DisplayNPCDialogue(const FDialogueNode& Node, AActor* NPCActor);
    
    /** 플레이어 선택지 표시 */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dialogue")
    void DisplayPlayerOptions(const TArray<FDialogueNode>& Options);
    
    /** 대화 UI 숨기기 */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dialogue")
    void HideDialogueUI();
    
    // ========== 선택 처리 ==========
    
    /** 플레이어가 선택지를 선택했을 때 */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dialogue")
    void OnPlayerSelectOption(int32 NodeId);
    
    // ========== 대화 상태 ==========
    
    /** 현재 대화 중인지 확인 */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dialogue")
    bool IsInDialogue() const;
    
    /** 대화 시작 가능한지 확인 */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dialogue")
    bool CanStartDialogue() const;
    
    // ========== 카메라/입력 제어 ==========
    
    /** 대화용 카메라로 전환 (BG3 스타일) */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dialogue")
    void SwitchToDialogueCamera(AActor* NPCActor);
    
    /** 일반 카메라로 복귀 */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dialogue")
    void RestoreGameplayCamera();
    
    /** 대화 중 입력 모드 설정 */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dialogue")
    void SetDialogueInputMode(bool bDialogueMode);
    
    // ========== 캐릭터 제어 ==========
    
    /** NPC를 바라보게 하기 */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dialogue")
    void FaceNPC(AActor* NPCActor);
    
    /** 대화 중 캐릭터 이동 제한 */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dialogue")
    void SetCharacterMovementEnabled(bool bEnabled);
    
    // ========== 추가 기능 (BG3 스타일) ==========
    
    /** 대화 히스토리 추가 */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dialogue")
    void AddToDialogueHistory(const FDialogueNode& Node, bool bIsPlayerChoice);
    
    /** 대화 스킵 가능 여부 */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dialogue")
    bool CanSkipDialogue() const;
};