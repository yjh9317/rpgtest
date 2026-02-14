// DialogueManagerComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DialogueManagerComponent.generated.h"

class UDialogueViewModel;
class UDialogueProviderComponent;
class UDialogueUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueStartedSignature, UDialogueProviderComponent*, Provider);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueEndedSignature, bool, bCompleted);

/**
 * PlayerController에 붙여서 대화 시스템을 관리하는 컴포넌트
 * "내가 대화를 진행하고 관리합니다"
 */
UCLASS(ClassGroup=(Dialogue), meta=(BlueprintSpawnableComponent))
class DIALOGUESYSTEM_API UDialogueManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDialogueManagerComponent();

    // ========== UI 설정 ==========
    
    /** 대화 UI 위젯 클래스 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Dialogue UI")
    TSubclassOf<UDialogueUserWidget> DialogueWidgetClass;
    
    UPROPERTY()
    TObjectPtr<UDialogueViewModel> DialogueViewModel;

    // ========== 대화 옵션 ==========
    
    /** 대화 중 입력 차단 여부 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Options")
    bool bBlockInputDuringDialogue = true;

    /** 대화 중 게임 일시정지 여부 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Options")
    bool bPauseGameDuringDialogue = false;

    /** 대화 중 마우스 표시 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Options")
    bool bShowMouseCursor = true;

    // ========== 대화 제어 ==========
    
    /**
     * 대화 시작
     */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogue(UDialogueProviderComponent* Provider);

    /**
     * 대화 종료
     */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue(bool bCompleted = false);

    /**
     * 현재 대화 중인지
     */
    UFUNCTION(BlueprintPure, Category = "Dialogue")
    bool IsInDialogue() const { return bIsInDialogue; }

    /**
     * 현재 대화 상대
     */
    UFUNCTION(BlueprintPure, Category = "Dialogue")
    UDialogueProviderComponent* GetCurrentProvider() const { return CurrentProvider; }

    // ========== 컴포넌트 접근 ==========
    
    /**
     * DialogueController 가져오기
     */
    // UFUNCTION(BlueprintPure, Category = "Dialogue")
    // UPlayer* GetDialogueController() const { return DialogueController; }

    /**
     * ViewModel 가져오기
     */
    // UFUNCTION(BlueprintPure, Category = "Dialogue")
    // UDialogueViewModel* GetViewModel() const { return ViewModel; }

    /**
     * 현재 대화 UI
     */
    UFUNCTION(BlueprintPure, Category = "Dialogue")
    UDialogueUserWidget* GetDialogueWidget() const { return DialogueWidget; }

    // ========== 델리게이트 ==========
    
    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueStartedSignature OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueEndedSignature OnDialogueEnded;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // ========== 내부 컴포넌트 ==========
    
    /** 대화 로직 관리자 */
    // UPROPERTY()
    // TObjectPtr<UDialogueController> DialogueController;

    /** UI 데이터 관리자 */
    // UPROPERTY()
    // TObjectPtr<UDialogueViewModel> ViewModel;

    UPROPERTY()
    TObjectPtr<UDialogueUserWidget> DialogueWidget;

    UPROPERTY()
    TObjectPtr<UDialogueProviderComponent> CurrentProvider;

    /** 대화 중 플래그 */
    UPROPERTY()
    bool bIsInDialogue = false;
    
    void CreateDialogueWidget();
    void RemoveDialogueWidget();
    void SetupInputMode(bool bDialogueMode);


    UFUNCTION()
    void OnDialogueEndedInternal(bool bCompleted);
};