// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HitboxInterface.generated.h"

struct FGameplayTagContainer;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UHitboxInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RPGSYSTEM_API IHitboxInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void OnHitAsVictim(float Damage,AActor* Attacker,AController* Instigator,const FHitResult& Hit,UDamageType* DamageType,FGameplayTagContainer GameplayTags);
	virtual void OnHitAsAttacker(AActor* Victim,const FHitResult& Hit,UDamageType* DamageType,FGameplayTagContainer GameplayTags);
};
