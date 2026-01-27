// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Components/PlayerCombatComponent.h"

UPlayerCombatComponent::UPlayerCombatComponent()
{
	// Player는 기본적으로 자동 전투 이탈 비활성화
	bAutoLeaveCombat = false;
}

void UPlayerCombatComponent::PostDamageReceived_Implementation(float FinalDamage, AActor* Instigator)
{
	Super::PostDamageReceived_Implementation(FinalDamage, Instigator);

	LastDamageTime = GetWorld()->GetTimeSeconds();

	// 히트 피드백
	ApplyHitFeedback(FinalDamage);

	// 무적 프레임 시작
	if (InvincibilityDuration > 0.f)
	{
		TriggerInvincibility();
	}

	// 자동 회복 중단 (피격 시)
	if (bEnableAutoRecovery)
	{
		StopAutoRecovery();
	}
}

void UPlayerCombatComponent::HandleDeath_Implementation(AActor* Killer)
{
	// 무적/회복 타이머 정리
	GetWorld()->GetTimerManager().ClearTimer(InvincibilityTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(AutoRecoveryTimerHandle);

	Super::HandleDeath_Implementation(Killer);
}

void UPlayerCombatComponent::TriggerInvincibility(float Duration)
{
	const float ActualDuration = Duration > 0.f ? Duration : InvincibilityDuration;

	SetInvulnerable(true);

	GetWorld()->GetTimerManager().SetTimer(
		InvincibilityTimerHandle,
		this,
		&UPlayerCombatComponent::OnInvincibilityEnd,
		ActualDuration,
		false
	);
}

bool UPlayerCombatComponent::IsInvincibilityActive() const
{
	return GetWorld()->GetTimerManager().IsTimerActive(InvincibilityTimerHandle);
}

void UPlayerCombatComponent::StartAutoRecovery()
{
	if (!bEnableAutoRecovery) return;

	GetWorld()->GetTimerManager().SetTimer(
		AutoRecoveryTimerHandle,
		this,
		&UPlayerCombatComponent::PerformAutoRecovery,
		0.1f,  // 0.1초마다
		true,
		AutoRecoveryDelay  // 첫 실행 전 대기
	);
}

void UPlayerCombatComponent::StopAutoRecovery()
{
	GetWorld()->GetTimerManager().ClearTimer(AutoRecoveryTimerHandle);
}

void UPlayerCombatComponent::ApplyHitFeedback(float Damage)
{
	APlayerController* PC = Cast<APlayerController>(
		Cast<APawn>(GetOwner())->GetController());

	if (!PC) return;

	// 카메라 셰이크
	if (HitCameraShake)
	{
		const float ShakeScale = FMath::Clamp(Damage / 50.f, 0.5f, 2.f);
		PC->ClientStartCameraShake(HitCameraShake, ShakeScale);
	}

	// 컨트롤러 진동
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
	if (bIsDead) return;

	const float CurrentHP = GetCurrentHealth();
	const float MaxHP = GetMaxHealth();

	if (CurrentHP >= MaxHP)
	{
		StopAutoRecovery();
		return;
	}

	// HP 회복 (0.1초당 회복량)
	const float RecoverAmount = AutoRecoveryRate * 0.1f;

	// TODO: StatsComponent 연동
	// if (UStatsComponent* Stats = GetStatsComponent())
	// {
	//     Stats->ModifyStat(FGameplayTag::RequestGameplayTag("Stat.Health"), RecoverAmount);
	// }
}