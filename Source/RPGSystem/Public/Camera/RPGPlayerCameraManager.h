// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RPGCameraManagerProvider.h"
#include "Camera/PlayerCameraManager.h"
#include "Data/RPGCameraDataAsset.h"
#include "RPGPlayerCameraManager.generated.h"

USTRUCT(BlueprintType)
struct FCameraTimelineCurves
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Curves")
	TObjectPtr<UCurveFloat> ResetControlRotationCurve;

	UPROPERTY(EditDefaultsOnly, Category = "Curves")
	TObjectPtr<UCurveFloat> ArmTargetOffsetCurve;

	UPROPERTY(EditDefaultsOnly, Category = "Curves")
	TObjectPtr<UCurveFloat> ArmSocketOffsetCurve;

	UPROPERTY(EditDefaultsOnly, Category = "Curves")
	TObjectPtr<UCurveFloat> YawRangeCurve;

	UPROPERTY(EditDefaultsOnly, Category = "Curves")
	TObjectPtr<UCurveFloat> PitchRangeCurve;

	UPROPERTY(EditDefaultsOnly, Category = "Curves")
	TObjectPtr<UCurveFloat> ArmLengthRangeCurve;

	UPROPERTY(EditDefaultsOnly, Category = "Curves")
	TObjectPtr<UCurveFloat> FOVRangeCurve;
};

class UMovieSceneSequence;
class UCameraAnimationSequence;

UCLASS()
class RPGSYSTEM_API ARPGPlayerCameraManager : public APlayerCameraManager , public IRPGCameraManagerProvider
{
	GENERATED_BODY()
public:
	ARPGPlayerCameraManager();
	
	
	virtual void InitializeFor(class APlayerController* PC) override;

	/*
	* This returns the real camera view, this isn't necessarily the view of the camera attached to your character.
	* For example, during camera animations, the camera used is different than the normal gameplay camera.
	*/
	UFUNCTION(BlueprintPure, Category = "Camera Manager")
	void GetRealCameraView(FVector& OutViewLocation, FRotator& OutViewRotation) { OutViewLocation = ViewTarget.POV.Location; OutViewRotation = ViewTarget.POV.Rotation; }

