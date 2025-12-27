// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Crafting/CraftingComponent.h"
#include "Inventory/InventoryCoreComponent.h"
#include "Inventory/Crafting/Data/CraftingRecipe.h"

UCraftingComponent::UCraftingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCraftingComponent::BeginPlay()
{
	Super::BeginPlay();
	// 시작 시 인벤토리 컴포넌트 미리 찾아두기
	GetInventoryCore();
}

UInventoryCoreComponent* UCraftingComponent::GetInventoryCore() const
{
	if (!CachedInventoryCore)
	{
		// 소유자(플레이어)로부터 인벤토리 컴포넌트 찾기
		if (AActor* Owner = GetOwner())
		{
			// const_cast를 사용하는 이유는 이 함수가 논리적으로는 const(조회)이지만, 캐싱을 위해 멤버 변수를 수정하기 때문입니다.
			// 혹은 mutable 키워드를 사용하거나, BeginPlay에서만 할당해도 됩니다.
			auto MutableThis = const_cast<UCraftingComponent*>(this);
			MutableThis->CachedInventoryCore = Owner->FindComponentByClass<UInventoryCoreComponent>();
		}
	}
	return CachedInventoryCore;
}

bool UCraftingComponent::CanCraft(const UCraftingRecipe* Recipe, int32 Amount) const
{
	if (!Recipe || Amount <= 0) return false;

	UInventoryCoreComponent* Inventory = GetInventoryCore();
	if (!Inventory) return false;

	// 1. 재료 확인 (Ingredients)
	// 'Main' 인벤토리에서 재료를 찾는다고 가정합니다. 
	// 필요하다면 모든 인벤토리를 순회하거나 Recipe에 지정된 인벤토리를 쓸 수 있습니다.
	FGuid MainInventoryGuid = Inventory->FindInventoryGuid(FName("Main")); 
	if (!MainInventoryGuid.IsValid()) return false;

	for (const FCraftingIngredient& Ingredient : Recipe->Ingredients)
	{
		// SoftObjectPtr 로드 (동기 로드)
		const UItemDefinition* IngredientDef = Ingredient.ItemDef.LoadSynchronous();
		if (!IngredientDef) continue; // 재료 정의가 없으면 스킵 혹은 실패 처리

		int32 RequiredQty = Ingredient.Quantity * Amount;
		
		// 인벤토리에 해당 아이템이 몇 개 있는지 확인
		int32 CurrentQty = Inventory->CountItemByDef(MainInventoryGuid, IngredientDef);

		if (CurrentQty < RequiredQty)
		{
			// 재료 부족
			return false; 
		}
	}

	// 2. 공간 확인 (Outputs)
	// (간단하게 구현: 결과물이 들어갈 공간이 있는지 체크)
	// 더 정교하게 하려면 InventoryCore에 CanAddItem 함수를 활용해야 합니다.
	for (const FCraftingOutput& Output : Recipe->Outputs)
	{
		// 단순히 빈 슬롯이 있는지 혹은 같은 아이템 스택에 여유가 있는지 체크해야 함.
		// 여기서는 LootNewItem이 실패하면 아이템을 바닥에 떨구는 식의 정책이 있다면 true를 반환해도 됨.
		// 일단은 인벤토리가 꽉 찼는지만 검사
		if (Inventory->IsInventoryFull(MainInventoryGuid))
		{
			// *주의* : 겹쳐질 수 있는 아이템이라면 꽉 차있어도 들어갈 수 있음.
			// 정확한 검사를 위해서는 InventoryCore에 'SimulateAddItem' 같은 기능이 필요함.
			// 현재는 간단히 '빈 슬롯이 없으면 실패'로 처리하거나 항상 true로 하고 Loot 시 처리하도록 함.
			// return false; 
		}
	}

	return true;
}

void UCraftingComponent::CraftItem(const UCraftingRecipe* Recipe, int32 Amount)
{
	if (!CanCraft(Recipe, Amount))
	{
		OnCraftingFailed.Broadcast(FText::FromString(TEXT("재료가 부족하거나 인벤토리가 가득 찼습니다.")));
		return;
	}

	// 1. 재료 소모
	ConsumeIngredients(Recipe, Amount);

	// 2. 결과물 지급
	GrantResults(Recipe, Amount);

	// 3. 성공 알림
	OnCraftingSuccess.Broadcast(Recipe);
}

void UCraftingComponent::ConsumeIngredients(const UCraftingRecipe* Recipe, int32 Amount)
{
	UInventoryCoreComponent* Inventory = GetInventoryCore();
	FGuid MainInventoryGuid = Inventory->FindInventoryGuid(FName("Main"));

	for (const FCraftingIngredient& Ingredient : Recipe->Ingredients)
	{
		const UItemDefinition* IngredientDef = Ingredient.ItemDef.Get();
		if (!IngredientDef) continue;

		int32 TotalToRemove = Ingredient.Quantity * Amount;

		// 인벤토리에서 아이템 제거
		Inventory->RemoveItemByDef(MainInventoryGuid, IngredientDef, TotalToRemove);
	}
}

void UCraftingComponent::GrantResults(const UCraftingRecipe* Recipe, int32 Amount)
{
	UInventoryCoreComponent* Inventory = GetInventoryCore();

	for (const FCraftingOutput& Output : Recipe->Outputs)
	{
		const UItemDefinition* OutputDef = Output.ItemDef.LoadSynchronous();
		if (!OutputDef) continue;

		int32 TotalToAdd = Output.Quantity * Amount;

		// 인벤토리에 아이템 추가 (LootNewItem은 남은 수량을 반환)
		int32 Remaining = Inventory->LootNewItem(OutputDef, TotalToAdd);

		if (Remaining > 0)
		{
			// 인벤토리에 다 못 넣은 경우 (바닥에 드랍하는 로직 추가 가능)
			UE_LOG(LogTemp, Warning, TEXT("인벤토리 공간 부족으로 %d개의 아이템을 잃어버렸습니다."), Remaining);
		}
	}
}

void UCraftingComponent::SetCurrentStation(AActor* StationActor)
{
	CurrentStationActor = StationActor;
}
