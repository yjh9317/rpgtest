// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Components/CombatComponentBase.h"
#include "Engine/DamageEvents.h"
#include "Status/StatsComponent.h"

UCombatComponentBase::UCombatComponentBase()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.5f;  // 전투 타임아웃만 체크하므로 느린 틱
}

void UCombatComponentBase::BeginPlay()
{
	Super::BeginPlay();

	// StatsComponent 캐싱
	if (AActor* Owner = GetOwner())
	{
		CachedStatsComp = Owner->FindComponentByClass<UStatsComponent>();
	}
}

void UCombatComponentBase::TickComponent(float DeltaTime, ELevelTick TickType, 
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bAutoLeaveCombat)
	{
		CheckCombatTimeout(DeltaTime);
	}
}

// === ICombatable 구현 ===
float UCombatComponentBase::GetCurrentHealth() const
{
	// TODO: StatsComponent 연동
	// if (UStatsComponent* Stats = GetStatsComponent())
	// {
	//     return Stats->GetStat(FGameplayTag::RequestGameplayTag("Stat.Health"));
	// }
	return 100.f;  // 임시
}

float UCombatComponentBase::GetMaxHealth() const
{
	// TODO: StatsComponent 연동
	return 100.f;  // 임시
}

float UCombatComponentBase::ReceiveDamage(const FDamageInfo& DamageInfo)
{
	if (bIsDead || bIsInvulnerable)
	{
		return 0.f;
	}

	// 최종 데미지 계산
	const float FinalDamage = CalculateFinalDamage(DamageInfo);

	if (FinalDamage <= 0.f)
	{
		return 0.f;
	}

	// HP 감소
	// TODO: StatsComponent 연동
	// if (UStatsComponent* Stats = GetStatsComponent())
	// {
	//     Stats->ModifyStat(FGameplayTag::RequestGameplayTag("Stat.Health"), -FinalDamage);
	// }

	// 전투 상태 진입
	if (DamageInfo.SourceActor.IsValid())
	{
		EnterCombat(DamageInfo.SourceActor.Get());
	}

	// 이벤트 발송
	OnDamageReceived.Broadcast(FinalDamage, DamageInfo.SourceActor.Get(), DamageInfo);

	// 후처리
	PostDamageReceived(FinalDamage, DamageInfo.SourceActor.Get());

	// 사망 체크
	if (GetCurrentHealth() <= 0.f)
	{
		HandleDeath(DamageInfo.SourceActor.Get());
	}

	return FinalDamage;
}

float UCombatComponentBase::ApplyDamage(AActor* Target, const FDamageInfo& DamageInfo)
{
	if (!Target) return 0.f;

	float AppliedDamage = 0.f;

	// ICombatable 구현체에 데미지 적용
	if (ICombatable* Combatable = Cast<ICombatable>(Target))
	{
		AppliedDamage = Combatable->ReceiveDamage(DamageInfo);
	}
	// CombatComponentBase를 직접 찾아서 적용
	else if (UCombatComponentBase* TargetCombat = Target->FindComponentByClass<UCombatComponentBase>())
	{
		AppliedDamage = TargetCombat->ReceiveDamage(DamageInfo);
	}
	// Unreal 기본 데미지 시스템
	else
	{
		AppliedDamage = Target->TakeDamage(DamageInfo.BaseDamage, FDamageEvent(), nullptr, GetOwner());
	}

	if (AppliedDamage > 0.f)
	{
		EnterCombat(Target);
		OnDamageDealt.Broadcast(AppliedDamage, Target, DamageInfo);
	}

	return AppliedDamage;
}

void UCombatComponentBase::EnterCombat(AActor* Opponent)
{
	LastCombatTime = GetWorld()->GetTimeSeconds();

	if (!bIsInCombat)
	{
		bIsInCombat = true;
		OnCombatStateChanged.Broadcast(true);
	}
}

void UCombatComponentBase::LeaveCombat()
{
	if (bIsInCombat)
	{
		bIsInCombat = false;
		OnCombatStateChanged.Broadcast(false);
	}
}

void UCombatComponentBase::SetInvulnerable(bool bNewInvulnerable)
{
	bIsInvulnerable = bNewInvulnerable;
}

float UCombatComponentBase::GetHealthPercent() const
{
	const float MaxHP = GetMaxHealth();
	if (MaxHP <= 0.f) return 0.f;
	return GetCurrentHealth() / MaxHP;
}

AActor* UCombatComponentBase::GetCurrentTarget() const
{
	return TargetActor;
}

// === 오버라이드 포인트 ===
float UCombatComponentBase::CalculateFinalDamage_Implementation(const FDamageInfo& DamageInfo)
{
	float Damage = DamageInfo.BaseDamage;

	// TODO: 방어력, 저항 계산
	// 기본은 원본 데미지 그대로

	return FMath::Max(0.f, Damage);
}

void UCombatComponentBase::PostDamageReceived_Implementation(float FinalDamage, AActor* Instigator)
{
	// 기본 구현: 아무것도 안 함
	// Player: 히트 이펙트, 카메라 흔들림
	// AI: 히트 리액션, 어그로 추가
}

void UCombatComponentBase::HandleDeath_Implementation(AActor* Killer)
{
	if (bIsDead) return;

	bIsDead = true;
	LeaveCombat();

	OnCombatDeath.Broadcast(Killer);
}

void UCombatComponentBase::CheckCombatTimeout(float DeltaTime)
{
	if (!bIsInCombat || bIsDead) return;

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastCombatTime >= CombatTimeout)
	{
		LeaveCombat();
	}
}

UStatsComponent* UCombatComponentBase::GetStatsComponent() const
{
	return CachedStatsComp.Get();
}