	/*
	 * Prepare the possession of a new pawn. Doesn't actually possess the pawn.
	 * APlayerController::Possess needs to be called directly after this function.
	 * @param  NewPawn                      The new pawn that we intend on possessing.
	 * @param  NewCameraDA                  The camera data asset that will be pushed after possessing.
											*Requires call to PostPosses after the APlayerController::Possess has been called!*
	 * @param  bBlendSpringArmProperties     Whether the new pawn's spring arm should prepare to blend from the previous spring arm.
											*This doesn't work with SetViewTargetWithBlend!*
											*Requires call to PostPosses after the APlayerController::Possess has been called!*
	 * @param  bMatchCameraRotation         Whether the new pawn should have the same camera spring arm rotation (aka Control Rotation) or not.
											*This doesn't work with SetViewTargetWithBlend!*
											*Requires call to PostPosses after the APlayerController::Possess has been called!*
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera Manager")
	void PrePossess(class APawn* NewPawn, class URPGCameraDataAssetBase* NewCameraDA, bool bBlendSpringArmProperties = false, bool bMatchCameraRotation = false);

	/*
	 * Finush the possession of a new pawn. APlayerController::Possess needs to have been called directly before this function.
	 * @param  bReplaceCurrentCameraDA      Whether to push the Camera DA that was passed in the PrePossess function (if any) should
											replace the current camera DA or be pushed on top of it.
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera Manager")
	void PostPossess(bool bReplaceCurrentCameraDA = true);

	UFUNCTION(BlueprintCallable, Category = "Camera Manager")
	void RefreshLevelSequences();

	UFUNCTION(BlueprintPure, Category = "Camera Manager")
	bool IsPlayingAnyLevelSequence() const { return NbrActiveLevelSequences > 0 || NbrActivePausedLevelSequences > 0; }

	// Plays a single new camera animation sequence. Any subsequent calls while this animation runs will interrupt the current animation.
	// This variation can be used in contexts where async nodes aren't allowd (e.g., AnimNotifies).
	UFUNCTION(BlueprintCallable, Category = "Camera Manager")
	void PlayCameraAnimation(class UCameraAnimationSequence* CameraSequence, struct FRPGCameraAnimationParams const& Params, bool bInterruptOthers, bool bDoCollisionChecks);

	/**
	 *  Enables/Disables all camera modifiers ONLY if they inherit from RPG Camera Modifier.
	 *  @param  bEnabled    - true to enable all RPG camera modifiers, false to disable.
	 *  @param  bImmediate  - If bEnabled is false: true to disable immediately with no blend out, false (default) to allow blend out
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera Manager")
	void ToggleRPGCameraModifiers(bool const bEnabled, bool const bImmediate = true);

	/**
	 *  Enables/Disables all camera modifiers, regardless of whether they inherit from RPG Camera Modifier.
	 *  @param  bEnabled    - true to enable all camera modifiers, false to disable.
	 *  @param  bImmediate  - If bEnabled is false: true to disable immediately with no blend out, false (default) to allow blend out
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera Manager")
	void ToggleCameraModifiers(bool const bEnabled, bool const bImmediate = true);

	/**
	 *  Enables/Disables all debugging of camera modifiers ONLY if they inherit from RPG Camera Modifier.
	 *  @param  bEnabled    - true to enable all debugging of RPG camera modifiers, false to disable.
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera Manager")
	void ToggleAllRPGModifiersDebug(bool const bEnabled);
	/**
	 *
	 *  Enables/Disables all debugging of camera modifiers regardless of whether they inherit from RPG Camera Modifier.
	 *  @param  bEnabled    - true to enable all debugging of all camera modifiers, false to disable.
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera Manager")
	void ToggleAllModifiersDebug(bool const bEnabled);

	UFUNCTION(BlueprintPure, Category = "Camera Manager")
	class USpringArmComponent* GetOwnerSpringArmComponent() const { return CameraArm.Get(); }

	UFUNCTION(BlueprintPure, Category = "Camera Manager")
	class URPGCameraDataAssetBase* GetCurrentCameraDataAsset() const { return CameraDataStack.IsEmpty() ? nullptr : CameraDataStack[CameraDataStack.Num() - 1]; }

	UFUNCTION(BlueprintCallable, Category = "Camera Manager")
	void PushCameraData(class URPGCameraDataAssetBase* CameraDA);

	// Pops the most recent Camera DA.
	UFUNCTION(BlueprintCallable, Category = "Camera Manager")
	void PopCameraDataHead();

	// Pops the given Camera DA. If it isn't in stack, it returns false.
	UFUNCTION(BlueprintCallable, Category = "Camera Manager")
	void PopCameraData(class URPGCameraDataAssetBase* CameraDA);

	UFUNCTION(BlueprintNativeEvent, Category = "Camera Manager")
	void OnCameraDataStackChanged(class URPGCameraDataAssetBase* CameraDA, bool bBlendCameraProperties = true);

	/** Draw a debug camera shape at the real camera's location, which can be different from the character's attached camera.
	 * For example, camera animations use a different camera then the character's attached camera.
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera Manager|Debug")
	void DrawRealDebugCamera(float Duration, FLinearColor CameraColor = FLinearColor::Red, float Thickness = 1.f) const;

	/** Draw a debug camera shape at the character's attached camera's location, which can be different from the real camera.
	 * For example, camera animations use a different camera then the character's attached camera.
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera Manager|Debug")
	void DrawGameDebugCamera(float Duration, bool bDrawCamera = true, FLinearColor CameraColor = FLinearColor::Blue, bool bDrawSpringArm = true, FLinearColor SpringArmColor = FLinearColor::Blue, float Thickness = 1.f) const;

	/* Draw Spring Arm.
	*/
	UFUNCTION(BlueprintCallable, Category = "Camera Manager|Debug")
	void DrawDebugSpringArm(FVector const& CameraLocation, float Duration, FLinearColor SpringArmColor = FLinearColor::Blue, float Thickness = 1.f) const;

	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Camera Manager")
	bool IsPlayingAnyCameraAnimation() const;

