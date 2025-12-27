// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "CraftingComponent.generated.h"

class UInventoryCoreComponent;
class UCraftingRecipe;
class UItemDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCraftingSuccess, const UCraftingRecipe*, Recipe);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCraftingFailed, FText, ErrorMessage);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RPGSYSTEM_API UCraftingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCraftingComponent();
protected:
	virtual void BeginPlay() override;

public:
	bool CanCraft(const UCraftingRecipe* Recipe, int32 Amount = 1) const;
	void CraftItem(const UCraftingRecipe* Recipe, int32 Amount = 1);
	void SetCurrentStation(AActor* StationActor);

protected:
	UInventoryCoreComponent* GetInventoryCore() const;
	void ConsumeIngredients(const UCraftingRecipe* Recipe, int32 Amount);
	void GrantResults(const UCraftingRecipe* Recipe, int32 Amount);

public:
	UPROPERTY(BlueprintAssignable, Category = "Crafting")
	FOnCraftingSuccess OnCraftingSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Crafting")
	FOnCraftingFailed OnCraftingFailed;

protected:
	UPROPERTY(Transient)
	TObjectPtr<AActor> CurrentStationActor;

	UPROPERTY(Transient)
	TObjectPtr<UInventoryCoreComponent> CachedInventoryCore;
};
