// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/Action/ActionOwner.h"
#include "GameFramework/Character.h"
#include "Interaction/Interface/InteractorInterface.h"
#include "RPGPlayerCharacter.generated.h"

class URPGSpringArmComponentBase;
class UDataAsset_InputConfig;
class UInteractionComponent;
class UInventoryCoreComponent;
class ULocomotionComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class RPGSYSTEM_API ARPGPlayerCharacter : public ACharacter, public IActionOwner,public IInteractorInterface
{
	GENERATED_BODY()
public:
	ARPGPlayerCharacter();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	URPGSpringArmComponentBase* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UDataAsset_InputConfig> InputConfig;
protected:
	virtual void BeginPlay() override;
	virtual void NotifyControllerChanged() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;
	
	void UpdateRotationSettings();
	void UpdateMovementSpeed();
#pragma region InputFunc
	void Input_Move(const FInputActionValue& Value);
	void Input_Look(const FInputActionValue& Value);
	void Input_Interact(const FInputActionValue& Value);
	void Input_Jump(const FInputActionValue& Value);
	void Input_PrimaryAction(const FInputActionValue& Value);
	void Input_SecondaryAction(const FInputActionValue& Value);
	void Input_AbilityAction(const FInputActionValue& Value, FGameplayTag ActionTag);
	void BindInputActions(class UCustomInputComponent* RPGInputComp);
#pragma endregion InputFunc
	
#pragma region Toggle
	
	void ToggleWalk();
	void ToggleCrouch();
	void ToggleSprint();
	
#pragma endregion Toggle
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UActionComponent> ActionComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UInventoryCoreComponent> InventoryCoreComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UInteractionComponent> InteractionComponent;
	
#pragma region StateVariable 
	UPROPERTY(BlueprintReadOnly, Category = "Input")
	bool bIsWalking = false;
	UPROPERTY(BlueprintReadOnly, Category = "Input")
	bool bIsCrouch = false;
	UPROPERTY(BlueprintReadOnly, Category = "Input")
	bool bIsSprint = false;
	UPROPERTY(BlueprintReadOnly, Category = "Input")
	bool bIsLandRecovery = false;
	UPROPERTY(BlueprintReadOnly, Category = "Input")
	bool bIsAiming = false;
	UPROPERTY(BlueprintReadOnly, Category = "Input")
	bool bIsBowReady = false;
	UPROPERTY(BlueprintReadOnly, Category = "Input")
	bool bIsInCombat = false;
	UPROPERTY(BlueprintReadOnly, Category = "Input")
	bool bIsPrimaryDown;
	UPROPERTY(BlueprintReadOnly, Category = "Input")
	bool bIsSecondaryDown;
	UPROPERTY(BlueprintReadOnly, Category = "Input")
	bool bIsTertiaryDown;
	UPROPERTY(BlueprintReadOnly, Category = "Input")
	bool bIsGuarding;
#pragma endregion StateVariable
	
public:
	
#pragma region InterfaceFunc
	virtual UActionComponent* GetActionComponent() const override { return ActionComponent;}
#pragma endregion InterfaceFunc
	
#pragma region SetFunc
	UFUNCTION(BlueprintCallable, Category = "Input")
	void ApplyInputPreset(UDataAsset_InputConfig* NewInputConfig, UInputMappingContext* NewMappingContext);

	UFUNCTION(BlueprintCallable, Category = "State")
	void SetIsAiming(bool bAiming);
	
	UFUNCTION(BlueprintCallable, Category = "State")
	void SetLandRecovery(bool bState);
	
#pragma endregion SetFunc
	
#pragma region GetFunc
	FORCEINLINE class URPGSpringArmComponentBase* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	
	virtual void StartInteractionWithObject(UInteractableComponent* InteractableComponent) override;
	virtual void EndInteractionWithObject(UInteractableComponent* InteractableComponent) override;
	virtual void RemoveInteractionWithObject(UInteractableComponent* InteractableComponent) override;
	virtual void InitializeInteractionWithObject(UInteractableComponent* InteractableComponent) override;
	virtual AActor* GetCurrentInteractableObject() override;
	
	FORCEINLINE bool IsWalking() const { return bIsWalking; }
	FORCEINLINE bool IsAiming() const { return bIsAiming; }
	FORCEINLINE bool IsCrouch() const { return bIsCrouch; }
	FORCEINLINE bool IsSprint() const { return bIsSprint; }
	FORCEINLINE bool IsLandRecovery() const { return bIsLandRecovery; }
	FORCEINLINE bool IsBowReady() const { return bIsBowReady;}
	FORCEINLINE bool IsInCombat() const { return bIsInCombat; }
	FORCEINLINE bool IsGuarding() const { return bIsGuarding; }
	FORCEINLINE bool IsPrimaryDown() const { return bIsPrimaryDown; }
	FORCEINLINE bool IsSecondaryDown() const { return bIsSecondaryDown; }
	FORCEINLINE bool IsTertiaryDown() const { return bIsTertiaryDown; }
#pragma endregion GetFunc
};
