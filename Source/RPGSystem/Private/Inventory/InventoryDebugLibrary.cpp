// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/InventoryDebugLibrary.h"

#include "Inventory/InventoryCoreComponent.h"
#include "Item/Data/ItemDataStructure.h"

void UInventoryDebugLibrary::LogAllInventories(const UObject* WorldContextObject, UInventoryCoreComponent* InventoryComp)
{
#if !UE_BUILD_SHIPPING // 배포 빌드 제외
	if (!InventoryComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Inventory Debug] InventoryComponent is Invalid."));
		return;
	}

	// [변경] GetAllInventoryEntries() 대신 기존 API인 GetAllInventoryGuids() 사용
	const TArray<FGuid> InventoryGuids = InventoryComp->GetAllInventoryGuids();

	UE_LOG(LogTemp, Log, TEXT("=========================================="));
	UE_LOG(LogTemp, Log, TEXT("       INVENTORY DEBUG LOG START          "));
	UE_LOG(LogTemp, Log, TEXT("=========================================="));
	UE_LOG(LogTemp, Log, TEXT("Total Inventories: %d"), InventoryGuids.Num());

	for (const FGuid& Guid : InventoryGuids)
	{
		// 각 인벤토리별로 상세 로그 출력 함수 재사용
		LogInventoryByGuid(InventoryComp, Guid);
	}
	
	UE_LOG(LogTemp, Log, TEXT("=========================================="));
	UE_LOG(LogTemp, Log, TEXT("        INVENTORY DEBUG LOG END           "));
	UE_LOG(LogTemp, Log, TEXT("=========================================="));
#endif
}

void UInventoryDebugLibrary::LogInventoryByGuid(UInventoryCoreComponent* InventoryComp, FGuid InventoryGuid)
{
#if !UE_BUILD_SHIPPING
	if (!InventoryComp) return;

	// 공개 API를 통해 데이터 접근
	const FInventoryMetaData* MetaData = InventoryComp->GetMetaData(InventoryGuid);
	const TArray<FInventorySlot>* Slots = InventoryComp->GetInventory(InventoryGuid);

	if (!MetaData || !Slots)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Inventory Debug] Inventory GUID %s not found."), *InventoryGuid.ToString());
		return;
	}

	// --- 헤더 정보 출력 ---
	FString HeaderLog = FString::Printf(TEXT(">>> Inventory: %s (Display: %s)"), 
		*MetaData->InventoryName.ToString(), 
		*MetaData->DisplayName.ToString());
	UE_LOG(LogTemp, Log, TEXT("%s"), *HeaderLog);

	// --- 무게 및 슬롯 정보 계산 ---
	int32 FilledSlots = 0;
	int32 TotalItemCount = 0;
	
	for (const FInventorySlot& Slot : *Slots)
	{
		if (!Slot.IsEmpty())
		{
			FilledSlots++;
			TotalItemCount += Slot.Quantity;
		}
	}

	FString StatsLog = FString::Printf(TEXT("    Slots: %d / %d Used | Total Items: %d"), 
		FilledSlots, MetaData->MaxSlots, TotalItemCount);

	if (MetaData->bUseWeight)
	{
		StatsLog += FString::Printf(TEXT(" | Weight: %.2f / %.2f"), 
			MetaData->CurrentWeight, MetaData->MaxWeight);
	}
	UE_LOG(LogTemp, Log, TEXT("%s"), *StatsLog);

	// --- 개별 아이템 목록 출력 ---
	if (FilledSlots > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("    [Contents]"));
		for (const FInventorySlot& Slot : *Slots)
		{
			if (!Slot.IsEmpty())
			{
				const UItemDefinition* Def = Slot.GetItemDefinition();
				FString ItemName = Def ? Def->ItemName.ToString() : TEXT("INVALID_ITEM_DEF");
				float ItemWeight = Def ? Def->GetWeight() * Slot.Quantity : 0.0f;

				// 예: [0] Health Potion (x5) - Weight: 2.5
				UE_LOG(LogTemp, Log, TEXT("      [%d] %s (x%d) - Weight: %.1f"), 
					Slot.SlotIndex, 
					*ItemName, 
					Slot.Quantity, 
					ItemWeight);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("    (Empty)"));
	}
	UE_LOG(LogTemp, Log, TEXT("------------------------------------------"));
#endif
}

void UInventoryDebugLibrary::LogInventoryByName(UInventoryCoreComponent* InventoryComp, FName InventoryName)
{
#if !UE_BUILD_SHIPPING
	if (!InventoryComp) return;

	FGuid Guid = InventoryComp->FindInventoryGuid(InventoryName);
	if (Guid.IsValid())
	{
		LogInventoryByGuid(InventoryComp, Guid);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Inventory Debug] Inventory Name '%s' not found."), *InventoryName.ToString());
	}
#endif
}

void UInventoryDebugLibrary::DrawDebugInventoryOnScreen(UInventoryCoreComponent* InventoryComp, float Duration)
{
#if !UE_BUILD_SHIPPING
	if (!InventoryComp || !GEngine) return;

	// [변경] GetAllInventoryEntries() 대신 GUID 목록으로 순회
	const TArray<FGuid> InventoryGuids = InventoryComp->GetAllInventoryGuids();
    
	FString DebugMessage = TEXT("=== [Inventory Debug] ===\n");
    
	for (const FGuid& Guid : InventoryGuids)
	{
		const FInventoryMetaData* MetaData = InventoryComp->GetMetaData(Guid);
		const TArray<FInventorySlot>* Slots = InventoryComp->GetInventory(Guid);

		if (!MetaData || !Slots) continue;

		int32 FilledSlots = 0;
		for(const FInventorySlot& Slot : *Slots)
		{
			if(!Slot.IsEmpty()) 
			{
				FilledSlots++;
			}
		}
        
		// 예: [Main] 5/30 Slots (Weight: 10.5/100.0)
		FString WeightInfo = MetaData->bUseWeight 
			? FString::Printf(TEXT("(W: %.1f/%.1f)"), MetaData->CurrentWeight, MetaData->MaxWeight) 
			: TEXT("");

		DebugMessage += FString::Printf(TEXT("[%s] %d/%d Slots %s\n"), 
			*MetaData->InventoryName.ToString(), 
			FilledSlots,
			MetaData->MaxSlots,
			*WeightInfo);
	}

	// 화면에 Cyan 색상으로 출력 (Key: -1로 설정하여 기존 메시지를 덮어쓰지 않고 추가)
	GEngine->AddOnScreenDebugMessage(-1, Duration, FColor::Cyan, DebugMessage);
#endif
}