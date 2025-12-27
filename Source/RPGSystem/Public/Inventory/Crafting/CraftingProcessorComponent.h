// CraftingProcessorComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/CraftingJob.h"
#include "CraftingProcessorComponent.generated.h"

class UCraftingRecipe;
class UInventoryCoreComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCraftingProgress, float ProgressPercent);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnJobStarted, const FCraftingJob& Job);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnJobFinished, const UCraftingRecipe* Recipe);
DECLARE_MULTICAST_DELEGATE(FOnJobCancelled);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPGSYSTEM_API UCraftingProcessorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCraftingProcessorComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	bool RequestCrafting(const UCraftingRecipe* Recipe, UInventoryCoreComponent* InputInventory);
	void CancelCurrentJob();
	float GetCurrentProgress() const;
	
public:
	FOnCraftingProgress OnCraftingProgress;
	FOnJobStarted OnJobStarted;
	FOnJobFinished OnJobFinished;
	FOnJobCancelled OnJobCancelled;

protected:
	void ProcessNextJob();
	void FinishCurrentJob();
	bool ConsumeAndReward(const UCraftingRecipe* Recipe);

protected:
	UPROPERTY(VisibleAnywhere, Category = "State")
	FCraftingJob CurrentJob;

	UPROPERTY(VisibleAnywhere, Category = "State")
	TArray<FCraftingJob> JobQueue;

	UPROPERTY()
	TObjectPtr<UInventoryCoreComponent> TargetInventory;

	bool bIsCrafting = false;
};