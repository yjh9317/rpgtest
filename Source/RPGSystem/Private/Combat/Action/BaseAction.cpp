// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Action/BaseAction.h"
#include "Combat/Action/ActionOwner.h"
#include "Combat/Action/Components/ActionComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"


UBaseAction::UBaseAction(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // 기본값 설정
    bCanExecuteWhileMoving = true;
    bCanExecuteInAir = false;
    CooldownDuration = 0.0f;
}


void UBaseAction::Initialize(AActor* NewActionOwner, UObject* NewSourceObject)
{
    OwnerCharacter = Cast<ACharacter>(NewActionOwner);
    SourceObject = NewSourceObject;

    if (bAutoDisableGestureForNonPlayerControlled)
    {
        bool bIsPlayerControlled = false;

        if (const APawn* OwnerPawn = Cast<APawn>(NewActionOwner))
        {
            bIsPlayerControlled = OwnerPawn->IsPlayerControlled();
        }
        else if (const AController* OwnerController = Cast<AController>(NewActionOwner))
        {
            bIsPlayerControlled = OwnerController->IsPlayerController();
        }

        if (!bIsPlayerControlled)
        {
            bEnableGestureDetection = false;
            bEnableDoubleClick = false;
        }
    }

    if (IActionOwner* Interface = Cast<IActionOwner>(NewActionOwner))
    {
        CachedActionComponent = Interface->GetActionComponent();
    }

    if (!CachedActionComponent && NewActionOwner)
    {
        CachedActionComponent = NewActionOwner->FindComponentByClass<UActionComponent>();
    }
    OnInitialized();
}

void UBaseAction::Execute()
{
    if (!CanExecute())
        return;

    bIsActive = true;
    ActivationTime = GetWorld()->GetTimeSeconds(); 
    LastExecutionTime = ActivationTime;

    ConsumeResources();
    ApplyGrantedTags();

    OnExecute();
}

bool UBaseAction::HandleInput(EActionInputPhase InputPhase, float InputValue)
{
    TrackInputGesture(InputPhase, InputValue);

    if (!bIsActive)
    {
        return false;
    }

    if (InputPhase == EActionInputPhase::Pressed)
    {
        return ProcessInput();
    }

    return false;
}

float UBaseAction::GetCurrentInputHoldTime() const
{
    if (!bInputPressed)
    {
        return 0.0f;
    }

    const float CurrentTime = GetWorldTimeSafe();
    return FMath::Max(0.0f, CurrentTime - InputPressedTime);
}

void UBaseAction::Tick(float DeltaTime)
{
    if (!bIsActive)
        return;

    OnTick(DeltaTime);
}

void UBaseAction::Interrupt()
{
    if (!bIsActive)
        return;

    OnInterrupt(); // 내부 로직(몽타주 정지 등) 처리

    EndAction(EActionEndReason::Interrupted);
}

void UBaseAction::Complete()
{
    if (!bIsActive)
        return;

    OnComplete();
    EndAction(EActionEndReason::Completed);
}

bool UBaseAction::CanExecute() const
{
    if (bIsActive)
    {
        // DebugHeader::LogWarning(TEXT("UBaseAction::CanExecute() : Active"));
        return false;
    }

    if (!HasRequiredResources())
    {
        // DebugHeader::LogWarning(TEXT("UBaseAction::CanExecute() : Resource"));
        return false;
    }

    // 이동 중 실행 체크
    if (!bCanExecuteWhileMoving && OwnerCharacter && OwnerCharacter->GetVelocity().Size() > 0.1f)
    {
        // DebugHeader::LogWarning(TEXT("UBaseAction::CanExecute() : Moving"));
        return false;
    }

    // 공중 실행 체크
    if (!bCanExecuteInAir && OwnerCharacter && OwnerCharacter->GetCharacterMovement()->IsFalling())
    {
        // DebugHeader::LogWarning(TEXT("UBaseAction::CanExecute() : Air"));
        return false;
    }

    return true;
}

bool UBaseAction::HasRequiredResources() const
{
    // TODO: 속성 시스템과 연동하여 리소스 체크
    // 현재는 단순 구현
    for (const FActionCost& Cost : ResourceCosts)
    {
        // 실제 구현에서는 AttributeComponent를 통해 체크
        // float CurrentValue = GetAttributeValue(Cost.CostAttributeTag);
        // if (CurrentValue < Cost.CostValue)
        //     return false;
    }
    
    return true;
}


float UBaseAction::GetCooldownRemaining() const
{
    if (CooldownDuration <= 0.0f)
        return 0.0f;

    float CurrentTime = GetWorld()->GetTimeSeconds();
    float ElapsedTime = CurrentTime - LastExecutionTime;
    
    return FMath::Max(0.0f, CooldownDuration - ElapsedTime);
}


