// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InventoryInitializer.generated.h"

class UInventoryCoreComponent;
/**
 * 
 */
UCLASS(Blueprintable, EditInlineNew)
class RPGSYSTEM_API UInventoryInitializer : public UObject
{
	GENERATED_BODY()
public:
	/** 초기화 진입점 */
	void Initialize(UInventoryCoreComponent* InOwner);

private:
	UPROPERTY()
	TObjectPtr<UInventoryCoreComponent> Owner;

	void CreateInventoriesFromConfigTable();
	void CreateInventoriesFromAutoList();
	void LoadItemsFromDataTables();
	void AddSingleInitialItems();
	void GenerateRandomLoot();
	void SortDefaultInventoryIfNeeded();
};