	UFUNCTION(BlueprintPure, Category = "Camera Manager|Movement")
	FVector GetOwnerVelocity() const;

	UFUNCTION(BlueprintPure, Category = "Camera Manager|Movement")
	void ComputeOwnerFloorDist(float SweepDistance, float CapsuleRadius, bool& bOutFloorExists, float& OutFloorDistance) const;

	UFUNCTION(BlueprintPure, Category = "Camera Manager|Movement")
	bool IsOwnerFalling() const;

	UFUNCTION(BlueprintPure, Category = "Camera Manager|Movement")
	bool IsOwnerStrafing() const;

	UFUNCTION(BlueprintPure, Category = "Camera Manager|Movement")
	bool IsOwnerMovingOnGround() const;

	UFUNCTION(BlueprintPure, Category = "Camera Manager|Movement")
	void ComputeOwnerFloorNormal(float SweepDistance, float CapsuleRadius, bool& bOutFloorExists, FVector& OutFloorNormal) const;

	UFUNCTION(BlueprintPure, Category = "Camera Modifier|Movement")
	void ComputeOwnerSlopeAngle(float& OutSlopePitchDegrees, float& OutSlopeRollDegrees);

	/*
	 * Returns value betwen 1 (the character is looking where they're moving) or -1 (looking in the opposite direction they're moving).
	 * Will return 0 if the character isn't moving.
	 */
	UFUNCTION(BlueprintPure, Category = "Camera Modifier|Movement")
	float ComputeOwnerLookAndMovementDot();

	/*
	 * Only use this when your pawn has multiple spring arm components and you need to use a different one.
	 * The spring arm is automatically reset to the first found spring arm component on the pawn when we initialize
	 * the camera manager or when we possess a new pawn.
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera Manager")
	void SetSpringArmComponent(class USpringArmComponent* Arm) { if (Arm) CameraArm = Arm; }

	/**
	 * Returns camera modifier for this camera of the given class, if it exists.
	 * Looks for inherited classes too. If there are multiple modifiers which fit, the first one is returned.
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera Manager", meta = (DeterminesOutputType = "ModifierClass"))
	UCameraModifier* FindCameraModifierOfClass(TSubclassOf<UCameraModifier> ModifierClass, bool bIncludeInherited);

	FVector GetCameraTurnRate() const { return FVector(YawTurnRate, PitchTurnRate, 0.f); }

	UCameraModifier const* FindCameraModifierOfClass(TSubclassOf<UCameraModifier> ModifierClass, bool bIncludeInherited) const;

	template<typename T>
	T* FindCameraModifierOfType()
	{
		bool constexpr bIncludeInherited = true;
		UCameraModifier* Modifier = FindCameraModifierOfClass(T::StaticClass(), bIncludeInherited);
		return Modifier != nullptr ? Cast<T>(Modifier) : nullptr;
	}

	template<typename T>
	T const* FindCameraModifierOfType() const
	{
		bool constexpr bIncludeInherited = true;
		UCameraModifier const* Modifier = FindCameraModifierOfClass(T::StaticClass(), bIncludeInherited);
		return Modifier != nullptr ? Cast<T>(Modifier) : nullptr;
	}
protected:
	void PushCameraData_Internal(class URPGCameraDataAssetBase* CameraDA, bool bBlendCameraProperties);

	void DoForEachRPGModifier(TFunction<void(class URPGCameraModifier*)> const& Function);

	// Breaks when the function returns true.
	void DoForEachRPGModifierWithBreak(TFunction<bool(class URPGCameraModifier*)> const& Function);

	virtual void DisplayDebug(class UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;
	virtual UCameraModifier* AddNewCameraModifier(TSubclassOf<UCameraModifier> ModifierClass) override;
	virtual bool RemoveCameraModifier(UCameraModifier* ModifierToRemove) override;

	virtual void SetViewTarget(AActor* NewViewTarget, FViewTargetTransitionParams TransitionParams) override;

	/**
	 * Called to give PlayerCameraManager a chance to adjust view rotation updates before they are applied.
	 * e.g. The base implementation enforces view rotation limits using LimitViewPitch, et al.
	 * @param DeltaTime - Frame time in seconds.
	 * @param OutViewRotation - In/out. The view rotation to modify.
	 * @param OutDeltaRot - In/out. How much the rotation changed this frame.
	 */
	virtual void ProcessViewRotation(float DeltaTime, FRotator& OutViewRotation, FRotator& OutDeltaRot);

