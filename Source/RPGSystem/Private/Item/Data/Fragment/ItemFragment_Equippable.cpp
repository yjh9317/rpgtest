// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Data/Fragment/ItemFragment_Equippable.h"

bool UItemFragment_Equippable::CanUse_Implementation(AActor* User) const
{
	// 1. 부모의 쿨다운 체크를 먼저 수행
	if (Super::IsOnCooldown(User->GetWorld()))
	{
		return false;
	}
	
	// 2. 이 아이템의 스탯 요구 조건(StatsFragment)과 유저의 스탯(StatsComponent) 비교 (선택적)
	// const UItemFragment_Stats* StatsFragment = GetOwnerInstance()->FindFragmentByClass<UItemFragment_Stats>();
	// const UStatsComponent* UserStats = User->FindComponentByClass<UStatsComponent>();
	// if (StatsFragment && UserStats)
	// {
	//     if (!UserStats->MeetsRequirements(StatsFragment->GetRequirements()))
	//     {
	//         return false; // "레벨이 부족합니다"
	//     }
	// }
	return false;
}

bool UItemFragment_Equippable::UseItem_Implementation(AActor* User)
{
	if (!User)
	{
		return false;
	}

	// 1. User로부터 EquipmentComponent를 찾습니다.
	// UEquipmentComponent* EquipmentComp = User->FindComponentByClass<UEquipmentComponent>();
	// if (!EquipmentComp)
	// {
	//     return false; 
	// }

	// 2. EquipmentComponent에 이 아이템 인스턴스와 슬롯 태그를 전달하여 장착을 요청합니다.
	// bool bEquipSuccess = EquipmentComp->EquipItem(GetOwnerInstance(), EquipmentSlotTag);

	// 3. 장착에 성공했다면, 공통 후처리(쿨다운 갱신 등)를 호출합니다.
	// if (bEquipSuccess)
	// {
	//     Super::OnItemUsed_Implementation(User); // 쿨다운 타이머 시작
	//     return true;
	// }

	return false; // 장착 실패 (예: 슬롯이 이미 찼음)
}

void UItemFragment_Equippable::OnItemUsed_Implementation(AActor* User)
{
	// 장착은 아이템을 소모(Consume)하지 않습니다.
	// 따라서 수량 감소 로직을 실행하지 않습니다.
    
	// 하지만 쿨다운 타이머 갱신 등 부모의 공통 로직은 필요할 수 있습니다.
	// UItemFragment_Usable::OnItemUsed_Implementation(User);
    
	// 만약 Usable의 기본 OnItemUsed가 수량 감소를 포함한다면,
	// 이 함수를 아예 비워두거나, 수량 감소를 제외한 쿨다운 갱신 로직만 별도로 호출해야 합니다.
}
