// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Data/Fragment/ItemFragment_Stats.h"

#include "Net/UnrealNetwork.h"

void UItemFragment_Stats::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UItemFragment_Stats, Stats);
}

void UItemFragment_Stats::AddOrUpdateStat(FGameplayTag StatTag, float AddValue)
{
	for (FItemStat& Stat : Stats)
	{
		if (Stat.StatTag == StatTag)
		{
			Stat.Value += AddValue;
			return;
		}
	}
	FItemStat NewStat;
	NewStat.StatTag = StatTag;
	NewStat.Value = AddValue;
	Stats.Add(NewStat);	
}