	/**
	 * Called to give PlayerCameraManager a chance to adjust both the yaw turn rate and pitch turn rate.
	 *
	 * @param DeltaTime - Frame time in seconds.
	 * @param InLocalControlRotation - The difference between the actor rotation and the control rotation.
	 * @param OutPitchTurnRate - Out. New value of the pitch turn rate (between 0 and 1).
	 * @param OutYawTurnRate - Out. New value of the yaw turn rate (between 0 and 1).
	 * @return Return true to prevent subsequent (lower priority) modifiers to further adjust rotation, false otherwise.
	 */
	virtual void ProcessTurnRate(float DeltaTime, FRotator InLocalControlRotation, float& OutPitchTurnRate, float& OutYawTurnRate);

	virtual void Tick(float DeltaTime) override;

	virtual void LimitViewYaw(FRotator& ViewRotation, float InViewYawMin, float InViewYawMax) override;

	// This updates the internal variables of the RPG Player Camera Manager. Make sure to call the parent function if you override this in BP.
	UFUNCTION(BlueprintNativeEvent, Category = "Camera Manager|Internal")
	void UpdateInternalVariables(float DeltaTime);

	// Usually uses the RPG Pawn Interface to fetch the rotation input of the camera (Mouse or Right Thumbstick). Override this if you want to provide your own way of getting the camera rotation input.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Camera Manager|Internal")
	FRotator GetRotationInput() const;

	// Usually uses the RPG Pawn Interface to fetch the movement input of the character (WASD or Left Thumbstick). Override this if you want to provide your own way of getting the camera rotation input.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Camera Manager|Internal")
	FVector GetMovementControlInput() const;

	UFUNCTION()
	void OnLevelSequenceStarted();

	UFUNCTION()
	void OnLevelSequencePaused();

	UFUNCTION()
	void OnLevelSequenceEnded();
protected:
	friend class URPGCameraModifier;
	
	// =================================================================
	// [Internal] Timelines
	// 블루프린트의 타임라인 노드에 대응하는 컴포넌트들입니다.
	// 실제 사용 시 CreateDefaultSubobject로 생성하거나 Tick에서 FMath::Interp로 대체 구현해야 합니다.
	// =================================================================
	
	UPROPERTY(BlueprintReadOnly, Category = "Internal|Timelines")
	TObjectPtr<class UTimelineComponent> ResetControlRotationToTimeline;

	UPROPERTY(BlueprintReadOnly, Category = "Internal|Timelines")
	TObjectPtr<class UTimelineComponent> ArmTargetOffsetTimeline;

	UPROPERTY(BlueprintReadOnly, Category = "Internal|Timelines")
	TObjectPtr<class UTimelineComponent> ArmSocketOffsetTimeline;

	UPROPERTY(BlueprintReadOnly, Category = "Internal|Timelines")
	TObjectPtr<class UTimelineComponent> YawRangeTimeline;

	UPROPERTY(BlueprintReadOnly, Category = "Internal|Timelines")
	TObjectPtr<class UTimelineComponent> PitchRangeTimeline;

	UPROPERTY(BlueprintReadOnly, Category = "Internal|Timelines")
	TObjectPtr<class UTimelineComponent> ResetControlRotationTimeline;

	UPROPERTY(BlueprintReadOnly, Category = "Internal|Timelines")
	TObjectPtr<class UTimelineComponent> ArmLengthRangeTimeline;

