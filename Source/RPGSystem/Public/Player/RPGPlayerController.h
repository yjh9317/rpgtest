// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RPGPlayerCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Interface/DialogueControllerInterface.h"
#include "SaveSystem/Interface/RPGActorSaveInterface.h"
#include "RPGPlayerController.generated.h"

class UDialogueManagerComponent;
class UDamageFloatManagerComponent;
class UPlayerInventoryWidget;
class UStatsViewModel;
/**
 * 
 */
UCLASS()
class RPGSYSTEM_API ARPGPlayerController :	public APlayerController, 
											public IRPGActorSaveInterface,
											public IDialogueControllerInterface
{
	GENERATED_BODY()
	
public:
	ARPGPlayerController();
	
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	
#pragma region DialogueInterface
public:
	// 인터페이스 구현
	virtual void StartDialogue_Implementation(UDialogue* Dialogue, AActor* NPCActor) override;
	virtual void EndDialogue_Implementation() override;
	virtual void DisplayNPCDialogue_Implementation(const FDialogueNode& Node, AActor* NPCActor) override;
	virtual void DisplayPlayerOptions_Implementation(const TArray<FDialogueNode>& Options) override;
	virtual void HideDialogueUI_Implementation() override;
	virtual void OnPlayerSelectOption_Implementation(int32 NodeId) override;
	virtual bool IsInDialogue_Implementation() const override;
	virtual bool CanStartDialogue_Implementation() const override;
    
	// BG3 스타일 기능들
	virtual void SwitchToDialogueCamera_Implementation(AActor* NPCActor) override;
	virtual void RestoreGameplayCamera_Implementation() override;
	virtual void SetDialogueInputMode_Implementation(bool bDialogueMode) override;
	virtual void FaceNPC_Implementation(AActor* NPCActor) override;
	virtual void SetCharacterMovementEnabled_Implementation(bool bEnabled) override;
	virtual void AddToDialogueHistory_Implementation(const FDialogueNode& Node, bool bIsPlayerChoice) override;
	virtual bool CanSkipDialogue_Implementation() const override;
#pragma endregion DialogueInterface
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UPlayerInventoryWidget> InventoryWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	TObjectPtr<UPlayerInventoryWidget> InventoryWidgetInstance;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class URPGHUDWidget> HUDWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	TObjectPtr<class URPGHUDWidget> HUDWidget;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> InventoryAction;
	
	UPROPERTY()
	TObjectPtr<UStatsViewModel> StatsViewModel;
	
	UPROPERTY()
	TObjectPtr<UDamageFloatManagerComponent> DamageFloatManagerComponent;
	
	UPROPERTY()
	TObjectPtr<UDialogueManagerComponent> DialogueManagerComponent;
	
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
	
	void ApplySavedInputSettings();

	UFUNCTION()
	void HandleSaveCompleted();
	UFUNCTION()
	void HandleSaveFailed();
	UFUNCTION()
	void HandleLoadCompleted();
};
