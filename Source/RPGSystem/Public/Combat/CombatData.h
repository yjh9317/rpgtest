
#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"
#include "CombatData.generated.h"

USTRUCT(BlueprintType)
struct FDamageInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer DamageTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<AActor> SourceActor = nullptr;

	/** Optional detailed damage classification (ex: Damage.Type.Slash, Damage.Type.Fire). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag DamageTypeTag;

	/** Additional poise damage used by stagger systems. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PoiseDamage = 0.f;

	/** Additional guard damage used by block/guard-break systems. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GuardBreakDamage = 0.f;

	/** Whether this hit can critically strike. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanCrit = false;

	/** Whether this hit should bypass temporary invulnerability checks. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bBypassInvulnerability = false;
};
