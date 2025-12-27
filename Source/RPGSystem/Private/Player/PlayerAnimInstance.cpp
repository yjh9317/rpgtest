// Fill out your copyright notice in the Description page of Project Settings.



#include "Player/PlayerAnimInstance.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"
#include "RPGSystemGameplayTags.h"
#include "Equipment/EquipmentComponent.h"
#include "Item/Data/ItemDefinition.h"
#include "Item/Data/ItemInstance.h"
#include "Player/RPGPlayerCharacter.h"

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Character = Cast<ACharacter>(GetOwningActor());
	if (Character)
	{
		RPGCharacter = Cast<ARPGPlayerCharacter>(Character);
		CharacterMovement = Character->GetCharacterMovement();
		PreviousRotation = Character->GetActorRotation();	
		
		if (USkeletalMeshComponent* MeshComp = GetOwningComponent())
		{
			MainAnimInstance = Cast<UPlayerAnimInstance>(MeshComp->GetAnimInstance());
		}

		// 1. 장비 컴포넌트 찾기
		UEquipmentComponent* EquipmentComp = Character->FindComponentByClass<UEquipmentComponent>();
		if (EquipmentComp)
		{
			EquipmentComp->OnEquipmentUpdated.RemoveDynamic(this, &UPlayerAnimInstance::OnEquipmentUpdated);
			EquipmentComp->OnEquipmentUpdated.AddDynamic(this, &UPlayerAnimInstance::OnEquipmentUpdated);

			UItemInstance* MainHandItem = EquipmentComp->GetEquippedItem(RPGGameplayTags::Equipment_Slot_MainHand);
			DetermineOverlayState(MainHandItem);
		}
	}
}

void UPlayerAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	if (!Character || !CharacterMovement)
	{
		return;
	}
	
	if (MainAnimInstance.IsValid() && MainAnimInstance.Get() != this)
	{
		UPlayerAnimInstance* MainInst = MainAnimInstance.Get();

		this->Velocity = MainInst->Velocity;
		this->GroundSpeed = MainInst->GroundSpeed;
		this->bShouldMove = MainInst->bShouldMove;
		this->bIsFalling = MainInst->bIsFalling;
		this->LocomotionDirection = MainInst->LocomotionDirection;
		this->LastLocomotionDirection = MainInst->LastLocomotionDirection;
		this->AimOffset = MainInst->AimOffset;
        
		this->OverlayState = MainInst->OverlayState;
		this->WeaponStyle = MainInst->WeaponStyle;

		this->WorldLocation = MainInst->WorldLocation;
		this->WorldRotation = MainInst->WorldRotation;
		this->LandingImpactSpeed = MainInst->LandingImpactSpeed;
		// Linked Layer는 여기서 업데이트 종료 (자체 계산 스킵)
		return;
	}

	// 1. 기본 데이터 업데이트
	Velocity = CharacterMovement->Velocity;
	GroundSpeed = Velocity.Size2D();
	
	// 가속도 확인
	FVector CurrentAcceleration = CharacterMovement->GetCurrentAcceleration();
	bHasAcceleration = CurrentAcceleration.SizeSquared() > 0.0f;
	bShouldMove = (GroundSpeed > 0.1f) && bHasAcceleration;
	
	bIsFalling = CharacterMovement->IsFalling();
	IsJumping = bIsFalling && (Velocity.Z > 0.0f); // 간단한 판별
	IsOnGround = !bIsFalling;
	IsCrouching = CharacterMovement->IsCrouching();

	// 2. 방향(Direction) 및 로컬 속도 계산
	FRotator ActorRot = Character->GetActorRotation();
	LocomotionDirection = UKismetAnimationLibrary::CalculateDirection(Velocity, ActorRot);
	
	if (GroundSpeed > 1.0f) // 아주 느릴 때는 갱신 안 함 (노이즈 방지)
	{
		LastLocomotionDirection = LocomotionDirection;
	}
	// Backward 판별 로직
	float AbsAngle = FMath::Abs(LocomotionDirection);
	if (bIsMovingBackward)
	{
		if (AbsAngle <= 100.0f) bIsMovingBackward = false;
	}
	else
	{
		if (AbsAngle > 110.0f) bIsMovingBackward = true;
	}
	
	// AimOffset 계산
	FRotator ControlRot = Character->GetControlRotation();
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);
	AimOffset = DeltaRot;

	// =========================================================================
	// Debugger 변수 업데이트 (계산 로직 추가)
	// =========================================================================
	
	// Location & Rotation
	WorldLocation = Character->GetActorLocation();
	WorldRotation = ActorRot;
	DisplacementSpeed = GroundSpeed;

	// Yaw Delta Speed (회전 속도)
	float YawDelta = UKismetMathLibrary::NormalizedDeltaRotator(ActorRot, PreviousRotation).Yaw;
	if (DeltaSeconds > 0.0f)
	{
		YawDeltaSpeed = YawDelta / DeltaSeconds;
	}
	PreviousRotation = ActorRot;

	// Velocity Data
	WorldVelocity = Velocity;
	HasVelocity = GroundSpeed > 0.1f;

	// Local Velocity (액터 기준 속도)
	FVector UnrotatedVelocity = ActorRot.UnrotateVector(Velocity);
	LocalVelocity2D = FVector(UnrotatedVelocity.X, UnrotatedVelocity.Y, 0.0f);
	
	LocalVelocityDirectionAngle = LocomotionDirection;
	LocalVelocityDirectionAngleWithOffset = LocomotionDirection; // 오프셋 로직이 있다면 여기에 적용
	
	LocalVelocityDirection = CalculateCardinalDirection(LocomotionDirection);
	LocalVelocityDirectionNoOffset = LocalVelocityDirection;

	// Acceleration Data
	FVector UnrotatedAccel = ActorRot.UnrotateVector(CurrentAcceleration);
	LocalAcceleration2D = FVector(UnrotatedAccel.X, UnrotatedAccel.Y, 0.0f);
	
	// 가속도 기반 방향
	float AccelAngle = UKismetAnimationLibrary::CalculateDirection(CurrentAcceleration, ActorRot);
	CardinalDirectionFromAcceleration = CalculateCardinalDirection(AccelAngle);

	// Pivot (급격한 방향 전환 시 사용, 예시 로직)
	PivotDirection2D = FVector::ZeroVector; 
	if (bShouldMove && FVector::DotProduct(Velocity.GetSafeNormal(), CurrentAcceleration.GetSafeNormal()) < -0.5f)
	{
		PivotDirection2D = LocalAcceleration2D.GetSafeNormal();
	}
	
	IsRunningIntoWall = false;
	TimeFalling = 0.0f; // CharacterMovement->GetTimeFalling() 등의 접근 필요하나 ThreadSafe에선 제한될 수 있음
	
	// Ground Distance (LineTrace 필요하나 ThreadSafe에선 제한적, 캡슐 하단 기준 근사치)
	GroundDistance = 0.0f; 

	// Aiming
	AimPitch = AimOffset.Pitch;
	AimYaw = AimOffset.Yaw;
	
	if (bIsFalling)
	{
		CurrentFallingSpeed = -Velocity.Z; 
	}

	if (!bIsFalling && bWasFalling)
	{
		LandingImpactSpeed = CurrentFallingSpeed;
	}

	bWasFalling = bIsFalling;
	
	if (RPGCharacter)
	{
		bIsPrimaryDown = RPGCharacter->IsPrimaryDown();
		
		bIsAiming = RPGCharacter->IsAiming();
		bIsBowReady = RPGCharacter->IsBowReady();
        bIsInCombat = RPGCharacter->IsInCombat();
		bIsGuarding = RPGCharacter->IsGuarding();
		
		
		float TargetAimAlpha = bIsAiming ? 1.0f : 0.0f;
		AimBlendWeight = FMath::FInterpTo(AimBlendWeight, TargetAimAlpha, DeltaSeconds, AimBlendSpeed);
		
		float TargetPrimaryAlpha = bIsPrimaryDown ? 1.0f : 0.0f;
		PrimaryBlendWeight = FMath::FInterpTo(PrimaryBlendWeight, TargetPrimaryAlpha, DeltaSeconds, PrimaryBlendSpeed);
		
		float TargetGuardAlpha = bIsGuarding ? 1.0f : 0.0f;
		GuardBlendWeight = FMath::FInterpTo(GuardBlendWeight, TargetPrimaryAlpha, DeltaSeconds, GuardBlendSpeed);
	}
}

ECardinalDirection UPlayerAnimInstance::CalculateCardinalDirection(float Angle) const
{
	if (Angle >= -45.0f && Angle <= 45.0f) return ECardinalDirection::Forward;
	if (Angle >= 45.0f && Angle <= 135.0f) return ECardinalDirection::Right;
	if (Angle <= -45.0f && Angle >= -135.0f) return ECardinalDirection::Left;
	return ECardinalDirection::Backward;
}


void UPlayerAnimInstance::OnEquipmentUpdated(FGameplayTag SlotTag, const UItemInstance* ItemInstance)
{
	if (SlotTag.MatchesTag(RPGGameplayTags::Equipment_Slot_MainHand))
	{
		DetermineOverlayState(ItemInstance);
	}
}

void UPlayerAnimInstance::DetermineOverlayState(const UItemInstance* ItemInstance)
{
	// 1. 일단 기본 상태로 초기화 (아이템이 없거나 매칭 안 되면 Default)
	OverlayState = ECharacterOverlayState::Default;

	if (!ItemInstance || !ItemInstance->GetItemDef())
	{
		return;
	}

	const UItemDefinition* ItemDef = ItemInstance->GetItemDef();

	for (const auto& Pair : WeaponTagToStateMap)
	{
		if (ItemDef->HasTag(Pair.Key))
		{
			OverlayState = Pair.Value;
		}
	}
	
}
