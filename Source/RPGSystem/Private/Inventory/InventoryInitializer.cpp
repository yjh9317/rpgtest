// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/InventoryInitializer.h"

#include "Inventory/InventoryCoreComponent.h"
#include "Item/Data/ItemDataStructure.h"

void UInventoryInitializer::Initialize(UInventoryCoreComponent* InOwner)
{
	Owner = InOwner;
	if (!Owner)
	{
		return;
	}

	// 1. 인벤토리 생성 단계
	CreateInventoriesFromConfigTable();
	CreateInventoriesFromAutoList();

	// 2. 아이템 지급 단계
	LoadItemsFromDataTables();
	AddSingleInitialItems();
	GenerateRandomLoot();

	// 3. 후처리 (정렬 등)
	SortDefaultInventoryIfNeeded();
}

void UInventoryInitializer::CreateInventoriesFromConfigTable()
{
	if (!Owner || !Owner->InventoryConfigTable)
	{
		return;
	}

	TArray<FInventoryCreateConfig*> Rows;
	Owner->InventoryConfigTable->GetAllRows<FInventoryCreateConfig>(TEXT("CreateInventoriesFromDataTable"), Rows);

	Rows.Sort([](const FInventoryCreateConfig& A, const FInventoryCreateConfig& B)
	{
		return A.CreationPriority < B.CreationPriority;
	});

	for (FInventoryCreateConfig* Row : Rows)
	{
		if (Row && Row->bAutoCreateFromTable)
		{
			const FGuid NewGuid = Owner->CreateInventory(*Row);
			UE_LOG(LogTemp, Log, TEXT("[Initializer] DataTable 인벤토리 생성: %s"), *Row->InventoryName.ToString());
		}
	}
}

void UInventoryInitializer::CreateInventoriesFromAutoList()
{
	if (!Owner) return;

	for (const FInventoryCreateConfig& Config : Owner->AutoCreateInventories)
	{
		Owner->CreateInventory(Config);
	}
}

void UInventoryInitializer::LoadItemsFromDataTables()
{
	if (!Owner) return;

	for (UDataTable* DataTable : Owner->InitialItemTables)
	{
		if (!DataTable) continue;

		TArray<FItemInitEntry*> Rows;
		DataTable->GetAllRows<FItemInitEntry>(TEXT("LoadItemsFromDataTables"), Rows);

		for (FItemInitEntry* Row : Rows)
		{
			if (!Row || !Row->ItemDefinition.IsValid()) continue;

			UItemDefinition* ItemDef = Row->ItemDefinition.LoadSynchronous();
			if (ItemDef)
			{
				// [변경] 타겟 인벤토리 이름이 명시되어 있다면 해당 인벤토리에 강제 추가
				if (!Row->TargetInventoryName.IsNone())
				{
					FGuid TargetGuid = Owner->FindInventoryGuid(Row->TargetInventoryName);
					if (TargetGuid.IsValid())
					{
						Owner->AddItemToInventory(TargetGuid, ItemDef, Row->Quantity);
					}
					else
					{
						// 타겟 이름을 찾을 수 없으면 스마트 루팅으로 대체
						Owner->LootNewItem(ItemDef, Row->Quantity);
					}
				}
				else
				{
					// [변경] 타겟이 없으면 스마트 루팅 (Main -> 기타 가방 순으로 자동 삽입)
					Owner->LootNewItem(ItemDef, Row->Quantity);
				}
			}
		}
	}
}

void UInventoryInitializer::AddSingleInitialItems()
{
	if (!Owner) return;

	for (const FItemStack& Stack : Owner->SingleInitialItems)
	{
		if (!Stack.ItemDefinition.IsValid()) continue;

		UItemDefinition* ItemDef = Stack.ItemDefinition.LoadSynchronous();
		if (ItemDef)
		{
			Owner->LootNewItem(ItemDef, Stack.Quantity);
		}
	}
}

void UInventoryInitializer::GenerateRandomLoot()
{
	if (!Owner) return;

	for (const FRandomizedLootTable& LootTable : Owner->RandomLootTables)
	{
		const int32 NumItems = FMath::RandRange(LootTable.MinLootItems, LootTable.MaxLootItems);

		for (int32 i = 0; i < NumItems; ++i)
		{
			if (LootTable.LootEntries.Num() == 0) break;

			const FLootEntry& Entry = LootTable.LootEntries[FMath::RandRange(0, LootTable.LootEntries.Num() - 1)];

			const float Roll = FMath::FRandRange(0.0f, 100.0f);
			if (Roll <= Entry.DropChance)
			{
				if (Entry.ItemDefinition.IsValid())
				{
					UItemDefinition* ItemDef = Entry.ItemDefinition.LoadSynchronous();
					if (ItemDef)
					{
						const int32 Quantity = FMath::RandRange(Entry.MinQuantity, Entry.MaxQuantity);
						Owner->LootNewItem(ItemDef, Quantity);
					}
				}
			}
		}
	}
}

void UInventoryInitializer::SortDefaultInventoryIfNeeded()
{
	if (!Owner || !Owner->bSortInitialItems) return;

	const FGuid DefaultGuid = Owner->FindInventoryGuid(Owner->DefaultInventoryForInitialItems);
	if (DefaultGuid.IsValid())
	{
		Owner->CompactInventory(DefaultGuid);
	}
}