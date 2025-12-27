// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Item/Data/Fragment/ItemFragment.h"
#include "ItemFragment_Stats.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FItemStat
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stat", meta = (Categories = "Character.Stat"))
	FGameplayTag StatTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stat")
	float Value = 0.0f;
};

UCLASS(meta = (DisplayName = "Stats"))
class RPGSYSTEM_API UItemFragment_Stats : public UItemFragment
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	TArray<FItemStat> Stats;

public:
	/** 스탯 값 변경 또는 추가 (업그레이드용) */
	void AddOrUpdateStat(FGameplayTag StatTag, float AddValue);
	
	/** 특정 태그의 스탯 값 가져오기 */
	UFUNCTION(BlueprintPure, Category = "Item Stats")
	float GetStatValue(const FGameplayTag& StatTag) const
	{
		for (const FItemStat& Stat : Stats)
		{
			if (Stat.StatTag == StatTag)
			{
				return Stat.Value;
			}
		}
		return 0.0f;
	}

	/** 특정 태그의 스탯이 있는지 확인 */
	UFUNCTION(BlueprintPure, Category = "Item Stats")
	bool HasStat(const FGameplayTag& StatTag) const
	{
		for (const FItemStat& Stat : Stats)
		{
			if (Stat.StatTag == StatTag)
			{
				return true;
			}
		}
		return false;
	}
};
