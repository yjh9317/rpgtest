// ItemFragment_Durability.h
#pragma once

#include "CoreMinimal.h"
#include "Item/Data/Fragment/ItemFragment.h"
#include "ItemFragment_Durability.generated.h"

/**
 * 내구도 시스템 Fragment
 * 
 * - MaxDurability: Definition에 설정되는 정적 값
 * - CurrentDurability: Instance별로 관리되는 런타임 값 (replicated)
 * - bBreakOnZero: 내구도 0일 때 파괴 여부
 */
UCLASS(meta = (DisplayName = "Durability"))
class RPGSYSTEM_API UItemFragment_Durability : public UItemFragment
{
	GENERATED_BODY()
	
public:
	//~UObject interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of UObject interface

	//~UItemFragment interface
	virtual void OnInstanced(UItemInstance* Instance) override;
	//~End of UItemFragment interface

public:
	/** 최대 내구도 (정적 설정) */
	UPROPERTY(EditDefaultsOnly, Category = "Durability")
	float MaxDurability = 100.0f;

	/** 내구도 0일 때 아이템 파괴 여부 */
	UPROPERTY(EditDefaultsOnly, Category = "Durability")
	bool bBreakOnZero = true;

	/** 현재 내구도 (런타임 가변, 복제됨) */
	UPROPERTY(Replicated)
	float CurrentDurability = 0.0f;

public:
	void DecreaseDurability(float Amount);
	void RepairDurability(float Amount);
	float GetDurabilityPercent() const;
	bool IsBroken() const;
};