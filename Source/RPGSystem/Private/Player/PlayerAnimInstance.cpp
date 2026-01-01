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
    
    // [Linked Anim Layer 처리]
    if (MainAnimInstance.IsValid() && MainAnimInstance.Get() != this)
    {
       UPlayerAnimInstance* MainInst = MainAnimInstance.Get();
       this->LocomotionData = MainInst->LocomotionData;
       this->CombatData     = MainInst->CombatData;
       return;
    }

    // -------------------------------------------------------------------------
    // [Case 2] Main Anim Instance 로직
    // -------------------------------------------------------------------------
    
    // [중요] 상태값을 먼저 가져옵니다. (방향 계산에 쓰기 위해 위로 올림)
    bool bCurrentAiming = false;
    if (RPGCharacter)
    {
        bCurrentAiming = RPGCharacter->IsAiming();
        
        // 나머지 상태들도 미리 업데이트
        LocomotionData.bIsSprint  = RPGCharacter->IsSprint();
        LocomotionData.bIsCrouch  = RPGCharacter->IsCrouch();
        LocomotionData.bIsWalking = RPGCharacter->IsWalking();
        
        CombatData.bIsInCombat = RPGCharacter->IsInCombat();
        CombatData.bIsAiming = bCurrentAiming; // 미리 저장
        CombatData.bIsGuarding = RPGCharacter->IsGuarding();
        CombatData.bIsPrimaryDown = RPGCharacter->IsPrimaryDown();
        CombatData.AimOffset = RPGCharacter->GetBaseAimRotation() - RPGCharacter->GetActorRotation();
    }

    // 1. 기본 이동 데이터 계산
    LocomotionData.WorldLocation = Character->GetActorLocation();
    LocomotionData.WorldRotation = Character->GetActorRotation();
    LocomotionData.Velocity = CharacterMovement->Velocity;
    LocomotionData.GroundSpeed = LocomotionData.Velocity.Size2D();
    LocomotionData.bHasAcceleration = (CharacterMovement->GetCurrentAcceleration().SizeSquared2D() > 0.0f);
    
    // Should Move 판단
    LocomotionData.bShouldMove = (LocomotionData.GroundSpeed > 3.0f && LocomotionData.bHasAcceleration);

    // Falling 상태 처리
    LocomotionData.bIsFalling = CharacterMovement->IsFalling();
    if (LocomotionData.bIsFalling)
    {
       CurrentFallingSpeed = LocomotionData.Velocity.Z; 
    }
    else if (bWasFalling)
    {
       LocomotionData.LandingImpactSpeed = FMath::Abs(CurrentFallingSpeed);
    }
    bWasFalling = LocomotionData.bIsFalling;

    // =========================================================
    // [핵심 변경 구간] 방향 및 워핑 계산 (LocomotionDirection)
    // =========================================================
    if (LocomotionData.bShouldMove)
    {
       LocomotionData.LastLocomotionDirection = LocomotionData.LocomotionDirection;
       
       // (A) 조준(Aiming) 중: 스트레이프 모드
       // 실제 이동 방향과 몸(카메라) 각도 차이를 계산 -> BlendSpace (-180 ~ 180) 적용
       if (bCurrentAiming)
       {
           FRotator VelocityRot = LocomotionData.Velocity.ToOrientationRotator();
           LocomotionData.LocomotionDirection = UKismetMathLibrary::NormalizedDeltaRotator(VelocityRot, Character->GetActorRotation()).Yaw;
       }
       // (B) 탐험(Exploration) 중: 오리엔트 모드
       // 캐릭터가 물리적으로 회전하므로, 다리는 '앞(0)'으로 고정 -> 자연스럽게 회전하며 달림
       else
       {
           // 0으로 바로 꽂아도 되지만, Aim -> NotAim 전환 시 부드럽게 풀리도록 보간
           LocomotionData.LocomotionDirection = FMath::FInterpTo(LocomotionData.LocomotionDirection, 0.0f, DeltaSeconds, 5.0f);
       }
    }
    else
    {
        // 멈춰있을 땐 방향 0 (혹은 이전 값 유지)
        LocomotionData.LocomotionDirection = 0.0f; 
    }
    
    // 후진 판단
    LocomotionData.bIsMovingBackward = (FMath::Abs(LocomotionData.LocomotionDirection) > 130.0f);

    // 블렌드 웨이트 계산
    CombatData.AimBlendWeight = FMath::FInterpTo(CombatData.AimBlendWeight, CombatData.bIsAiming ? 1.0f : 0.0f, DeltaSeconds, AimBlendSpeed);
    CombatData.PrimaryBlendWeight = FMath::FInterpTo(CombatData.PrimaryBlendWeight, CombatData.bIsPrimaryDown ? 1.0f : 0.0f, DeltaSeconds, PrimaryBlendSpeed);
    CombatData.GuardBlendWeight = FMath::FInterpTo(CombatData.GuardBlendWeight, CombatData.bIsGuarding ? 1.0f : 0.0f, DeltaSeconds, GuardBlendSpeed);
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
	CombatData.OverlayState = ECharacterOverlayState::Default;

	if (!ItemInstance || !ItemInstance->GetItemDef())
	{
		return;
	}

	const UItemDefinition* ItemDef = ItemInstance->GetItemDef();

	for (const auto& Pair : WeaponTagToStateMap)
	{
		if (ItemDef->HasTag(Pair.Key))
		{
			CombatData.OverlayState = Pair.Value;
		}
	}
	
}
