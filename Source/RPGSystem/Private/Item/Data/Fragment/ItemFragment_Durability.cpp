// ItemFragment_Durability.cpp
#include "Item/Data/Fragment/ItemFragment_Durability.h"
#include "Net/UnrealNetwork.h"

void UItemFragment_Durability::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UItemFragment_Durability, CurrentDurability);
}

void UItemFragment_Durability::OnInstanced(UItemInstance* Instance)
{
	Super::OnInstanced(Instance);

	// 런타임 변수를 정적 설정 값으로 초기화
	CurrentDurability = MaxDurability;
}

void UItemFragment_Durability::DecreaseDurability(float Amount)
{
	CurrentDurability = FMath::Max(0.0f, CurrentDurability - Amount);
}

void UItemFragment_Durability::RepairDurability(float Amount)
{
	CurrentDurability = FMath::Min(MaxDurability, CurrentDurability + Amount);
}

float UItemFragment_Durability::GetDurabilityPercent() const
{
	return MaxDurability > 0.0f ? (CurrentDurability / MaxDurability) : 0.0f;
}

bool UItemFragment_Durability::IsBroken() const
{
	return bBreakOnZero && CurrentDurability <= 0.0f;
}