// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RPGPlayerCharacter.h"
#include "GameFramework/PlayerController.h"
#include "SaveSystem/Interface/RPGActorSaveInterface.h"
#include "RPGPlayerController.generated.h"

class UPlayerInventoryWidget;
/**
 * 
 */
UCLASS()
class RPGSYSTEM_API ARPGPlayerController : public APlayerController, public IRPGActorSaveInterface
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UPlayerInventoryWidget> InventoryWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	TObjectPtr<UPlayerInventoryWidget> InventoryWidgetInstance;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class URPGHUDWidget> HUDWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	TObjectPtr<class URPGHUDWidget> HUDWidgetInstance;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> InventoryAction;
	
	UPROPERTY()
	TObjectPtr<class UStatsViewModel> StatsViewModel;
	
#pragma region SaveInterfaceFunc
	
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
	virtual void ActorLoaded_Implementation() override;
#pragma endregion SaveInterfaceFunc
	
#pragma region SaveFunc
	void StoreDefaultPawn();
	void PossessOtherPawn(APawn* OtherPawn);
	void PossessOriginalPawn();
	
#pragma endregion SaveFunc
	
#pragma region SaveData
	UPROPERTY(SaveGame)
	FRotator OtherPawnViewRot;
	
	UPROPERTY(SaveGame)
	TSoftObjectPtr<APawn> OtherPawnRef;
	
	UPROPERTY()
	TSoftObjectPtr<APawn> DefaultPawnRef;
#pragma endregion SaveData
public:
	void TryInteract();
	void ToggleInventory();
	
	void TestSaveGame();
	void TestLoadGame();

	UFUNCTION()
	void HandleSaveCompleted();
	UFUNCTION()
	void HandleSaveFailed();
	UFUNCTION()
	void HandleLoadCompleted();
};
