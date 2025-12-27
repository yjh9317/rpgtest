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
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

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
	
	bIsLandRecovery = true;

	float FallingSpeed = -GetVelocity().Z;
	if (FallingSpeed > 700.0f) 
	{
		bIsLandRecovery = true;
	}
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
		
		// RPGInputComp->BindNativeAction(InputConfig, RPGGameplayTags::Input_Action_Primary, ETriggerEvent::Started, this, &ThisClass::Input_PrimaryAction);
		// RPGInputComp->BindNativeAction(InputConfig, RPGGameplayTags::Input_Action_Primary, ETriggerEvent::Completed, this, &ThisClass::Input_PrimaryAction);
		// RPGInputComp->BindNativeAction(InputConfig, RPGGameplayTags::Input_Action_Secondary, ETriggerEvent::Started, this, &ThisClass::Input_SecondaryAction);
		// RPGInputComp->BindNativeAction(InputConfig, RPGGameplayTags::Input_Action_Secondary, ETriggerEvent::Completed, this, &ThisClass::Input_SecondaryAction);
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

bool ARPGPlayerCharacter::UpdateRotationSettings()
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!Movement) return true;

	// 1. 회전을 막아야 하는 상황인지 체크 (공중 OR 경직)
	bool bShouldLockRotation = Movement->IsFalling() || bIsLandRecovery;

	if (bShouldLockRotation)
	{
		// [회전 금지] 카메라를 돌려도 캐릭터는 가만히 있어야 함
		Movement->bUseControllerDesiredRotation = false;
		Movement->bOrientRotationToMovement = false; // 이동 방향 회전도 잠금
		bUseControllerRotationYaw = false;
	}
	else
	{
		// 2. [반대 상황] 땅에 있고 움직일 수 있는 상태 (원래 설정 복구)
        
		// (A) 조준(Aiming) 중이라면? -> 카메라 따라돌기
		if (bIsAiming) 
		{
			Movement->bUseControllerDesiredRotation = true;
			Movement->bOrientRotationToMovement = false;
		}
		// (B) 평상시(Exploration) -> 이동 방향으로 돌기 (소울류 기본)
		else
		{
			Movement->bUseControllerDesiredRotation = false;
			Movement->bOrientRotationToMovement = true;
		}
	}
	return false;
}

void ARPGPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	UpdateRotationSettings();
}

void ARPGPlayerCharacter::Input_Move(const FInputActionValue& Value)
{
	if (bIsLandRecovery)
	{
		return;
	}
	
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
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
	Jump();
}

void ARPGPlayerCharacter::Input_PrimaryAction(const FInputActionValue& Value)
{
	const bool bPressed = Value.Get<bool>();
	bIsPrimaryDown = bPressed;
	
	if (ActionComponent)
	{
		ActionComponent->ExecuteAction(RPGGameplayTags::Input_Action_Primary);
	}
}

void ARPGPlayerCharacter::Input_SecondaryAction(const FInputActionValue& Value)
{
	const bool bPressed = Value.Get<bool>();
	
	bIsGuarding = bPressed;
	bIsAiming = bPressed;
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

	if (bPressed)
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

void ARPGPlayerCharacter::SetIsAiming(bool bAiming)
{
	bIsAiming = bAiming;
}

void ARPGPlayerCharacter::SetLandRecovery(bool bState)
{
	bIsLandRecovery = bState;

	if (bIsLandRecovery)
	{
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
	}
	else
	{
		GetCharacterMovement()->bUseControllerDesiredRotation = true;
		
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