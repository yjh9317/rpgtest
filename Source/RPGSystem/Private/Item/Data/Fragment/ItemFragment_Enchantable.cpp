// ItemFragment_Enchantable.cpp
#include "Item/Data/Fragment/ItemFragment_Enchantable.h"
#include "Net/UnrealNetwork.h"

void UItemFragment_Enchantable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UItemFragment_Enchantable, ActiveEnchantments);
}

bool UItemFragment_Enchantable::AddEnchantment(const FEnchantmentData& NewEnchantment)
{
	// 슬롯이 가득 찼는지 확인
	if (ActiveEnchantments.Num() >= MaxEnchantmentSlots)
	{
		return false;
	}

	// 이미 같은 인챈트가 있는지 확인
	if (HasEnchantment(NewEnchantment.EnchantmentTag))
	{
		return false;
	}

	// 허용된 타입인지 확인
	if (AllowedEnchantmentTypes.Num() > 0 && !AllowedEnchantmentTypes.HasTag(NewEnchantment.EnchantmentTag))
	{
		return false;
	}

	ActiveEnchantments.Add(NewEnchantment);
	return true;
}

bool UItemFragment_Enchantable::RemoveEnchantment(const FGameplayTag& EnchantmentTag)
{
	for (int32 i = 0; i < ActiveEnchantments.Num(); ++i)
	{
		if (ActiveEnchantments[i].EnchantmentTag == EnchantmentTag)
		{
			ActiveEnchantments.RemoveAt(i);
			return true;
		}
	}
	return false;
}

bool UItemFragment_Enchantable::HasEnchantment(const FGameplayTag& EnchantmentTag) const
{
	for (const FEnchantmentData& Enchantment : ActiveEnchantments)
	{
		if (Enchantment.EnchantmentTag == EnchantmentTag)
		{
			return true;
		}
	}
	return false;
}

bool UItemFragment_Enchantable::IsEnchantmentSlotsFull() const
{
	return ActiveEnchantments.Num() >= MaxEnchantmentSlots;
}