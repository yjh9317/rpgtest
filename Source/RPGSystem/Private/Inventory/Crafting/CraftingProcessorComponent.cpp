// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Crafting/CraftingProcessorComponent.h"
#include "Inventory/InventoryCoreComponent.h"
#include "Inventory/Crafting/Data/CraftingRecipe.h"

// Sets default values for this component's properties
UCraftingProcessorComponent::UCraftingProcessorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UCraftingProcessorComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsCrafting)
	{
		CurrentJob.TimeRemaining -= DeltaTime;

		float Progress = 1.0f - (CurrentJob.TimeRemaining / CurrentJob.TotalDuration);
		OnCraftingProgress.Broadcast(FMath::Clamp(Progress, 0.0f, 1.0f));

		if (CurrentJob.TimeRemaining <= 0.0f)
		{
			FinishCurrentJob();
		}
	}
}

bool UCraftingProcessorComponent::RequestCrafting(const UCraftingRecipe* Recipe,UInventoryCoreComponent* InputInventory)
{
	if (!Recipe || !InputInventory) return false;

	// 1. 재료 선 검사 (Old: HasPlayerEnoughGold 등과 유사)
	// 여기서는 실제로 소모하진 않고 체크만 합니다.
	// if (!CheckIngredients(Recipe, InputInventory)) return false;

	// 2. 작업 생성
	FCraftingJob NewJob;
	NewJob.Recipe = Recipe;
	NewJob.TotalDuration = Recipe->CraftingTime;
	NewJob.TimeRemaining = Recipe->CraftingTime;

	// 3. 인벤토리 참조 저장 (제작 완료 시 사용)
	TargetInventory = InputInventory;

	// 4. 대기열 추가
	JobQueue.Add(NewJob);

	// 5. 현재 쉬고 있다면 작업 시작
	if (!bIsCrafting)
	{
		ProcessNextJob();
	}

	return true;
}

void UCraftingProcessorComponent::ProcessNextJob()
{
	if (JobQueue.IsEmpty())
	{
		bIsCrafting = false;
		SetComponentTickEnabled(false);
		return;
	}

	// 큐에서 꺼내기
	CurrentJob = JobQueue[0];
	JobQueue.RemoveAt(0);

	bIsCrafting = true;
	SetComponentTickEnabled(true); // 타이머 시작

	OnJobStarted.Broadcast(CurrentJob);
}

void UCraftingProcessorComponent::CancelCurrentJob()
{
	if (!bIsCrafting)
	{
		return;
	}

	// 2. 제작 프로세스 중지
	bIsCrafting = false;
	SetComponentTickEnabled(false);

	// 3. 재료 환불 (Refund Ingredients)
	// TargetInventory가 유효하고, 레시피 정보가 있다면 재료를 돌려줍니다.
	if (TargetInventory && CurrentJob.Recipe)
	{
		for (const FCraftingIngredient& Ingredient : CurrentJob.Recipe->Ingredients)
		{
			// 재료 아이템 정의 로드 (Soft Pointer이므로 로드 필요)
			const UItemDefinition* IngredientDef = Ingredient.ItemDef.LoadSynchronous();
            
			if (IngredientDef)
			{
				TargetInventory->LootNewItem(IngredientDef, Ingredient.Quantity);
			}
		}
        
		UE_LOG(LogTemp, Log, TEXT("Crafting Cancelled: Ingredients Refunded."));
	}

	// 4. 현재 작업 데이터 초기화
	CurrentJob = FCraftingJob(); // 빈 상태로 리셋

	// 5. 취소 알림 방송 (UI에서 진행바 숨김 처리 등)
	OnJobCancelled.Broadcast();
}

float UCraftingProcessorComponent::GetCurrentProgress() const
{
	if (!bIsCrafting || CurrentJob.TotalDuration <= 0.0f)
	{
		return 0.0f;
	}

	float ElapsedTime = CurrentJob.TotalDuration - CurrentJob.TimeRemaining;
	float Progress = ElapsedTime / CurrentJob.TotalDuration;

	return FMath::Clamp(Progress, 0.0f, 1.0f);
}

void UCraftingProcessorComponent::FinishCurrentJob()
{
	if (ConsumeAndReward(CurrentJob.Recipe))
	{
		OnJobFinished.Broadcast(CurrentJob.Recipe);
	}
	else
	{
		// 인벤토리가 꽉 찼거나 재료가 중간에 사라짐 -> 실패 처리
		// (Old의 EventFailedToInitializeCraftingProcess 대응)
	}

	// 다음 작업 수행
	ProcessNextJob();
}

bool UCraftingProcessorComponent::ConsumeAndReward(const UCraftingRecipe* Recipe)
{
	if (!TargetInventory || !Recipe) return false;

	// TODO: 인벤토리 컴포넌트 함수를 사용하여 재료 제거 및 결과 아이템 추가
	// 예: TargetInventory->ConsumeItems(Recipe->Ingredients);
	// 예: TargetInventory->AddItem(Recipe->OutputItemDef);
    
	return true;
}

