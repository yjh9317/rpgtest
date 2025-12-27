// Fill out your copyright notice in the Description page of Project Settings.


#include "HitBox/HitboxInterface.h"
#include "GameplayTagContainer.h"

// Add default functionality here for any IHitboxInterface functions that are not pure virtual.
void IHitboxInterface::OnHitAsVictim(float Damage, AActor* Attacker, AController* Instigator, const FHitResult& Hit,
	UDamageType* DamageType, FGameplayTagContainer GameplayTags)
{
}

void IHitboxInterface::OnHitAsAttacker(AActor* Victim, const FHitResult& Hit, UDamageType* DamageType,
	FGameplayTagContainer GameplayTags)
{
}
