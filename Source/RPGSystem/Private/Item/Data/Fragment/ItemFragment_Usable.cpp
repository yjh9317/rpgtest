// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Data/Fragment/ItemFragment_Usable.h"

void UItemFragment_Usable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

bool UItemFragment_Usable::IsOnCooldown(const UWorld* World) const
{
	return true;
}

float UItemFragment_Usable::GetRemainingCooldown(const UWorld* World) const
{
	return 0.f;
}
