// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/DialogueManagerComponent.h"
#include "Component/DialogueProviderComponent.h"
#include "UI/DialogueUserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "UI/DialogueViewModel.h"

UDialogueManagerComponent::UDialogueManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UDialogueManagerComponent::BeginPlay()
{
    Super::BeginPlay();

    // DialogueController 생성
    // DialogueController = NewObject<UDialogueController>(this, TEXT("DialogueController"));
    // if (DialogueController)
    // {
    //     DialogueController->OnDialogueEnded.AddDynamic(this, &UDialogueManagerComponent::OnDialogueEndedInternal);
    // }

    // ViewModel 생성
    DialogueViewModel = NewObject<UDialogueViewModel>(this, TEXT("DialogueViewModel"));
    // if (DialogueViewModel && DialogueController)
    // {
    //     DialogueViewModel->Initialize(DialogueController);
    // }

    UE_LOG(LogTemp, Log, TEXT("DialogueManagerComponent initialized"));
}

void UDialogueManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (bIsInDialogue)
    {
        EndDialogue(false);
    }

    if (DialogueViewModel)
    {
        // DialogueViewModel->Cleanup();
    }

    Super::EndPlay(EndPlayReason);
}

bool UDialogueManagerComponent::StartDialogue(UDialogueProviderComponent* Provider)
{
    if (!Provider)
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Invalid Provider!"));
        return false;
    }

    if (bIsInDialogue)
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Already in dialogue!"));
        return false;
    }

    APlayerController* PC = Cast<APlayerController>(GetOwner());
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("DialogueManager: Owner is not PlayerController!"));
        return false;
    }

    // Provider에게 대화 가져오기
    UDialogue* DialogueToStart = Provider->GetDialogueToProvide(PC);
    if (!DialogueToStart)
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Provider has no dialogue to provide!"));
        return false;
    }

    // DialogueController로 대화 시작
    // bool bSuccess = DialogueController->StartDialogue(
    //     DialogueToStart,
    //     Provider->GetOwner(),
    //     PC
    // );

    // if (!bSuccess)
    // {
    //     UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Failed to start dialogue!"));
    //     return false;
    // }

    // 상태 업데이트
    bIsInDialogue = true;
    CurrentProvider = Provider;

    // Provider에게 알림
    Provider->OnDialogueStarted();

    // UI 생성
    CreateDialogueWidget();

    // 입력 모드 변경
    SetupInputMode(true);

    // 게임 일시정지
    if (bPauseGameDuringDialogue)
    {
        UGameplayStatics::SetGamePaused(GetWorld(), true);
    }

    // 델리게이트 브로드캐스트
    OnDialogueStarted.Broadcast(Provider);

    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Started dialogue with %s"), 
        *Provider->GetOwner()->GetName());

    return true;
}

void UDialogueManagerComponent::EndDialogue(bool bCompleted)
{
    if (!bIsInDialogue)
    {
        return;
    }

    // UI 제거
    RemoveDialogueWidget();

    // 입력 모드 복원
    SetupInputMode(false);

    // 게임 재개
    if (bPauseGameDuringDialogue)
    {
        UGameplayStatics::SetGamePaused(GetWorld(), false);
    }

    // Provider에게 알림
    if (CurrentProvider)
    {
        CurrentProvider->OnDialogueFinished(bCompleted);
        CurrentProvider = nullptr;
    }

    // 상태 업데이트
    bIsInDialogue = false;

    // 델리게이트 브로드캐스트
    OnDialogueEnded.Broadcast(bCompleted);

    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Ended dialogue (Completed: %s)"), 
        bCompleted ? TEXT("true") : TEXT("false"));
}

void UDialogueManagerComponent::CreateDialogueWidget()
{
    if (!DialogueWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("DialogueManager: DialogueWidgetClass not set!"));
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
        // NPC Actor 설정
        if (CurrentProvider)
        {
            DialogueWidget->NPCActor = CurrentProvider->GetOwner();
        }

        DialogueWidget->AddToViewport(100);

        UE_LOG(LogTemp, Log, TEXT("DialogueManager: Created dialogue widget"));
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
        // UI 전용 입력 모드
        FInputModeUIOnly InputMode;
        if (DialogueWidget)
        {
            InputMode.SetWidgetToFocus(DialogueWidget->TakeWidget());
        }
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = bShowMouseCursor;
    }
    else
    {
        // 게임 입력 모드로 복원
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
    }
}

void UDialogueManagerComponent::OnDialogueEndedInternal(bool bCompleted)
{
    EndDialogue(bCompleted);
}

