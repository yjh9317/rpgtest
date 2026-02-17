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
	
	if (InteractionComponent)
	{
		InteractionComponent->InitializeInteraction(Cast<APlayerController>(GetController()));
	}
}

void ARPGPlayerCharacter::UpdateRotationSettings()
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!Movement) return;

	// 1. ???읈??筌띾맩釉????롫뮉 ?怨뱀넺?紐? 筌ｋ똾寃?(?⑤벊夷?OR 野껋럩彛?
	bool bShouldLockRotation = Movement->IsFalling() || bIsLandRecovery;

	if (bShouldLockRotation)
	{
		// [???읈 疫뀀뜆?]
		Movement->bUseControllerDesiredRotation = false;
		Movement->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = false;
	}
	else
	{
		// 2. [???읈 ??됱뒠] ??녿퓠 ??뉙???筌욊낯??????덈뮉 ?怨밴묶
       
		// (A) 鈺곌퀣?(Aiming) 餓?-> 燁삳?李??獄쎻뫚堉??곗쨮 ???읈 (Strafe)
		if (bIsAiming) 
		{
			Movement->bUseControllerDesiredRotation = true;
			Movement->bOrientRotationToMovement = false;
			Movement->RotationRate = FRotator(0.0f, 720.0f, 0.0f); // 鈺곌퀣? ????쥓?????읈
		}
		// (B) ??깃맒??Exploration) -> ??猷?獄쎻뫚堉??곗쨮 ???읈 (Orient)
		else
		{
			Movement->bUseControllerDesiredRotation = false;
			Movement->bOrientRotationToMovement = true;
			Movement->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ??猷????봔??뺤쑎?????읈
		}
	}
}

