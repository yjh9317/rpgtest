// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Data/Fragment/ItemFragment_Consumable.h"
#include "Item/Data/ItemInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Status/StatsComponent.h"

bool UItemFragment_Consumable::CanUse_Implementation(AActor* User) const
{
	return Super::CanUse_Implementation(User);
}

bool UItemFragment_Consumable::UseItem_Implementation(AActor* User)
{
	// 1. 효과 적용 (액터 스폰 없이 바로 컴포넌트에 접근)
	ApplyEffects(User);

	// 2. 사운드/이펙트 재생 (UGameplayStatics 사용)
	if (!UseSound.IsNull())
	{
		UGameplayStatics::PlaySoundAtLocation(User, UseSound.LoadSynchronous(), User->GetActorLocation());
	}

	// 3. true를 반환하여 아이템 수량 감소 (InventoryComponent가 처리)
	return bConsumeOnUse;
}

void UItemFragment_Consumable::OnItemUsed_Implementation(AActor* User)
{
	Super::OnItemUsed_Implementation(User);
}

void UItemFragment_Consumable::ApplyEffects(AActor* User)
{
	// 유저의 스탯 컴포넌트 찾기
	UStatsComponent* StatsComp = User->FindComponentByClass<UStatsComponent>();
	if (!StatsComp) return;

	for (const FConsumableEffect& Effect : Effects)
	{
		switch (Effect.EffectType)
		{
		case EConsumableEffectType::Instant:
			// 즉시 회복: 함수 직접 호출
			StatsComp->ModifyStatValue(Effect.StatTag, Effect.Value);
			break;

		case EConsumableEffectType::Duration:
			{
				// FGameplayTag::EmptyTag 대신 적절한 버프 ID 태그 생성 필요 (혹은 Effect 구조체에 BuffID 추가 권장)
				FGameplayTag BuffID = FGameplayTag::RequestGameplayTag(FName("Effect.Buff.Consumable")); 
                
				// AddTemporaryStatBuff 함수 사용
				StatsComp->AddTemporaryStatBuff(
					Effect.StatTag, 
					BuffID, 
					Effect.Value, 
					Effect.Duration, 
					EModifierSourceType::Flat // 기획에 따라 Percentage 등으로 변경 가능
				);
			}
			break;
		case EConsumableEffectType::OverTime:
			{
				// 별도의 타이머 로직이나 GameplayEffect(GAS)가 없다면,
				// 여기서는 간단하게 Duration 버프로 처리하거나 별도 로직 구현이 필요합니다.
				// StatsComponent에는 도트 데미지/힐을 자체 처리하는 함수는 보이지 않습니다.
			}
			break;
		}
	}
}

