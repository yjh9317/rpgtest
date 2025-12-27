// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Components/CombatComponent.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UCombatComponent::SetCurrentTarget(AActor* NewTarget)
{
}


float UCombatComponent::ApplyDamageToTarget(const FDamageInfo& DamageInfo)
{
	return 1.f;
}

float UCombatComponent::ApplyDamage(AActor* Target, const FDamageInfo& DamageInfo)
{
	return 1.f;
}

bool UCombatComponent::IsAlive() const
{
	return true;
}

void UCombatComponent::EnterCombat()
{
}

void UCombatComponent::LeaveCombat()
{
}

float UCombatComponent::CalculateFinalDamage(const FDamageInfo& DamageInfo, AActor* Target) const
{
	return 0.f;
}

void UCombatComponent::HandleDeath(AActor* Target)
{
}

