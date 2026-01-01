// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Action/Actions/MeleeAttackAction.h"

#include "Command/Components/CommandBufferComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"


UMeleeAttackAction::UMeleeAttackAction(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    ActionName = FText::FromString(TEXT("Melee Attack"));
    bCanExecuteWhileMoving = false;
    bCanExecuteInAir = false;
    bInterruptible = true;
    bMovementCancelable = true;

    BaseStaminaCost = 10.0f;
    CurrentComboIndex = 0;
}

void UMeleeAttackAction::Initialize(AActor* NewActionOwner, UObject* NewSourceObject)
{
    Super::Initialize(NewActionOwner, NewSourceObject);
    if (OwnerCharacter && OwnerCharacter->GetController())
    {
        // if (IComponentManager* ComponentManager = Cast<IComponentManager>(OwnerCharacter->GetController()))
        // {
        //     CachedCommandBuffer = ComponentManager->GetInputBufferComponent();
        // }
    }
}

bool UMeleeAttackAction::CanExecute() const
{
    return Super::CanExecute();
}

void UMeleeAttackAction::OnExecute_Implementation()
{
    if (!ValidateExecution())
    {
        OnComplete();
        return;
    }

    LastAttackTime = GetWorld()->GetTimeSeconds();
    PerformAttack();
}

void UMeleeAttackAction::OnComplete_Implementation()
{
    // 콤보 이어갈 수 있으면 바로 끝내지 않도록 하는 로직은
    // 필요에 따라 유지/삭제
    if (bComboWindowOpen && CurrentComboIndex < MeleeData.ComboMontages.Num() - 1)
    {
        return;
    }

    ResetCombo();
    Super::OnComplete();
}

void UMeleeAttackAction::OnInterrupt_Implementation()
{
    ResetCombo();
    Super::OnInterrupt();
}

bool UMeleeAttackAction::TryProcessComboInput()
{
    if (!GetWorld()) return false;

    if (!bComboWindowOpen || CurrentComboIndex >= MeleeData.ComboMontages.Num() - 1)
    {
        return false;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    LastAttackTime = CurrentTime;
    CurrentComboIndex++;
    PerformAttack();

    return true;
}

void UMeleeAttackAction::OnComboWindowClosed()
{
    if (CachedCommandBuffer && CachedCommandBuffer->HasBufferedAction(ActionTag))
    {
        CurrentComboIndex++;
        LastAttackTime = GetWorld()->GetTimeSeconds();
        PerformAttack();
        CachedCommandBuffer->ConsumeBufferedCommand(ActionTag);
    }
}

void UMeleeAttackAction::CancelToMovement(const FVector& Direction)
{
    if (!bMovementCancelable || !OwnerCharacter)
    {
        return;
    }

    if (UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance())
    {
        AnimInstance->Montage_Stop(BlendOutTime, CurrentAnimMontage);
    }

    if (UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement())
    {
        Movement->SetMovementMode(MOVE_Walking);
    }

    if (!Direction.IsNearlyZero())
    {
        OwnerCharacter->AddMovementInput(Direction);
    }

    // 이동 캔슬로 액션 종료
    Complete();
}

void UMeleeAttackAction::OnAttackMontageEnded()
{
    if (!CachedCommandBuffer || !CachedCommandBuffer->HasBufferedAction(ActionTag))
    {
        ResetCombo();
        OnComplete();
    }
}

void UMeleeAttackAction::PerformAttack()
{
    const float StaminaCost = GetStaminaCostForCombo();
    // TODO: BaseStaminaCost와 연계해서 HasAttackResources/ConsumeAttackResources 커스터마이즈 가능

    if (!PlayAttackMontage())
    {
        OnComplete();
        return;
    }

    CurrentAnimMontage = MeleeData.ComboMontages[CurrentComboIndex];

    AlreadyHitActors.Empty();
    bComboWindowOpen = false;
    bMovementCancelable = false;
}

bool UMeleeAttackAction::PlayAttackMontage()
{
    if (!OwnerCharacter || MeleeData.ComboMontages.Num() == 0)
    {
        return false;
    }

    int32 MontageIndex = FMath::Clamp(CurrentComboIndex, 0, MeleeData.ComboMontages.Num() - 1);
    UAnimMontage* AttackMontage = MeleeData.ComboMontages[MontageIndex];
    if (!AttackMontage)
    {
        return false;
    }

    UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (!AnimInstance)
    {
        return false;
    }

    float PlayResult = AnimInstance->Montage_Play(AttackMontage, 1.0f);
    return PlayResult > 0.0f;
}

void UMeleeAttackAction::ResetCombo()
{
    CurrentComboIndex = 0;
    bComboWindowOpen = false;
    bMovementCancelable = false;
    CurrentAnimMontage = nullptr;
    AlreadyHitActors.Empty();
}

bool UMeleeAttackAction::ProcessInput()
{
    return TryProcessComboInput();
}

bool UMeleeAttackAction::ValidateExecution() const
{
    return OwnerCharacter &&
           OwnerCharacter->GetMesh() &&
           OwnerCharacter->GetMesh()->GetAnimInstance() &&
           MeleeData.IsValid();
}

float UMeleeAttackAction::GetStaminaCostForCombo() const
{
    float ComboMultiplier = 1.0f + (CurrentComboIndex * 0.2f);
    return BaseStaminaCost * ComboMultiplier;
}

