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

USTRUCT(BlueprintType)
struct FRPGAnimLocomotionData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GroundSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShouldMove = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasAcceleration = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsSprint = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCrouch = false; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsWalking = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsFalling = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LandingImpactSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsMovingBackward = false;

	// 워핑 및 방향
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LocomotionDirection = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LastLocomotionDirection = 0.0f;

	// 월드 정보 (디버깅/매칭용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator WorldRotation = FRotator::ZeroRotator;
};

// ---------------------------------------------------------
// 2. 전투 관련 데이터 (Combat Data)
// ---------------------------------------------------------
USTRUCT(BlueprintType)
struct FRPGAnimCombatData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECharacterOverlayState OverlayState = ECharacterOverlayState::Default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WeaponStyle = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator AimOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsInCombat = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsGuarding = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAiming = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsBowReady = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPrimaryDown = false;

	// 블렌딩 가중치
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AimBlendWeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PrimaryBlendWeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GuardBlendWeight = 0.0f;
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
	ECardinalDirection CalculateCardinalDirection(float Angle) const;

protected:
	// 메인 인스턴스 참조 (Linked Anim Layer일 때 사용)
	UPROPERTY(Transient, BlueprintReadOnly, Category = "References")
	TWeakObjectPtr<UPlayerAnimInstance> MainAnimInstance;
	
	UPROPERTY(BlueprintReadOnly, Category = "References")
	TObjectPtr<class ACharacter> Character;
	
	UPROPERTY(BlueprintReadOnly, Category = "References")
	TObjectPtr<class ARPGPlayerCharacter> RPGCharacter;

	UPROPERTY(BlueprintReadOnly, Category = "References")
	TObjectPtr<class UCharacterMovementComponent> CharacterMovement;

	// =========================================================
	// [수정] 구조체 변수 선언 (개별 변수 삭제됨)
	// =========================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RPG|Locomotion")
	FRPGAnimLocomotionData LocomotionData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RPG|Combat")
	FRPGAnimCombatData CombatData;

	// =========================================================
	// 설정값 및 내부 계산용 변수 (동기화 불필요)
	// =========================================================
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Config")
	float GroundSpeedInterpSpeed = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Config")
	float StopSpeed = 600.f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Config")
	float AimBlendSpeed = 10.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Config")
	float PrimaryBlendSpeed = 10.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Config")
	float GuardBlendSpeed = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	TMap<FGameplayTag, ECharacterOverlayState> WeaponTagToStateMap;

	// 내부 로직용 변수 (Sync하지 않고 직접 계산)
	bool bWasFalling = false;
	float CurrentFallingSpeed = 0.0f;
	FRotator PreviousRotation;
};