void UBaseAction::ConsumeResources()
{
    // TODO: 속성 시스템과 연동
    for (const FActionCost& Cost : ResourceCosts)
    {
        // GetAttributeComponent()->ModifyAttribute(Cost.CostAttributeTag, -Cost.CostValue);
        UE_LOG(LogTemp, Verbose, TEXT("Consuming %f of %s"), 
               Cost.CostValue, *Cost.CostStatTag.ToString());
    }
}

void UBaseAction::RefundResources(float Percentage)
{
    // TODO: 속성 시스템과 연동
    for (const FActionCost& Cost : ResourceCosts)
    {
        float RefundAmount = Cost.CostValue * Percentage;
        // GetAttributeComponent()->ModifyAttribute(Cost.CostAttributeTag, RefundAmount);
        UE_LOG(LogTemp, Verbose, TEXT("Refunding %f of %s"), 
               RefundAmount, *Cost.CostStatTag.ToString());
    }
}

void UBaseAction::ApplyGrantedTags()
{
    if (CachedActionComponent && GrantedTags.Num() > 0)
    {
        CachedActionComponent->AddTags(GrantedTags);
    }
}

void UBaseAction::RemoveGrantedTags()
{
    if (CachedActionComponent && GrantedTags.Num() > 0)
    {
        CachedActionComponent->RemoveTags(GrantedTags);
    }
}

void UBaseAction::StartCooldown()
{
    
}

void UBaseAction::EndAction(EActionEndReason EndReason)
{
    bIsActive = false;
    RemoveGrantedTags();

    if (EndReason == EActionEndReason::Completed || EndReason == EActionEndReason::Interrupted)
    {
        StartCooldown();
    }

    OnActionEnded.ExecuteIfBound(this, EndReason);
}

void UBaseAction::TrackInputGesture(EActionInputPhase InputPhase, float InputValue)
{
    const float CurrentTime = GetWorldTimeSafe();

    if (InputPhase == EActionInputPhase::Pressed)
    {
        if (!bInputPressed)
        {
            bInputPressed = true;
            bLongPressTriggered = false;
            InputPressedTime = CurrentTime;
            OnInputPressedEvent(InputValue);
        }
        return;
    }

    if (InputPhase == EActionInputPhase::Held)
    {
        if (!bInputPressed)
        {
            bInputPressed = true;
            bLongPressTriggered = false;
            InputPressedTime = CurrentTime;
            OnInputPressedEvent(InputValue);
        }

        const float HoldDuration = FMath::Max(0.0f, CurrentTime - InputPressedTime);
        OnInputHeldEvent(HoldDuration, InputValue);

        if (bEnableGestureDetection && !bLongPressTriggered && HoldDuration >= LongPressThreshold)
        {
            bLongPressTriggered = true;
            OnLongPressEvent(HoldDuration);
        }
        return;
    }

    if (InputPhase == EActionInputPhase::Released)
    {
        if (!bInputPressed)
        {
            return;
        }

        const float HoldDuration = FMath::Max(0.0f, CurrentTime - InputPressedTime);
        bInputPressed = false;

        OnInputReleasedEvent(HoldDuration, InputValue);

        if (!bEnableGestureDetection)
        {
            bLongPressTriggered = false;
            InputPressedTime = -1.0f;
            return;
        }

        const bool bIsShortPress = HoldDuration <= ShortPressThreshold;
        if (bIsShortPress)
        {
            OnShortPressEvent(HoldDuration);

            if (!bEnableDoubleClick)
            {
                OnSingleClickEvent();
            }
            else if (bPendingSingleClick)
            {
                ClearPendingSingleClick();
                OnDoubleClickEvent();
            }
            else
            {
                bPendingSingleClick = true;
                if (UWorld* World = GetWorld())
                {
                    World->GetTimerManager().SetTimer(
                        SingleClickTimerHandle,
                        this,
                        &UBaseAction::HandleSingleClickTimeout,
                        DoubleClickThreshold,
                        false
                    );
                }
                else
                {
                    bPendingSingleClick = false;
                    OnSingleClickEvent();
                }
            }
        }
        else if (!bLongPressTriggered && HoldDuration >= LongPressThreshold)
        {
            // If held callbacks were not received, detect long press on release as fallback.
            OnLongPressEvent(HoldDuration);
        }

        bLongPressTriggered = false;
        InputPressedTime = -1.0f;
    }
}

void UBaseAction::HandleSingleClickTimeout()
{
    if (!bPendingSingleClick)
    {
        return;
    }

    bPendingSingleClick = false;
    OnSingleClickEvent();
}

void UBaseAction::ClearPendingSingleClick()
{
    if (!bPendingSingleClick)
    {
        return;
    }

    bPendingSingleClick = false;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(SingleClickTimerHandle);
    }
}

float UBaseAction::GetWorldTimeSafe() const
{
    if (const UWorld* World = GetWorld())
    {
        return World->GetTimeSeconds();
    }

    return 0.0f;
}

APlayerController* UBaseAction::GetOwnerController() const
{
    if (OwnerCharacter)
    {
        return Cast<APlayerController>(OwnerCharacter->GetController());
    }
    return nullptr;
}