	UPROPERTY(BlueprintReadOnly, Category = "Internal|Timelines")
	TObjectPtr<class UTimelineComponent> FOVRangeTimeline;
	
	// =================================================================
	// [Settings] Curves
	// 에디터에서 이 구조체 안에 커브들을 할당하면 됩니다.
	// =================================================================
    
	UPROPERTY(EditDefaultsOnly, Category = "Camera|Settings")
	FCameraTimelineCurves TimelineCurves;
	
	// =================================================================
	// [Internal] State Caching (Previous & Target Values)
	// =================================================================
	
	// Reset Rotation
	UPROPERTY(Transient) FRotator StartControlRotation;
	UPROPERTY(Transient) FRotator TargetResetRotation;
	UPROPERTY(Transient) bool bInterruptResetWithLookInput;

	// Arm Length
	UPROPERTY(Transient) float PreviousMinArmLength;
	UPROPERTY(Transient) float PreviousMaxArmLength;
	UPROPERTY(Transient) float TargetMinArmLength;
	UPROPERTY(Transient) float TargetMaxArmLength;

	// FOV
	UPROPERTY(Transient) float PreviousMinFOV;
	UPROPERTY(Transient) float PreviousMaxFOV;
	UPROPERTY(Transient) float TargetMinFOV;
	UPROPERTY(Transient) float TargetMaxFOV;

	// Offsets
	UPROPERTY(Transient) FVector PreviousArmSocketOffset;
	UPROPERTY(Transient) FVector TargetArmSocketOffset;
	UPROPERTY(Transient) FVector PreviousArmTargetOffset;
	UPROPERTY(Transient) FVector TargetArmTargetOffset;

	// Angle Constraints
	UPROPERTY(Transient) float PreviousMinPitch;
	UPROPERTY(Transient) float PreviousMaxPitch;
	UPROPERTY(Transient) float TargetMinPitch;
	UPROPERTY(Transient) float TargetMaxPitch;

	UPROPERTY(Transient) float PreviousMinYaw;
	UPROPERTY(Transient) float PreviousMaxYaw;
	UPROPERTY(Transient) float TargetMinYaw;
	UPROPERTY(Transient) float TargetMaxYaw;
	
	// =================================================================
	// Modifier
	// =================================================================
	
	UPROPERTY(EditDefaultsOnly, Category = "Modifiers|Classes")
	TSubclassOf<class URPGCameraAnimationModifier> AnimationModifierClass;

	UPROPERTY(EditDefaultsOnly, Category = "Modifiers|Classes")
	TSubclassOf<class URPGCameraCollisionModifier> CollisionModifierClass;

	UPROPERTY(EditDefaultsOnly, Category = "Modifiers|Classes")
	TSubclassOf<class URPGCameraDitheringModifier> DitheringModifierClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Modifiers|Classes|Child")
	TSubclassOf<class UFocusCameraModifier> FocusModifierClass;

	UPROPERTY(EditDefaultsOnly, Category = "Modifiers|Classes|Child")
	TSubclassOf<class UYawFollowCameraModifier> YawFollowModifierClass;

	UPROPERTY(EditDefaultsOnly, Category = "Modifiers|Classes|Child")
	TSubclassOf<class UPitchFollowModifier> PitchFollowModifierClass;

	UPROPERTY(EditDefaultsOnly, Category = "Modifiers|Classes|Child")
	TSubclassOf<class UArmOffsetModifier> ArmOffsetModifierClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Modifiers|Classes|Child")
	TSubclassOf<class UArmLagModifier> ArmLagModifierClass;

	UPROPERTY(EditDefaultsOnly, Category = "Modifiers|Classes|Child")
	TSubclassOf<class UPitchToArmLengthAndFOVModifier> PitchToArmFOVModifierClass;

