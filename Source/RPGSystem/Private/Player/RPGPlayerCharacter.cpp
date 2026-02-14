// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/RPGPlayerCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "RPGSystemGameplayTags.h"
#include "Camera/Component/RPGSpringArmComponentBase.h"
#include "Input/CustomInputComponent.h"
#include "Interaction/InteractableComponent.h"
#include "Interaction/InteractionComponent.h"
#include "Inventory/InventoryCoreComponent.h"


//////////////////////////////////////////////////////////////////////////
// ARPGS_PlayerCharacter

ARPGPlayerCharacter::ARPGPlayerCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 0.0f;
	GetCharacterMovement()->AirControl = 0.2f; 

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<URPGSpringArmComponentBase>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	
	ActionComponent = CreateDefaultSubobject<UActionComponent>(TEXT("ActionComponent"));
	InventoryCoreComponent = CreateDefaultSubobject<UInventoryCoreComponent>(TEXT("InventoryCoreComponent"));
	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));
}


void ARPGPlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f; 
	bIsLandRecovery = true;

	float FallingSpeed = -GetVelocity().Z;
	if (FallingSpeed > 700.0f) 
	{
		bIsLandRecovery = true;
	}
}

void ARPGPlayerCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
	UpdateRotationSettings();
}

void ARPGPlayerCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}


void ARPGPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	UCustomInputComponent* RPGInputComp = CastChecked<UCustomInputComponent>(PlayerInputComponent);

	if (RPGInputComp && InputConfig)
	{
		RPGInputComp->BindNativeAction(InputConfig, RPGGameplayTags::Input_Action_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move);
		RPGInputComp->BindNativeAction(InputConfig, RPGGameplayTags::Input_Action_Look, ETriggerEvent::Triggered, this, &ThisClass::Input_Look);
		
		RPGInputComp->BindNativeAction(InputConfig, RPGGameplayTags::Input_Action_Jump, ETriggerEvent::Started, this, &ThisClass::Input_Jump);
		RPGInputComp->BindNativeAction(InputConfig, RPGGameplayTags::Input_Action_Jump, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		
		RPGInputComp->BindNativeAction(InputConfig, RPGGameplayTags::Input_Action_Interact, ETriggerEvent::Started, this, &ThisClass::Input_Interact);

		RPGInputComp->BindNativeAction(InputConfig, RPGGameplayTags::Input_Action_Crouch, ETriggerEvent::Started, this, &ThisClass::ToggleCrouch);
		RPGInputComp->BindNativeAction(InputConfig, RPGGameplayTags::Input_Action_Sprint, ETriggerEvent::Started, this, &ThisClass::ToggleSprint);
		RPGInputComp->BindNativeAction(InputConfig, RPGGameplayTags::Input_Action_Walk, ETriggerEvent::Started, this, &ThisClass::ToggleWalk);
		
		RPGInputComp->BindNativeAction(InputConfig, RPGGameplayTags::Input_Action_Primary, ETriggerEvent::Started, this, &ThisClass::Input_PrimaryAction);
		RPGInputComp->BindNativeAction(InputConfig, RPGGameplayTags::Input_Action_Primary, ETriggerEvent::Completed, this, &ThisClass::Input_PrimaryAction);
		RPGInputComp->BindNativeAction(InputConfig, RPGGameplayTags::Input_Action_Secondary, ETriggerEvent::Started, this, &ThisClass::Input_SecondaryAction);
		RPGInputComp->BindNativeAction(InputConfig, RPGGameplayTags::Input_Action_Secondary, ETriggerEvent::Completed, this, &ThisClass::Input_SecondaryAction);
		RPGInputComp->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityAction);
		RPGInputComp->BindNativeAction(InputConfig, RPGGameplayTags::Input_Action_Ascend, ETriggerEvent::Triggered, this, &ThisClass::Input_Ascend, false);
		RPGInputComp->BindNativeAction(InputConfig, RPGGameplayTags::Input_Action_Ascend, ETriggerEvent::Completed, this, &ThisClass::Input_Ascend, false);
		RPGInputComp->BindNativeAction(InputConfig, RPGGameplayTags::Input_Action_Descend, ETriggerEvent::Triggered, this, &ThisClass::Input_Descend, false);
		RPGInputComp->BindNativeAction(InputConfig, RPGGameplayTags::Input_Action_Descend, ETriggerEvent::Completed, this, &ThisClass::Input_Descend, false);
	}
}

void ARPGPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (InventoryCoreComponent)
	{
		InventoryCoreComponent->InitializeInventory();
	}
	
	if (InteractionComponent)
	{
		InteractionComponent->InitializeInteraction(Cast<APlayerController>(GetController()));
	}
}

void ARPGPlayerCharacter::UpdateRotationSettings()
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!Movement) return;

	// 1. 회전을 막아야 하는 상황인지 체크 (공중 OR 경직)
	bool bShouldLockRotation = Movement->IsFalling() || bIsLandRecovery;

	if (bShouldLockRotation)
	{
		// [회전 금지]
		Movement->bUseControllerDesiredRotation = false;
		Movement->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = false;
	}
	else
	{
		// 2. [회전 허용] 땅에 있고 움직일 수 있는 상태
       
		// (A) 조준(Aiming) 중 -> 카메라 방향으로 회전 (Strafe)
		if (bIsAiming) 
		{
			Movement->bUseControllerDesiredRotation = true;
			Movement->bOrientRotationToMovement = false;
			Movement->RotationRate = FRotator(0.0f, 720.0f, 0.0f); // 조준 시 빠른 회전
		}
		// (B) 평상시(Exploration) -> 이동 방향으로 회전 (Orient)
		else
		{
			Movement->bUseControllerDesiredRotation = false;
			Movement->bOrientRotationToMovement = true;
			Movement->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // 이동 시 부드러운 회전
		}
	}
}

void ARPGPlayerCharacter::Input_AbilityAction(const FInputActionValue& Value, FGameplayTag ActionTag)
{
	if (ActionComponent)
	{
		ActionComponent->ExecuteAction(ActionTag);
	}
}

void ARPGPlayerCharacter::Input_Ascend(const FInputActionValue& Value)
{
	const float Axis = Value.Get<float>();
	if (ResolveMovementModeForInput() != EPlayerMovementMode::Fly)
	{
		FlyingAscendInput = 0.0f;
		return;
	}
	FlyingAscendInput = FMath::Clamp(Axis, 0.0f, 1.0f);
}

void ARPGPlayerCharacter::Input_Descend(const FInputActionValue& Value)
{
	const float Axis = Value.Get<float>();
	if (ResolveMovementModeForInput() != EPlayerMovementMode::Fly)
	{
		FlyingDescendInput = 0.0f;
		return;
	}
	FlyingDescendInput = FMath::Clamp(Axis, 0.0f, 1.0f);
}

