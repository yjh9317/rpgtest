// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Components/EnemyCombatComponent.h"

UEnemyCombatComponent::UEnemyCombatComponent()
{
	bAutoLeaveCombat = true;
	CombatTimeout = 8.f;  // Enemy는 좀 더 오래 전투 상태 유지
}

void UEnemyCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// TargetingComponent 캐싱 (Controller 또는 Pawn에서)
	if (AActor* Owner = GetOwner())
	{
		// CachedTargetingComp = Owner->FindComponentByClass<UTargetingComponent>();
		//
		// if (!CachedTargetingComp.IsValid())
		// {
		// 	if (APawn* Pawn = Cast<APawn>(Owner))
		// 	{
		// 		if (AController* Controller = Pawn->GetController())
		// 		{
		// 			CachedTargetingComp = Controller->FindComponentByClass<UTargetingComponent>();
		// 		}
		// 	}
		// }
	}
}

void UEnemyCombatComponent::PostDamageReceived_Implementation(float FinalDamage, AActor* Instigator)
{
	Super::PostDamageReceived_Implementation(FinalDamage, Instigator);

	// 어그로 추가
	if (Instigator)
	{
		AddAggroFromDamage(Instigator, FinalDamage);
	}

	// 히트 리액션
	if (bEnableHitReaction)
	{
		TriggerHitReaction(FinalDamage);
	}

	// 체력바 표시 갱신
	if (bShowHealthBar)
	{
		LastHealthBarShowTime = GetWorld()->GetTimeSeconds();
	}
}

void UEnemyCombatComponent::HandleDeath_Implementation(AActor* Killer)
{
	// 경험치 지급
	if (Killer)
	{
		GrantExperience(Killer);
	}

	// 드랍 아이템 생성
	SpawnDrops();

	Super::HandleDeath_Implementation(Killer);
}

bool UEnemyCombatComponent::ShouldShowHealthBar() const
{
	if (!bShowHealthBar) return false;
	if (bIsDead) return false;

	const float TimeSinceShown = GetWorld()->GetTimeSeconds() - LastHealthBarShowTime;
	return TimeSinceShown < HealthBarVisibleDuration;
}

void UEnemyCombatComponent::AddAggroFromDamage(AActor* Instigator, float Damage)
{
	// if (!CachedTargetingComp.IsValid()) return;
	//
	// const float AggroAmount = Damage * AggroOnDamageMultiplier;
	// CachedTargetingComp->AddAggro(Instigator, AggroAmount);
}

void UEnemyCombatComponent::TriggerHitReaction(float Damage)
{
	if (Damage >= StaggerThreshold)
	{
		OnStagger.Broadcast();
	}
	else if (Damage >= HitReactionThreshold)
	{
		OnHitReaction.Broadcast(Damage);
	}
}

void UEnemyCombatComponent::SpawnDrops()
{
	if (!DropTableClass) return;

	// TODO: 드랍 테이블 시스템 연동
	// FVector SpawnLocation = GetOwner()->GetActorLocation();
	// GetWorld()->SpawnActor<AActor>(DropTableClass, SpawnLocation, FRotator::ZeroRotator);
}

void UEnemyCombatComponent::GrantExperience(AActor* Killer)
{
	if (ExperienceReward <= 0) return;

	// TODO: 경험치 시스템 연동
	// Killer의 ExperienceComponent나 PlayerState에 경험치 추가
	UE_LOG(LogTemp, Log, TEXT("Granting %d XP to %s"), ExperienceReward, *GetNameSafe(Killer));
}