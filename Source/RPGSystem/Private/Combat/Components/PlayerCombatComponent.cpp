// Fill out your copyright notice in the Description page of Project Settings.

#include "Combat/Components/PlayerCombatComponent.h"

#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Status/StatsComponent.h"

UPlayerCombatComponent::UPlayerCombatComponent()
{
	// Player keeps combat state manually in current design.
	bAutoLeaveCombat = false;
}

void UPlayerCombatComponent::PostDamageReceived_Implementation(float FinalDamage, AActor* Instigator)
{
	Super::PostDamageReceived_Implementation(FinalDamage, Instigator);

	if (UWorld* World = GetWorld())
	{
		LastDamageTime = World->GetTimeSeconds();
	}

	ApplyHitFeedback(FinalDamage);

	if (InvincibilityDuration > 0.f)
	{
		TriggerInvincibility();
	}

	if (bEnableAutoRecovery)
	{
		StopAutoRecovery();
		StartAutoRecovery();
	}
}

void UPlayerCombatComponent::HandleDeath_Implementation(AActor* Killer)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(InvincibilityTimerHandle);
		World->GetTimerManager().ClearTimer(AutoRecoveryTimerHandle);
	}

	Super::HandleDeath_Implementation(Killer);
}

void UPlayerCombatComponent::TriggerInvincibility(float Duration)
{
	const float ActualDuration = Duration > 0.f ? Duration : InvincibilityDuration;
	SetInvulnerable(true);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			InvincibilityTimerHandle,
			this,
			&UPlayerCombatComponent::OnInvincibilityEnd,
			ActualDuration,
			false
		);
	}
}

bool UPlayerCombatComponent::IsInvincibilityActive() const
{
	if (const UWorld* World = GetWorld())
	{
		return World->GetTimerManager().IsTimerActive(InvincibilityTimerHandle);
	}

	return false;
}

void UPlayerCombatComponent::StartAutoRecovery()
{
	if (!bEnableAutoRecovery)
	{
		return;
	}

	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			AutoRecoveryTimerHandle,
			this,
			&UPlayerCombatComponent::PerformAutoRecovery,
			0.1f,
			true,
			AutoRecoveryDelay
		);
	}
}

void UPlayerCombatComponent::StopAutoRecovery()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoRecoveryTimerHandle);
	}
}

void UPlayerCombatComponent::ApplyHitFeedback(float Damage)
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC)
	{
		return;
	}

	if (HitCameraShake)
	{
		const float ShakeScale = FMath::Clamp(Damage / 50.f, 0.5f, 2.f);
		PC->ClientStartCameraShake(HitCameraShake, ShakeScale);
	}

	if (ControllerVibrationIntensity > 0.f)
	{
		const float Intensity = FMath::Clamp(Damage / 100.f, 0.2f, 1.f) * ControllerVibrationIntensity;
		PC->PlayDynamicForceFeedback(
			Intensity, 0.2f, true, true, true, true,
			EDynamicForceFeedbackAction::Start
		);
	}
}

void UPlayerCombatComponent::OnInvincibilityEnd()
{
	SetInvulnerable(false);
}

void UPlayerCombatComponent::PerformAutoRecovery()
{
	if (bIsDead)
	{
		return;
	}

	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	const float CurrentHP = GetCurrentHealth();
	const float MaxHP = GetMaxHealth();
	if (CurrentHP >= MaxHP)
	{
		StopAutoRecovery();
		return;
	}

	const float RecoverAmount = AutoRecoveryRate * 0.1f;
	if (UStatsComponent* Stats = GetStatsComponent())
	{
		if (HealthStatTag.IsValid() && Stats->HasStat(HealthStatTag))
		{
			Stats->ModifyStatValue(HealthStatTag, RecoverAmount);
		}
	}
}