	UPROPERTY(EditDefaultsOnly, Category = "Modifiers|Classes|Child")
	TSubclassOf<class UAnglesConstraintModifier> AnglesConstraintModifierClass;
	
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Modifiers|Cache")
	TObjectPtr<class URPGCameraAnimationModifier> CachedAnimationModifier;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Modifiers|Cache")
	TObjectPtr<class URPGCameraCollisionModifier> CachedCollisionModifier;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Modifiers|Cache")
	TObjectPtr<class URPGCameraDitheringModifier> CachedDitheringModifier;
	
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Modifiers|Cache")
	TObjectPtr<class UFocusCameraModifier> CachedFocusModifier;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Modifiers|Cache")
	TObjectPtr<class UYawFollowCameraModifier> CachedYawFollowModifier;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Modifiers|Cache")
	TObjectPtr<class UPitchFollowModifier> CachedPitchFollowModifier;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Modifiers|Cache")
	TObjectPtr<class UArmOffsetModifier> CachedArmOffsetModifier;
	
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Modifiers|Cache")
	TObjectPtr<class UArmLagModifier> CachedArmLagModifier;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Modifiers|Cache")
	TObjectPtr<class UPitchToArmLengthAndFOVModifier> CachedPitchToArmFOVModifier;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Modifiers|Cache")
	TObjectPtr<class UAnglesConstraintModifier> CachedAnglesConstraintModifier;

    // =================================================================
    // [RPG Camera] Current Values
    // 현재 적용 중인 카메라 설정값들입니다.
    // =================================================================

    UPROPERTY(Transient, BlueprintReadOnly, Category = "RPG|Camera|State")
    float MinFOV;

    UPROPERTY(Transient, BlueprintReadOnly, Category = "RPG|Camera|State")
    float MaxFOV;

    UPROPERTY(Transient, BlueprintReadOnly, Category = "RPG|Camera|State")
    FVector ArmSocketOffset;

    UPROPERTY(Transient, BlueprintReadOnly, Category = "RPG|Camera|State")
    FVector ArmTargetOffset;

    UPROPERTY(Transient, BlueprintReadOnly, Category = "RPG|Camera|State")
    float MinArmLength;

    UPROPERTY(Transient, BlueprintReadOnly, Category = "RPG|Camera|State")
    float MaxArmLength;

    // =================================================================
    // [Debug & Data]
    // =================================================================

    UPROPERTY(Transient, BlueprintReadOnly, Category = "Debug")
    FTimerHandle RealCamDebugTimer;

    UPROPERTY(Transient, BlueprintReadOnly, Category = "Debug")
    FTimerHandle GameCamDebugTimer;

    /** 기본 카메라 데이터 에셋 (초기화 시 사용) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Default")
    TObjectPtr<class URPGCameraDataAssetBase> DefaultCameraData;
	
	// =================================================================
	// Internal
	// =================================================================

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Camera Manager|Internal")
	TArray<class URPGCameraDataAssetBase*> CameraDataStack;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Camera Manager|Internal")
	TObjectPtr<class ACharacter> OwnerCharacter;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Camera Manager|Internal")
	TObjectPtr<class APawn> OwnerPawn;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Camera Manager|Internal")
	TObjectPtr<class USpringArmComponent> CameraArm;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Camera Manager|Internal")
	TObjectPtr<class UCharacterMovementComponent> MovementComponent;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Camera Manager|Internal")
	TArray<class ALevelSequenceActor*> LevelSequences;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera Manager|Angle Constraints", meta = (UIMin = 0.f, UIMax = 1.f, ClampMin = 0.f, ClampMax = 1.f))
	float PitchTurnRate = 1.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera Manager|Angle Constraints", meta = (UIMin = 0.f, UIMax = 1.f, ClampMin = 0.f, ClampMax = 1.f))
	float YawTurnRate = 1.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Camera Manager|Internal")
	float AspectRatio = 0.f;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Camera Manager|Internal")
	float VerticalFOV = 0.f;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Camera Manager|Internal")
	float HorizontalFOV = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera Manager|Internal")
	bool bHasMovementInput = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera Manager|Internal")
	FVector MovementInput = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera Manager|Internal")
	float TimeSinceMovementInput = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera Manager|Internal")
	bool bHasRotationInput = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera Manager|Internal")
	FRotator RotationInput = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera Manager|Internal")
	float TimeSinceRotationInput = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera Manager|Internal")
	float OriginalArmLength = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera Manager|Internal")
	TArray<class URPGCameraModifier*> RPGModifiersList;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera Manager|Internal")
	TArray<class URPGCameraAddOnModifier*> RPGAddOnModifiersList;

	UPROPERTY(Transient)
	int NbrActiveLevelSequences = 0;

	UPROPERTY(Transient)
	int NbrActivePausedLevelSequences = 0;

	struct PossessPayload
	{
		TObjectPtr<class URPGCameraDataAssetBase> PendingCameraDA = nullptr;
		FRotator PendingControlRotation = FRotator::ZeroRotator;
		bool bBlendCameraProperties = false;
		bool bMatchCameraRotation = false;
	} PendingPossessPayload;
	
public:
	void SetupModifierReferences();

    /** 카메라 데이터를 설정합니다 (Push/Replace) */
    void SetCameraData(class URPGCameraDataAssetBase* NewCameraData);