void ARPGPlayerCharacter::ApplyGroundMovement(const FVector2D& MovementVector)
{
	if (Controller == nullptr)
	{
		return;
	}

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	
	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ARPGPlayerCharacter::ApplyFlyingMovement(const FVector2D& MovementVector)
{
	if (Controller == nullptr)
	{
		return;
	}

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// 비행: 기본적으로 앞으로 진행 + A/D는 좌우 이동, W/S(IA_Ascend, IA_Descend)는 상하 입력
	FlyingVerticalInput = FMath::Clamp(FlyingAscendInput - FlyingDescendInput, -1.0f, 1.0f);
	AddMovementInput(ForwardDirection, FlyingForwardSpeedScale);
	AddMovementInput(RightDirection, MovementVector.X);
	AddMovementInput(FVector::UpVector, FlyingVerticalInput);
}

void ARPGPlayerCharacter::ApplySwimmingMovement(const FVector2D& MovementVector)
{
	if (Controller == nullptr)
	{
		return;
	}

	// 수영: 마우스(카메라) 방향 기준으로 전진/측면 이동
	const FRotator ControlRotation = Controller->GetControlRotation();
	const FVector ForwardDirection = ControlRotation.Vector();
	const FVector RightDirection = FRotationMatrix(ControlRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ARPGPlayerCharacter::ApplyRidingMovement(const FVector2D& MovementVector)
{
	// Riding은 Strafe가 아니라 조향 + 전후 속도(말 조작 느낌)
	RideSteerInput = FMath::Clamp(MovementVector.X, -1.0f, 1.0f);
	RideThrottleInput = FMath::Clamp(MovementVector.Y, -1.0f, 1.0f);
}

void ARPGPlayerCharacter::UpdateRidingMovement(float DeltaSeconds)
{
	if (Controller == nullptr)
	{
		return;
	}

	// 1) A/D는 회전(조향)
	const float DeltaYaw = RideSteerInput * RideTurnRateDegPerSec * DeltaSeconds;
	AddActorWorldRotation(FRotator(0.0f, DeltaYaw, 0.0f));

	// 2) W/S는 가감속
	const float TargetSpeed = RideThrottleInput * RideMaxForwardSpeed;
	const float InterpRate = (FMath::Abs(TargetSpeed) > FMath::Abs(RideCurrentSpeed)) ? RideAcceleration : RideBraking;
	RideCurrentSpeed = FMath::FInterpTo(RideCurrentSpeed, TargetSpeed, DeltaSeconds, InterpRate / FMath::Max(1.0f, RideMaxForwardSpeed));

	// 3) 현재 바라보는 방향으로 전진/후진
	AddMovementInput(GetActorForwardVector(), RideCurrentSpeed / FMath::Max(1.0f, RideMaxForwardSpeed));
}

EPlayerMovementMode ARPGPlayerCharacter::ResolveMovementModeForInput() const
{
	if (ForcedMovementMode == EPlayerMovementMode::Fly ||
		ForcedMovementMode == EPlayerMovementMode::Swimming ||
		ForcedMovementMode == EPlayerMovementMode::Riding)
	{
		return ForcedMovementMode;
	}

	if (const UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		if (Movement->IsSwimming())
		{
			return EPlayerMovementMode::Swimming;
		}
		if (Movement->MovementMode == MOVE_Flying)
		{
			return EPlayerMovementMode::Fly;
		}
	}

	return EPlayerMovementMode::Jog;
}

void ARPGPlayerCharacter::BindInputActions(class UCustomInputComponent* RPGInputComp)
{
}

void ARPGPlayerCharacter::UpdateMovementSpeed()
{
	if (!GetCharacterMovement()) return;

	float TargetSpeed = 500.0f; // 기본 속도 (Jog)

	if (bIsAiming)
	{
		TargetSpeed = 250.0f; // 조준 시 느리게
	}
	else if (bIsSprint)
	{
		TargetSpeed = 800.0f; // 달리기 속도
	}
	else if (bIsWalking)
	{
		TargetSpeed = 200.0f; // 걷기 속도
	}

	// 실제 적용
	GetCharacterMovement()->MaxWalkSpeed = TargetSpeed;
}

void ARPGPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (ResolveMovementModeForInput() == EPlayerMovementMode::Riding)
	{
		UpdateRidingMovement(DeltaSeconds);
	}
	
}

void ARPGPlayerCharacter::Input_Move(const FInputActionValue& Value)
{
	if (bIsLandRecovery)
	{
		return;
	}
	
	const FVector2D MovementVector = Value.Get<FVector2D>();

	switch (ResolveMovementModeForInput())
	{
	case EPlayerMovementMode::Fly:
		ApplyFlyingMovement(MovementVector);
		break;
	case EPlayerMovementMode::Swimming:
		ApplySwimmingMovement(MovementVector);
		break;
	case EPlayerMovementMode::Riding:
		ApplyRidingMovement(MovementVector);
		break;
	default:
		ApplyGroundMovement(MovementVector);
		break;
	}
}

void ARPGPlayerCharacter::Input_Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ARPGPlayerCharacter::Input_Interact(const FInputActionValue& Value)
{
	if (InteractionComponent)
	{
		InteractionComponent->TryInteract();
	}
}

void ARPGPlayerCharacter::Input_Jump(const FInputActionValue& Value)
{
	if (bIsLandRecovery)
	{
		return;
	}
	
	if (bIsSprint)
	{
		// 달리기 점프
		GetCharacterMovement()->BrakingDecelerationFalling = 0.0f;
	}
	else
	{
		// 걷기 점프
		GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	}
	
	Jump();
}

void ARPGPlayerCharacter::Input_PrimaryAction(const FInputActionValue& Value)
{
	bool bVal = Value.Get<bool>();
	UE_LOG(LogTemp, Warning, TEXT("Left Input Value: %s"), bVal ? TEXT("TRUE") : TEXT("FALSE"));
	bIsPrimaryDown = bVal;
	
	if (ActionComponent)
	{
		ActionComponent->ExecuteAction(RPGGameplayTags::Input_Action_Primary);
	}
}

void ARPGPlayerCharacter::Input_SecondaryAction(const FInputActionValue& Value)
{
	bool bVal = Value.Get<bool>();
	UE_LOG(LogTemp, Warning, TEXT("Right Input Value: %s"), bVal ? TEXT("TRUE") : TEXT("FALSE"));
	bIsGuarding = !bIsGuarding;
	bool Aiming = !bIsAiming;
	SetIsAiming(Aiming);
	// FGameplayTag WeaponTag = GetCurrentWeaponTag(); 
	// if (WeaponTag.MatchesTag(RPGGameplayTags::Item_Weapon_Bow))
	// {
	// 	// 활이면 -> 조준(Aiming)
	// 	SetIsAiming(bPressed);
	// }
	// else if (WeaponTag.MatchesTag(RPGGameplayTags::Item_Weapon_Shield))
	// {
	// 	// 방패면 -> 방어(Guarding)
	// 	bIsGuarding = bPressed;
	// }

	if (bVal)
	{
		// [Start] 액션 시작 요청
		ActionComponent->ExecuteAction(RPGGameplayTags::Input_Action_Secondary);
	}
	else
	{
		// [Stop] 액션 중단 요청
		ActionComponent->InterruptAction(RPGGameplayTags::Input_Action_Secondary);
	}
}

void ARPGPlayerCharacter::ToggleWalk()
{
	bIsWalking = !bIsWalking;

}

void ARPGPlayerCharacter::ToggleCrouch()
{
	bIsCrouch = !bIsCrouch;

}

void ARPGPlayerCharacter::ToggleSprint()
{
	bIsSprint = !bIsSprint;

}

void ARPGPlayerCharacter::ApplyInputPreset(UDataAsset_InputConfig* NewInputConfig,
	UInputMappingContext* NewMappingContext)
{
}

void ARPGPlayerCharacter::SetIsAiming(bool bAiming)
{
	if (bIsAiming == bAiming) return;
	bIsAiming = bAiming;
	
	UpdateRotationSettings();
}

void ARPGPlayerCharacter::SetLandRecovery(bool bState)
{
	bIsLandRecovery = bState;
	
	UpdateRotationSettings();
}

void ARPGPlayerCharacter::SetForcedMovementMode(EPlayerMovementMode NewMode)
{
	ForcedMovementMode = NewMode;
	if (ForcedMovementMode != EPlayerMovementMode::Riding)
	{
		RideCurrentSpeed = 0.0f;
		RideSteerInput = 0.0f;
		RideThrottleInput = 0.0f;
		FlyingAscendInput = 0.0f;
		FlyingDescendInput = 0.0f;
		FlyingVerticalInput = 0.0f;
	}
}


void ARPGPlayerCharacter::StartInteractionWithObject(UInteractableComponent* InteractableComponent)
{
	if (InteractableComponent)
	{
		// 입력 조건(Hold 완료 등)이 충족되었으므로 실제 상호작용 실행
		InteractableComponent->OnInteraction(this);
	}
}

void ARPGPlayerCharacter::EndInteractionWithObject(UInteractableComponent* InteractableComponent) {}
void ARPGPlayerCharacter::RemoveInteractionWithObject(UInteractableComponent* InteractableComponent) {}
void ARPGPlayerCharacter::InitializeInteractionWithObject(UInteractableComponent* InteractableComponent) {}

AActor* ARPGPlayerCharacter::GetCurrentInteractableObject()
{
	if (InteractionComponent)
	{
		UInteractableComponent* TargetComp = InteractionComponent->GetCurrentInteractable();
		if (TargetComp)
		{
			return TargetComp->GetOwner();
		}
	}
    
	return nullptr;
}