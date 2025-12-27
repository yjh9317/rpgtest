#pragma once

#include "CoreMinimal.h"
#include "ItemDefinition.h"
#include "ItemInitializationData.generated.h"


USTRUCT(BlueprintType)
struct FItemInitEntry : public FTableRowBase
{
	GENERATED_BODY()
    
	// ItemDefinition Asset 참조
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UItemDefinition> ItemDefinition;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity = 1;
	
	UPROPERTY(EditAnywhere)
	FName TargetInventoryName; 
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SlotIndex = -1; // -1이면 자동 배치
};

USTRUCT(BlueprintType)
struct FLootEntry
{
	GENERATED_BODY()
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UItemDefinition> ItemDefinition;
    
	// 드랍 확률 (0-100)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "100"))
	float DropChance = 50.0f;
    
	// 드랍 시 수량 범위
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinQuantity = 1;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxQuantity = 1;
};



USTRUCT(BlueprintType)
struct FRandomizedLootTable
{
	GENERATED_BODY()
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FLootEntry> LootEntries;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinLootItems = 1;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxLootItems = 3;
};

USTRUCT(BlueprintType)
struct FItemStack
{
	GENERATED_BODY()
    
	// ItemDefinition 직접 참조로 변경
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UItemDefinition> ItemDefinition;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity = 1;
};