    // =================================================================
    // [Camera Arm]
    // =================================================================

    void GetCurrentArmOffsets(FVector& OutSocketOffset, FVector& OutTargetOffset) const;

    void GetCurrentArmLengthRange(float& MinLength, float& MaxLength) const;

    FVector GetCameraPivotLocation() const;

    /** 컨트롤러의 회전값을 반환합니다. */
    FRotator GetControlRotation() const;

    // =================================================================
    // [Camera FOV]
    // =================================================================

    void GetCurrentFOVRange(float& OutMinFOV, float& OutMaxFOV) const;

    // =================================================================
    // [Camera Pitch To Arm/FOV]
    // =================================================================

    void SetPitchToArmLengthAndFOVCurves(const FCameraPitchToArmAndFOVCurveSettings& InSettings);

    // =================================================================
    // [Camera Animations]
    // =================================================================

    float GetCameraAnimationDuration(UMovieSceneSequence* Sequence,const FRPGCameraAnimationParams& AnimationParams) const;

    void StopCameraAnimation(UCameraAnimationSequence* Sequence, bool bImmediate = false);

    bool IsPlayingCameraAnimation(UCameraAnimationSequence* Sequence) const;

    void GetCurrentCameraAnimations(TArray<UCameraAnimationSequence*>& OutAnimations) const;

    // =================================================================
    // [Camera Follow] (Yaw/Pitch)
    // =================================================================

    bool IsYawMovementFollowEnabled() const;
	bool IsPitchMovementFollowEnabled() const;
	UFUNCTION(BlueprintCallable, Category = "Camera Manager")
    void SetYawMovementFollowSettings(const FCameraYawFollowSettings& InSettings);
	UFUNCTION(BlueprintCallable, Category = "Camera Manager")
    void SetPitchMovementFollowSettings(const FCameraPitchFollowSettings& InSettings);
	UFUNCTION(BlueprintCallable, Category = "Camera Manager")
    void ToggleYawAndPitchFollow(bool bEnableYaw, bool bEnablePitch, bool bTriggerPitchFollowWhenFalling,bool bTriggerPitchFollowOnSlopes);

    // =================================================================
    // [Camera Angle Constraints]
    // =================================================================

    void SetPitchConstraints(const FCameraPitchConstraintSettings& InSettings);
    void SetYawConstraints(const FCameraYawConstraintSettings& InSettings);

    // =================================================================
    // [Camera Focus]
    // =================================================================

    void SetCameraFocusSettings(const FRPGCameraFocusSettings& InSettings);
	UFUNCTION(BlueprintCallable)
    void ToggleCameraFocus(bool bEnable);
	UFUNCTION(BlueprintCallable)
	URPGFocusTargetMethod* SetCameraFocusTargetingMethod(TSubclassOf<URPGFocusTargetMethod> MethodClass);

    bool IsFocusEnabledInCurrentData() const;

    // =================================================================
    // [Camera Dithering]
    // =================================================================

    void SetCameraDitheringSettings(const FCameraDitheringSettings& InSettings);

