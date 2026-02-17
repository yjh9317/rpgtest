#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Dialogue.h"
#include "DialogueRuntimeTypes.h"
#include "DialogueProviderComponent.generated.h"

UENUM(BlueprintType)
enum class EDialogueProvideMode : uint8
{
    /** 항상 같은 대화 */
    Single          UMETA(DisplayName = "Single Dialogue"),
    /** 처음/반복 구분 */
    FirstAndRepeat  UMETA(DisplayName = "First & Repeat"),
    /** 조건에 따라 다른 대화 */
    Conditional     UMETA(DisplayName = "Conditional")
};

/**
 * 조건부 대화 정의
 */
USTRUCT(BlueprintType)
struct FConditionalDialogue
{
    GENERATED_BODY()

    /** 대화 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UDialogue> Dialogue;

    /** 조건들 (모두 만족해야 함) */
    UPROPERTY(Instanced, EditDefaultsOnly, BlueprintReadOnly)
    TArray<UDialogueConditions*> Conditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Priority = 0;
};

UCLASS(ClassGroup=(Dialogue), meta=(BlueprintSpawnableComponent))
class DIALOGUESYSTEM_API UDialogueProviderComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDialogueProviderComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Info")
    FText NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Info")
    TObjectPtr<UTexture2D> DefaultPortrait;

    /** 상호작용 프롬프트 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Info")
    FText InteractionPrompt = FText::FromString("Talk");

    /** 상호작용 거리 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Info")
    float InteractionDistance = 200.0f;

    /** 대화 제공 방식 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EDialogueProvideMode ProvideMode = EDialogueProvideMode::Single;

    /** [Single] 단일 대화 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Single",
        meta = (EditCondition = "ProvideMode == EDialogueProvideMode::Single", EditConditionHides))
    TObjectPtr<UDialogue> Dialogue;

    /** [FirstAndRepeat] 처음 대화 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|First & Repeat",
        meta = (EditCondition = "ProvideMode == EDialogueProvideMode::FirstAndRepeat", EditConditionHides))
    TObjectPtr<UDialogue> FirstDialogue;

    /** [FirstAndRepeat] 반복 대화 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|First & Repeat",
        meta = (EditCondition = "ProvideMode == EDialogueProvideMode::FirstAndRepeat", EditConditionHides))
    TObjectPtr<UDialogue> RepeatDialogue;

    /** [Conditional] 조건부 대화 목록 (우선순위 높은 순으로 체크) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Conditional",
        meta = (EditCondition = "ProvideMode == EDialogueProvideMode::Conditional", EditConditionHides))
    TArray<FConditionalDialogue> ConditionalDialogues;

    /** [Conditional] 기본 대화 (조건 모두 실패 시) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Conditional",
        meta = (EditCondition = "ProvideMode == EDialogueProvideMode::Conditional", EditConditionHides))
    TObjectPtr<UDialogue> DefaultDialogue;

    // ========== 대화 시작 조건 ==========
    
    /** 대화 시작 전 체크할 조건들 */
    UPROPERTY(Instanced, EditDefaultsOnly, BlueprintReadOnly, Category = "Dialogue|Conditions")
    TArray<UDialogueConditions*> StartConditions;

    /** 한 번만 대화 가능 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Options")
    bool bOneTimeOnly = false;

    /** 대화 후 비활성화 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Options")
    bool bDisableAfterDialogue = false;

    // ========== 상태 ==========
    
    /** 이미 대화했는지 여부 */
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue|State")
    bool bHasSpoken = false;

    /** 대화한 횟수 */
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue|State")
    int32 DialogueCount = 0;

    // ========== 함수 ==========
    
    /**
     * 대화 시작 가능한지 체크
     */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool CanStartDialogue(APlayerController* Player) const;

    /**
     * 제공할 대화 가져오기 (조건에 맞는 대화 선택)
     */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    UDialogue* GetDialogueToProvide(APlayerController* Player);

    /**
     * 대화 시작됨 (DialogueManager가 호출)
     */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void OnDialogueStarted();

    /**
     * 대화 종료됨 (DialogueManager가 호출)
     */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void OnDialogueFinished(bool bCompleted);

    /**
     * 상태 리셋
     */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void ResetDialogueState();

protected:
    virtual void BeginPlay() override;

private:
    /**
     * 조건부 대화 선택
     */
    UDialogue* SelectConditionalDialogue(APlayerController* Player);

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Presentation")
    TMap<int32, FDialogueOptionMeta> NodeOptionMetadata;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Presentation")
    TMap<int32, FDialogueCinematicCue> NodeCinematicCues;

    UFUNCTION(BlueprintPure, Category = "Dialogue|Presentation")
    bool GetOptionMetadata(int32 NodeId, FDialogueOptionMeta& OutMeta) const;

    UFUNCTION(BlueprintPure, Category = "Dialogue|Presentation")
    bool GetCinematicCue(int32 NodeId, FDialogueCinematicCue& OutCue) const;

    UFUNCTION(BlueprintPure, Category = "Dialogue|Presentation")
    FText ResolveSpeakerName() const;

    UFUNCTION(BlueprintPure, Category = "Dialogue|Presentation")
    UTexture2D* ResolveSpeakerPortrait() const;
};
