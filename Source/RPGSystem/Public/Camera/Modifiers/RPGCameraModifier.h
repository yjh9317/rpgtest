// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraModifier.h"
#include "RPGCameraModifier.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API URPGCameraModifier : public UCameraModifier
{
	GENERATED_BODY()
	
public:
	virtual void EnableModifier() override;
	virtual void DisableModifier(bool bImmediate) override;
	bool GetDebugEnabled() const { return bDebug; }

	/**
	 *  Function called once this modifier gets enabled.
	 *  @param  LastPOV  - the last view POV of the camera.
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "RPG|Camera Modifier")
	void OnModifierEnabled(FMinimalViewInfo const& LastPOV);

	/**
	 *  Function called once this modifier gets disabled.
	 *  @param  bWasImmediate  - true if modifier was disabled without a blend out.
	 *  @param  LastPOV  - the last view POV of the camera.
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "RPG|Camera Modifier")
	void OnModifierDisabled(FMinimalViewInfo const& LastPOV, bool bWasImmediate);

	/**
	 * Called to give modifiers a chance to adjust view rotation updates before they are applied.
	 *
	 * Default just returns ViewRotation unchanged
	 * @param ViewTarget - Current view target.
	 * @param InLocalControlRotation - The difference between the actor rotation and the control rotation.
	 * @param DeltaTime - Frame time in seconds.
	 * @param InViewLocation - In. The view location of the camera.
	 * @param InViewRotation - In. The view rotation of the camera.
	 * @param InDeltaRot - In/out. How much the rotation changed this frame.
	 * @param OutDeltaRot - Out. How much the control rotation should change this frame.
	 * @return Return true to prevent subsequent (lower priority) modifiers to further adjust rotation, false otherwise.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "RPG|Camera Modifier")
	bool ProcessControlRotation(AActor* ViewTarget, float DeltaTime, FVector InViewLocation, FRotator InViewRotation, FRotator InLocalControlRotation, FRotator InDeltaRot, FRotator& OutDeltaRot);

	/**
	 * Called to give modifiers a chance to adjust arm length and FOV before they are applied.
	 *
	 * @param DeltaTime - Frame time in seconds.
	 * @param InFOV - The Current FOV of the camera.
	 * @param InArmLength - The Current Arm Length of the camera.
	 * @param ViewLocation - The view location of the camera.
	 * @param ViewRotation - The view rotation of the camera.
	 * @param OutFOV - The New FOV of the camera.
	 * @param OutArmLength - The New Arm Length of the camera.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "RPG|Camera Modifier")
	void ProcessBoomLengthAndFOV(float DeltaTime, float InFOV, float InArmLength, FVector ViewLocation, FRotator ViewRotation, float& OutFOV, float& OutArmLength);

	/**
	 * Called to give modifiers a chance to adjust arm offsets before they are applied.
	 *
	 * @param DeltaTime - Frame time in seconds.
	 * @param InSocketOffset - The Current Socket Offset of the camera.
	 * @param InTargetOffset - The Current Target Offset of the camera.
	 * @param ViewLocation - The view location of the camera.
	 * @param ViewRotation - The view rotation of the camera.
	 * @param OutSocketOffset - New Socket Offset of the camera.
	 * @param OutTargetOffset - New Target Offset of the camera.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "RPG|Camera Modifier")
	void ProcessBoomOffsets(float DeltaTime, FVector InSocketOffset, FVector InTargetOffset, FVector ViewLocation, FRotator ViewRotation, FVector& OutSocketOffset, FVector& OutTargetOffset);

	/**
	 * Called to give modifiers a chance to adjust miscelaneous stuff at the end of the update order.
	 *
	 * @param DeltaTime - Frame time in seconds.
	 * @param ViewLocation - The view location of the camera.
	 * @param ViewRotation - The view rotation of the camera.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "RPG|Camera Modifier")
	void PostUpdate(float DeltaTime, FVector ViewLocation, FRotator ViewRotation);

	UFUNCTION(BlueprintNativeEvent, Category = "RPG|Camera Modifier")
	void OnAnyLevelSequenceStarted();

	UFUNCTION(BlueprintNativeEvent, Category = "RPG|Camera Modifier")
	void OnAnyLevelSequenceEnded();

	UFUNCTION(BlueprintNativeEvent, Category = "RPG|Camera Modifier")
	void OnSetViewTarget(bool bImmediate, bool bNewTargetIsOwner);

	UFUNCTION(BlueprintPure, Category = "RPG|Camera Modifier")
	bool IsDebugEnabled() const;

	UFUNCTION(BlueprintCallable, Category = "RPG|Camera Modifier")
	void ToggleDebug(bool const bEnabled);

	UFUNCTION(BlueprintCallable, Category = "RPG|Camera Modifier")
	void SetAlpha(float InAlpha) { Alpha = InAlpha; }

	/**
	 * Called to give modifiers a chance to adjust both the yaw turn rate and pitch turn rate. However the input for looking needs to have RPGCameraTurnRateModifier.
	 *
	 * @param DeltaTime - Frame time in seconds.
	 * @param InLocalControlRotation - The difference between the actor rotation and the control rotation.
	 * @param OutPitchTurnRate - Out. New value of the pitch turn rate (between 0 and 1).
	 * @param OutYawTurnRate - Out. New value of the yaw turn rate (between 0 and 1).
	 * @return Return true to prevent subsequent (lower priority) modifiers to further adjust rotation, false otherwise.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "RPG|Camera Modifier")
	bool ProcessTurnRate(float DeltaTime, FRotator InLocalControlRotation, float InPitchTurnRate, float InYawTurnRate, float& OutPitchTurnRate, float& OutYawTurnRate);

	bool CanPlayDuringCameraAnimation() const { return bPlayDuringCameraAnimations; }

	UFUNCTION(BlueprintPure, Category = "RPG|Camera Modifier|Movement")
	FVector GetOwnerVelocity() const;

	UFUNCTION(BlueprintPure, Category = "RPG|Camera Modifier|Movement")
	bool IsOwnerFalling() const;

	UFUNCTION(BlueprintPure, Category = "RPG|Camera Modifier|Movement")
	bool IsOwnerStrafing() const;

	UFUNCTION(BlueprintPure, Category = "RPG|Camera Modifier|Movement")
	bool IsOwnerMovingOnGround() const;

	UFUNCTION(BlueprintPure, Category = "RPG|Camera Modifier|Movement")
	void ComputeOwnerFloorDistance(float SweepDistance, float CapsuleRadius, bool& bOutFloorExists, float& OutFloorDistance) const;

	UFUNCTION(BlueprintPure, Category = "RPG|Camera Modifier|Movement")
	void ComputeOwnerFloorNormal(float SweepDistance, float CapsuleRadius, bool& bOutFloorExists, FVector& OutFloorNormal) const;

	UFUNCTION(BlueprintPure, Category = "RPG|Camera Modifier|Movement")
	void ComputeOwnerSlopeAngle(float& OutSlopePitchDegrees, float& OutSlopeRollDegrees);

	/*
	 * Returns value betwen 1 (the character is looking where they're moving) or -1 (looking in the opposite direction they're moving).
	 * Will return 0 if the character isn't moving.
	 */
	UFUNCTION(BlueprintPure, Category = "RPG|Camera Modifier|Movement")
	float ComputeOwnerLookAndMovementDot();

protected:
	virtual bool ModifyCamera(float DeltaTime, FMinimalViewInfo& InOutPOV) override;
	virtual void ModifyCamera(float DeltaTime, FVector ViewLocation, FRotator ViewRotation, float FOV, FVector& OutViewLocation, FRotator& OutViewRotation, float& OutFOV) override;
	virtual bool ProcessViewRotation(AActor* ViewTarget, float DeltaTime, FRotator& OutViewRotation, FRotator& OutDeltaRot) override;

	template<typename T>
	T* GetViewTargetAs() const { return Cast<T>(GetViewTarget()); }

	void UpdateOwnerReferences();

	void UpdateInternalVariables(float DeltaTime);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifier Settings")
	bool bPlayDuringCameraAnimations = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "RPG|Camera Modifier|Internal")
	TObjectPtr<class ARPGPlayerCameraManager> RPGCameraManager = nullptr;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "RPG|Camera Modifier|Internal")
	TObjectPtr<class APlayerController> OwnerController = nullptr;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "RPG|Camera Modifier|Internal")
	TObjectPtr<class ACharacter> OwnerCharacter = nullptr;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "RPG|Camera Modifier|Internal")
	TObjectPtr<class APawn> OwnerPawn = nullptr;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "RPG|Camera Modifier|Internal")
	TObjectPtr<class USpringArmComponent> SpringArm = nullptr;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "RPG|Camera Modifier|Internal")
	TObjectPtr<class UCharacterMovementComponent> MovementComponent = nullptr;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "RPG|Camera Modifier|Internal")
	FVector CurrentSocketOffset;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "RPG|CProcessBoomLengthAndFOV_Implementationamera Modifier|Internal")
	FVector CurrentTargetOffset;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "RPG|Camera Modifier|Internal")
	float CurrentArmLength;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "RPG|Camera Modifier|Internal")
	bool bHasMovementInput;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "RPG|Camera Modifier|Internal")
	FVector PreviousMovementInput;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "RPG|Camera Modifier|Internal")
	FVector MovementInput;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "RPG|Camera Modifier|Internal")
	float TimeSinceMovementInput;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "RPG|Camera Modifier|Internal")
	bool bHasRotationInput;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "RPG|Camera Modifier|Internal")
	FRotator RotationInput;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "RPG|Camera Modifier|Internal")
	float TimeSinceRotationInput;
};

/**
 * Base Camera Modifier Class for Add-on modifiers
 */
UCLASS(abstract, ClassGroup = "RPG|Add On|Camera Modifiers")
class RPGSYSTEM_API URPGCameraAddOnModifier : public URPGCameraModifier
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "RPG|Add On|Camera Modifiers")
	void SetSettings(class URPGCameraAddOnModifierSettings* InSettings);

	// Add-On Settings class associated to this add-on modifier.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	TSubclassOf<class URPGCameraAddOnModifierSettings> SettingsClass;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Settings")
	TObjectPtr<URPGCameraAddOnModifierSettings> Settings;
};