    // =================================================================
    // [Camera Lag]
    // =================================================================

    void SetCameraLagSettings(const FCameraArmLagSettings& InSettings);

    // =================================================================
    // [Debug / AddOns]
    // =================================================================

    void UpdateAddOnSettings(TSubclassOf<class URPGCameraAddOnModifierSettings> SettingsClass, class URPGCameraAddOnModifierSettings* NewSettings);
	
protected:
	// =================================================================
	// [Internal] Timeline Callbacks
	// =================================================================
	UFUNCTION() void OnResetControlRotationTimelineUpdate(float Alpha);
	UFUNCTION() void OnArmLengthRangeTimelineUpdate(float Alpha);
	UFUNCTION() void OnFOVRangeTimelineUpdate(float Alpha);
	UFUNCTION() void OnArmSocketOffsetTimelineUpdate(float Alpha);
	UFUNCTION() void OnArmTargetOffsetTimelineUpdate(float Alpha);
	UFUNCTION() void OnPitchRangeTimelineUpdate(float Alpha);
	UFUNCTION() void OnYawRangeTimelineUpdate(float Alpha);
	
public:
	// =================================================================
	// [Public API] 1~4. Control Rotation Reset
	// =================================================================

	/** (1,2번) 특정 회전값으로 컨트롤러 회전을 부드럽게 리셋 */
	UFUNCTION(BlueprintCallable, Category = "RPG|Camera|Control")
	void ResetControlRotationTo(FRotator TargetRotation, float Duration, UCurveFloat* Curve, bool bInterruptWithLookInput);

	/** (3,4번) 캐릭터의 뒷모습을 바라보도록 컨트롤러 회전 리셋 */
	UFUNCTION(BlueprintCallable, Category = "RPG|Camera|Control")
	void ResetControlRotation(float Duration, UCurveFloat* Curve, bool bInterruptWithLookInput);

	// =================================================================
	// [Public API] 5~6. Camera Range Settings (Arm Length, FOV)
	// =================================================================

	/** (5번) Arm Length Min/Max 범위를 변경 */
	UFUNCTION(BlueprintCallable, Category = "RPG|Camera|Settings")
	void SetArmLengthRange(float InMinArmLength, float InMaxArmLength, float InBlendTime, UCurveFloat* InBlendCurve);

	/** (6번) FOV Min/Max 범위를 변경 */
	UFUNCTION(BlueprintCallable, Category = "RPG|Camera|Settings")
	void SetFOVRange(float InMinFOV, float InMaxFOV, float InBlendTime, UCurveFloat* InBlendCurve);

	// =================================================================
	// [Public API] 7~8. Camera Offsets
	// =================================================================

	/** (7번) 소켓 오프셋 변경 */
	UFUNCTION(BlueprintCallable, Category = "RPG|Camera|Settings")
	void SetArmSocketOffset(FVector ArmOffset, float BlendTime, UCurveFloat* BlendCurve);

	/** (8번) 타겟 오프셋 변경 */
	UFUNCTION(BlueprintCallable, Category = "RPG|Camera|Settings")
	void SetArmTargetOffset(FVector InArmTargetOffset, float BlendTime, UCurveFloat* BlendCurve);

	// =================================================================
	// [Public API] 9~10. Angle Constraints
	// =================================================================

	/** (9번) Pitch 회전 제한 범위 변경 */
	UFUNCTION(BlueprintCallable, Category = "RPG|Camera|Settings")
	void SetPitchViewConstraints(float InViewPitchMin, float InViewPitchMax, float InBlendTime, UCurveFloat* InBlendCurve);

	/** (10번) Yaw 회전 제한 범위 변경 */
	UFUNCTION(BlueprintCallable, Category = "RPG|Camera|Settings")
	void SetYawViewConstraints(float InViewYawMin, float InViewYawMax, float InBlendTime, UCurveFloat* InBlendCurve);
	
	virtual ARPGPlayerCameraManager* GetRPGPlayerCameraManager_Implementation() override { return this;}
};
