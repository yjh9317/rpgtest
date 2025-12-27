// Source/RPGSystem/Public/Locomotion/LocomotionAnimInstance.h

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "LocomotionEnum.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"


UENUM(BlueprintType)
enum class ECharacterOverlayState : uint8
{
	Default		UMETA(DisplayName = "Default"),
	GreatSword	UMETA(DisplayName = "Great Sword"),
	SwordShield	UMETA(DisplayName = "Sword & Shield"),
	Bow			UMETA(DisplayName = "Bow"),
	Magician	UMETA(DisplayName = "Magician")
};

class UCharacterMovementComponent;
class ULocomotionComponent;
class UAnimSequence;
class UEquipmentComponent;
class UItemInstance;

UCLASS()
class RPGSYSTEM_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeInitializeAnimation() override;
	
protected:
	UFUNCTION()
	void OnEquipmentUpdated(FGameplayTag SlotTag, const UItemInstance* ItemInstance);
	void DetermineOverlayState(const UItemInstance* ItemInstance);

	// 헬퍼 함수: 로컬 속도 방향 계산
	ECardinalDirection CalculateCardinalDirection(float Angle) const;

protected:
	// 무기용 AnimInstance, 
	UPROPERTY(Transient, BlueprintReadOnly, Category = "References")
	TWeakObjectPtr<UPlayerAnimInstance> MainAnimInstance;
	
	UPROPERTY(BlueprintReadOnly, Category = "References")
	TObjectPtr<class ACharacter> Character;
	
	UPROPERTY(BlueprintReadOnly, Category = "References")
	TObjectPtr<class ARPGPlayerCharacter> RPGCharacter;

	UPROPERTY(BlueprintReadOnly, Category = "References")
	TObjectPtr<class UCharacterMovementComponent> CharacterMovement;

	// --- 기본 이동 데이터 ---
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FVector Velocity;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float GroundSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bShouldMove;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsFalling; 
	
	bool bWasFalling = false;

	float CurrentFallingSpeed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|Landing")
	float LandingImpactSpeed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsMovingBackward;

	// --- 워핑 데이터 ---
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float LocomotionDirection;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float LastLocomotionDirection;
	
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FRotator AimOffset;
	
	UPROPERTY(BlueprintReadOnly, Category = "State")
	ECharacterOverlayState OverlayState;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	TMap<FGameplayTag, ECharacterOverlayState> WeaponTagToStateMap;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	int WeaponStyle = 0;

	// =========================================================================
	// SLocomotionDebugger 지원 변수 (Debugger가 Reflection으로 접근)
	// =========================================================================
	
	// Location
	UPROPERTY(BlueprintReadOnly, Category = "Debugger|Location")
	FVector WorldLocation;

	UPROPERTY(BlueprintReadOnly, Category = "Debugger|Location")
	float DisplacementSpeed;

	// Rotation
	UPROPERTY(BlueprintReadOnly, Category = "Debugger|Rotation")
	FRotator WorldRotation;

	UPROPERTY(BlueprintReadOnly, Category = "Debugger|Rotation")
	float YawDeltaSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Debugger|Rotation")
	float AdditiveLeanAngle;

	UPROPERTY(BlueprintReadOnly, Category = "Debugger|Rotation")
	float RootYawOffset;

	// Velocity
	UPROPERTY(BlueprintReadOnly, Category = "Debugger|Velocity")
	FVector WorldVelocity;

	UPROPERTY(BlueprintReadOnly, Category = "Debugger|Velocity")
	FVector LocalVelocity2D;

	UPROPERTY(BlueprintReadOnly, Category = "Debugger|Velocity")
	float LocalVelocityDirectionAngle;

	UPROPERTY(BlueprintReadOnly, Category = "Debugger|Velocity")
	float LocalVelocityDirectionAngleWithOffset;

	UPROPERTY(BlueprintReadOnly, Category = "Debugger|Velocity")
	ECardinalDirection LocalVelocityDirection;

	UPROPERTY(BlueprintReadOnly, Category = "Debugger|Velocity")
	ECardinalDirection LocalVelocityDirectionNoOffset;

	// Acceleration
	UPROPERTY(BlueprintReadOnly, Category = "Debugger|Acceleration")
	FVector LocalAcceleration2D;

	UPROPERTY(BlueprintReadOnly, Category = "Debugger|Acceleration")
	FVector PivotDirection2D;

	UPROPERTY(BlueprintReadOnly, Category = "Debugger|Acceleration")
	ECardinalDirection CardinalDirectionFromAcceleration;

	// Character State Flags
	UPROPERTY(BlueprintReadOnly, Category = "Debugger|State")
	bool IsOnGround;

	UPROPERTY(BlueprintReadOnly, Category = "Debugger|State")
	bool IsCrouching;

	UPROPERTY(BlueprintReadOnly, Category = "Debugger|State")
	bool IsJumping;

	UPROPERTY(BlueprintReadOnly, Category = "Debugger|State")
	bool bIsSprint;

	UPROPERTY(BlueprintReadOnly, Category = "Debugger|State")
	bool HasVelocity;

	UPROPERTY(BlueprintReadOnly, Category = "Debugger|State")
	bool bHasAcceleration;

	UPROPERTY(BlueprintReadOnly, Category = "Debugger|State")
	bool IsRunningIntoWall;

	UPROPERTY(BlueprintReadOnly, Category = "Debugger|State")
	bool CrouchStateChange;

	UPROPERTY(BlueprintReadOnly, Category = "Debugger|State")
	float TimeSinceFiredWeapon;

	// Jump / Fall
	UPROPERTY(BlueprintReadOnly, Category = "Debugger|Jump")
	float TimeToJumpApex;

	UPROPERTY(BlueprintReadOnly, Category = "Debugger|Jump")
	float TimeFalling;

	UPROPERTY(BlueprintReadOnly, Category = "Debugger|Jump")
	float GroundDistance;

	UPROPERTY(BlueprintReadOnly, Category = "Debugger|Aiming")
	float AimPitch;

	UPROPERTY(BlueprintReadOnly, Category = "Debugger|Aiming")
	float AimYaw;
	
	UPROPERTY(BlueprintReadOnly, Category = "Aiming")
	bool bIsAiming;

	UPROPERTY(BlueprintReadOnly, Category = "Aiming")
	bool bIsBowReady;
	
	UPROPERTY(BlueprintReadOnly, Category = "Aiming")
	float AimBlendWeight = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Aiming")
	float AimBlendSpeed = 10.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "Mouse")
	bool bIsPrimaryDown;
	
	UPROPERTY(BlueprintReadOnly, Category = "Mouse")
	float PrimaryBlendWeight = 0.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Mouse")
	float PrimaryBlendSpeed = 10.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "Guard")
	bool bIsGuarding;
	
	UPROPERTY(BlueprintReadOnly, Category = "Guard")
	float GuardBlendWeight = 0.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Guard")
	float GuardBlendSpeed = 10.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsInCombat;
	
	UPROPERTY(EditDefaultsOnly)
	float StopSpeed = 600.f;
private:
	// 델타 타임 계산용 이전 프레임 회전값
	FRotator PreviousRotation;
};