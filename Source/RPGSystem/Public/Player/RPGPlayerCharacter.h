// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/RPGCharacterBase.h"
#include "Combat/Action/ActionOwner.h"
#include "Interaction/Interface/InteractorInterface.h"
#include "Player/LocomotionEnum.h"
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
class RPGSYSTEM_API ARPGPlayerCharacter : public ARPGCharacterBase, public IActionOwner,public IInteractorInterface
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
	void Input_Ascend(const FInputActionValue& Value);
	void Input_Descend(const FInputActionValue& Value);
	void ApplyGroundMovement(const FVector2D& MovementVector);
	void ApplyFlyingMovement(const FVector2D& MovementVector);
	void ApplySwimmingMovement(const FVector2D& MovementVector);
	void ApplyRidingMovement(const FVector2D& MovementVector);
	void UpdateRidingMovement(float DeltaSeconds);
	EPlayerMovementMode ResolveMovementModeForInput() const;
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

	UPROPERTY(Transient)
	TMap<FGameplayTag, bool> AbilityDigitalInputStates;

	UPROPERTY(Transient)
	TMap<FGameplayTag, float> AbilityAxisInputStates;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Mode")
	EPlayerMovementMode ForcedMovementMode = EPlayerMovementMode::Jog;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Flying")
	float FlyingForwardSpeedScale = 0.35f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|Flying")
	float FlyingVerticalInput = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|Flying")
	float FlyingAscendInput = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|Flying")
	float FlyingDescendInput = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Riding")
	float RideTurnRateDegPerSec = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Riding")
	float RideAcceleration = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Riding")
	float RideBraking = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Riding")
	float RideMaxForwardSpeed = 900.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|Riding")
	float RideCurrentSpeed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|Riding")
	float RideSteerInput = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|Riding")
	float RideThrottleInput = 0.0f;
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
	
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetForcedMovementMode(EPlayerMovementMode NewMode);
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