void ARPGPlayerCharacter::Input_AbilityAction(const FInputActionValue& Value, FGameplayTag ActionTag)
{
	EActionInputPhase InputPhase = EActionInputPhase::Held;
	float InputValue = 0.0f;
	bool bShouldDispatch = false;
	const EInputActionValueType ValueType = Value.GetValueType();

	if (ValueType == EInputActionValueType::Boolean)
	{
		const bool bCurrent = Value.Get<bool>();
		const bool bPrevious = AbilityDigitalInputStates.FindRef(ActionTag);

		if (bCurrent && !bPrevious)
		{
			InputPhase = EActionInputPhase::Pressed;
			InputValue = 1.0f;
			bShouldDispatch = true;
		}
		else if (bCurrent && bPrevious)
		{
			InputPhase = EActionInputPhase::Held;
			InputValue = 1.0f;
			bShouldDispatch = true;
		}
		else if (!bCurrent && bPrevious)
		{
			InputPhase = EActionInputPhase::Released;
			InputValue = 0.0f;
			bShouldDispatch = true;
		}

		AbilityDigitalInputStates.Add(ActionTag, bCurrent);
	}
	else if (ValueType == EInputActionValueType::Axis1D)
	{
		const float Axis = Value.Get<float>();
		const float PrevAxis = AbilityAxisInputStates.FindRef(ActionTag);
		const bool bWasActive = !FMath::IsNearlyZero(PrevAxis);
		const bool bIsActive = !FMath::IsNearlyZero(Axis);

		if (bIsActive && !bWasActive)
		{
			InputPhase = EActionInputPhase::Pressed;
			InputValue = Axis;
			bShouldDispatch = true;
		}
		else if (bIsActive)
		{
			InputPhase = EActionInputPhase::Held;
			InputValue = Axis;
			bShouldDispatch = true;
		}
		else if (!bIsActive && bWasActive)
		{
			InputPhase = EActionInputPhase::Released;
			InputValue = 0.0f;
			bShouldDispatch = true;
		}

		AbilityAxisInputStates.Add(ActionTag, Axis);
	}
	else
	{
		InputPhase = EActionInputPhase::Held;
		InputValue = 1.0f;
		bShouldDispatch = true;
	}

	if (ActionComponent && bShouldDispatch)
	{
		ActionComponent->ExecuteAction(ActionTag, InputPhase, InputValue);
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

	// ??쑵六? 疫꿸퀡??怨몄몵嚥???롮몵嚥?筌욊쑵六?+ A/D???ル슣????猷? W/S(IA_Ascend, IA_Descend)???怨밸릭 ??낆젾
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

	// ??륁겫: 筌띾뜆???燁삳?李?? 獄쎻뫚堉?疫꿸퀣???곗쨮 ?袁⑹춭/筌γ볝늺 ??猷?
	const FRotator ControlRotation = Controller->GetControlRotation();
	const FVector ForwardDirection = ControlRotation.Vector();
	const FVector RightDirection = FRotationMatrix(ControlRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ARPGPlayerCharacter::ApplyRidingMovement(const FVector2D& MovementVector)
{
	// Riding?? Strafe揶쎛 ?袁⑤빍??鈺곌퀬堉?+ ?袁れ뜎 ??얜즲(筌?鈺곌퀣???癒?덱)
	RideSteerInput = FMath::Clamp(MovementVector.X, -1.0f, 1.0f);
	RideThrottleInput = FMath::Clamp(MovementVector.Y, -1.0f, 1.0f);
}

void ARPGPlayerCharacter::UpdateRidingMovement(float DeltaSeconds)
{
	if (Controller == nullptr)
	{
		return;
	}

	// 1) A/D?????읈(鈺곌퀬堉?
	const float DeltaYaw = RideSteerInput * RideTurnRateDegPerSec * DeltaSeconds;
	AddActorWorldRotation(FRotator(0.0f, DeltaYaw, 0.0f));

	// 2) W/S??揶쎛揶쏅Ŋ??
	const float TargetSpeed = RideThrottleInput * RideMaxForwardSpeed;
	const float InterpRate = (FMath::Abs(TargetSpeed) > FMath::Abs(RideCurrentSpeed)) ? RideAcceleration : RideBraking;
	RideCurrentSpeed = FMath::FInterpTo(RideCurrentSpeed, TargetSpeed, DeltaSeconds, InterpRate / FMath::Max(1.0f, RideMaxForwardSpeed));

	// 3) ?袁⑹삺 獄쏅뗀?よ퉪???獄쎻뫚堉??곗쨮 ?袁⑹춭/?袁⑹춭
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

	float TargetSpeed = 500.0f; // 疫꿸퀡????얜즲 (Jog)

	if (bIsAiming)
	{
		TargetSpeed = 250.0f; // 鈺곌퀣? ???癒?봺野?
	}
	else if (bIsSprint)
	{
		TargetSpeed = 800.0f; // ???곫묾???얜즲
	}
	else if (bIsWalking)
	{
		TargetSpeed = 200.0f; // 椰꾨０由???얜즲
	}

	// ??쇱젫 ?怨몄뒠
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
		// ???곫묾??癒곕늄
		GetCharacterMovement()->BrakingDecelerationFalling = 0.0f;
	}
	else
	{
		// 椰꾨０由??癒곕늄
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
		ActionComponent->ExecuteAction(
			RPGGameplayTags::Input_Action_Primary,
			bVal ? EActionInputPhase::Pressed : EActionInputPhase::Released,
			bVal ? 1.0f : 0.0f
		);
	}
}

void ARPGPlayerCharacter::Input_SecondaryAction(const FInputActionValue& Value)
{
	bool bVal = Value.Get<bool>();
	UE_LOG(LogTemp, Warning, TEXT("Right Input Value: %s"), bVal ? TEXT("TRUE") : TEXT("FALSE"));
	bIsSecondaryDown = bVal;
	bIsGuarding = bVal;
	SetIsAiming(bVal);

	if (!ActionComponent)
	{
		return;
	}

	const bool bHandled = ActionComponent->ExecuteAction(
		RPGGameplayTags::Input_Action_Secondary,
		bVal ? EActionInputPhase::Pressed : EActionInputPhase::Released,
		bVal ? 1.0f : 0.0f
	);

	// Backward compatibility for legacy actions that still rely on explicit interrupt on release.
	if (!bVal && !bHandled && ActionComponent->IsActionActive(RPGGameplayTags::Input_Action_Secondary))
	{
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
		// ??낆젾 鈺곌퀗援?Hold ?袁⑥┷ ?????겸뫗???뤿????嚥???쇱젫 ?怨뱀깈?臾믪뒠 ??쎈뻬
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
