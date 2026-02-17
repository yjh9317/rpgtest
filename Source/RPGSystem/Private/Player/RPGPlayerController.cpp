// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/RPGPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Component/DialogueProviderComponent.h"
#include "Component/DialogueManagerComponent.h"
#include "DamageFloat/DamageFloatManagerComponent.h"
#include "Dialogue/RPGDialogueCameraDirectorComponent.h"
#include "HUD/RPGHUDWidget.h"
#include "Input/RPGInputFunctionLibrary.h"
#include "Interaction/Interface/InteractableInterface.h"
#include "Inventory/InventoryCoreComponent.h"
#include "Inventory/UI/PlayerInventoryWidget.h"
#include "Kismet/GameplayStatics.h"
#include "SaveSystem/RPGSaveFunctionLibrary.h"
#include "SaveSystem/Async/RPGAsyncLoadGame.h"
#include "SaveSystem/Async/RPGAsyncSaveGame.h"
#include "SaveSystem/SaveGame/InfoSaveGame.h"
#include "SaveSystem/SaveGame/RPGSettingsSaveGame.h"
#include "SaveSystem/Subsystem/RPGSaveSubsystem.h"
#include "Status/StatsComponent.h"
#include "Status/StatsViewModel.h"
#include "UI/DialogueViewModel.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

ARPGPlayerController::ARPGPlayerController()
{
	DamageFloatManagerComponent = CreateDefaultSubobject<UDamageFloatManagerComponent>(TEXT("DamageFloatManagerComponent"));
	DialogueManagerComponent = CreateDefaultSubobject<UDialogueManagerComponent>(TEXT("DialogueManagerComponent"));
	DialogueCameraDirectorComponent = CreateDefaultSubobject<URPGDialogueCameraDirectorComponent>(TEXT("DialogueCameraDirectorComponent"));
}

void ARPGPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		// 서브시스템 가져오기
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (InputMappingContext)
			{
				Subsystem->AddMappingContext(InputMappingContext, 1);
			}
		}
	}
	
	if (HUDWidgetClass && IsLocalController())
	{
		HUDWidget = CreateWidget<URPGHUDWidget>(this, HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport(); // Z-Order를 낮게 설정 (인벤토리보다 뒤에)
			HUDWidget->InitializeHUD(GetPawn());
		}
	}
	
	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<URPGHUDWidget>(this, HUDWidgetClass);
        
		UStatsViewModel* VM = NewObject<UStatsViewModel>(this);
		APawn* MyPawn = GetPawn();
		if (MyPawn)
		{
			UStatsComponent* StatsComp = MyPawn->FindComponentByClass<UStatsComponent>();
			if (StatsComp)
			{
				VM->InitializeStats(StatsComp);
			}
		}
        
		HUDWidget->SetViewModel(VM);
	}
}

void ARPGPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (InventoryAction)
		{
			EnhancedInputComponent->BindAction(InventoryAction, ETriggerEvent::Started, this, &ARPGPlayerController::ToggleInventory);
		}
		InputComponent->BindKey(FInputChord(EKeys::Two, true, true, false, false), IE_Pressed, this, &ARPGPlayerController::TestSaveGame);
		InputComponent->BindKey(FInputChord(EKeys::Three, true, true, false, false), IE_Pressed, this, &ARPGPlayerController::TestLoadGame);
	}
}

// Pawn이 변경될 때(예: 탈것 탑승, 사망 후 부활) HUD 참조 갱신 필요
void ARPGPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	if (HUDWidget)
	{
		HUDWidget->InitializeHUD(InPawn);
		// ViewModel 재연결 로직 등...
	}
}



void ARPGPlayerController::TryInteract()
{
	FVector Start, End;
	// ... 카메라 위치에서 앞쪽으로 Start/End 계산 ...

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetPawn());

	// ★ 핵심: 우리가 만든 'Interaction' 채널로 쏜다!
	// (Project Settings -> Collision에서 Interaction 채널의 이름을 알아와야 함, 보통 ECC_GameTraceChannel1 등)
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult, 
		Start, 
		End, 
		ECC_GameTraceChannel1, // <-- 이게 Interaction 채널이라고 가정
		Params
	);

	if (bHit)
	{
		
		if (IInteractableInterface* Interface = Cast<IInteractableInterface>(HitResult.GetActor()))
		{
			Interface->Interaction(this);
		}
	}
}

void ARPGPlayerController::ToggleInventory()
{
	if (!InventoryWidgetInstance && InventoryWidgetClass)
	{
		InventoryWidgetInstance = CreateWidget<UPlayerInventoryWidget>(this, InventoryWidgetClass);
	}

	if (!InventoryWidgetInstance) return;
	
	if (!InventoryWidgetInstance->IsInViewport())
	{
		APawn* ControlledPawn = GetPawn();
		UInventoryCoreComponent* InvComp = ControlledPawn ? ControlledPawn->FindComponentByClass<UInventoryCoreComponent>() : nullptr;

		if (InvComp)
		{
			// ★ 중요: 열기 직전에 초기화/갱신 호출 ★
			// 인벤토리 이름("Main")은 기획에 맞게 변경하세요.
			InventoryWidgetInstance->InitializeInventory(InvComp, FName("Main"));
		}

		// B. 화면에 추가
		InventoryWidgetInstance->AddToViewport();

		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(InventoryWidgetInstance->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
	else
	{
		// === [닫기 로직] ===

		// A. 화면에서 제거
		InventoryWidgetInstance->RemoveFromParent();

		// B. 입력 모드 복구 (게임 전용)
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		bShowMouseCursor = false;
	}
}

void ARPGPlayerController::TestSaveGame()
{
	int32 SaveFlags = (1 << (int32)ESaveTypeFlags::SF_Level) | (1 << (int32)ESaveTypeFlags::SF_Player);

	// 2. 서브시스템에 현재 슬롯 이름 설정 (Async Task가 이 이름을 참조함)
	URPGSaveSubsystem* SaveSystem = GetGameInstance()->GetSubsystem<URPGSaveSubsystem>();
	if (SaveSystem)
	{
		SaveSystem->SetCurrentSaveGameName(TEXT("Slot_01"));
	}

	// 3. 비동기 저장 태스크 생성 및 실행
	// (이 태스크가 내부적으로 Prepare -> Save Player -> Save Level 과정을 모두 수행합니다)
	URPGAsyncSaveGame* SaveTask = URPGAsyncSaveGame::AsyncSaveActors(this, SaveFlags);
    
	if (SaveTask)
	{
		SaveTask->OnCompleted.AddDynamic(this, &ARPGPlayerController::HandleSaveCompleted);
		SaveTask->OnFailed.AddDynamic(this, &ARPGPlayerController::HandleSaveFailed);
		SaveTask->Activate();
       
		UE_LOG(LogTemp, Log, TEXT("Save Task Started..."));
	}
}

void ARPGPlayerController::TestLoadGame()
{
	URPGSaveSubsystem* SaveSystem = GetGameInstance()->GetSubsystem<URPGSaveSubsystem>();
	if (SaveSystem)
	{
		// 1. 슬롯 이름 설정
		SaveSystem->SetCurrentSaveGameName(TEXT("Slot_01"));

		// 2. 로드 플래그 설정 (플레이어 + 레벨)
		// LF_Player: 플레이어 위치, 스탯 등
		// LF_Level: 레벨 내 상자, 문, 적 상태 등
		int32 LoadFlags = (1 << (int32)ELoadTypeFlags::LF_Player) | (1 << (int32)ELoadTypeFlags::LF_Level);

		// 3. 비동기 로드 실행
		// bFullReload = true: 기존 상태를 초기화하고 덮어씌움 (보통 true 권장)
		URPGAsyncLoadGame* LoadTask = URPGAsyncLoadGame::AsyncLoadActors(this, LoadFlags, true);
        
		if (LoadTask)
		{
			LoadTask->OnCompleted.AddDynamic(this, &ARPGPlayerController::HandleLoadCompleted);
			LoadTask->Activate(); // [중요] 내부적으로 TryLoad -> Apply Data 과정을 모두 수행함
            
			UE_LOG(LogTemp, Log, TEXT("Async Load Task Started..."));
		}
	}
}

void ARPGPlayerController::ApplySavedInputSettings()
{
	URPGSettingsSaveGame* Settings = Cast<URPGSettingsSaveGame>(UGameplayStatics::LoadGameFromSlot("GlobalSettings", 0));
	if (!Settings) return; // 저장된 게 없으면 기본값 사용

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (!Subsystem || !InputMappingContext) return;

	// 3. 저장된 WASD(Axis) 오버라이드 적용
	for (const auto& Pair : Settings->AxisKeyOverrides)
	{
		FKey OriginalKey = Pair.Key;
		FKey NewKey = Pair.Value;

		const UInputAction* TargetAction = nullptr;
    
		for (const FEnhancedActionKeyMapping& Mapping : InputMappingContext->GetMappings())
		{
			if (Mapping.Key == OriginalKey)
			{
				TargetAction = Mapping.Action;
				break;
			}
		}

		// 대상 액션이 있다면, 라이브러리 함수를 통해 안전하게 키를 교체합니다.
		// (RebindKey 함수 내부에서 Unmap -> MapKey 과정을 처리해줍니다)
		if (TargetAction)
		{
			URPGInputFunctionLibrary::RebindKey(this, InputMappingContext, TargetAction, OriginalKey, NewKey);
		}
	}

	// 4. 일반 액션 매핑 적용 (ActionMappings 순회)
	// ... (유사한 로직으로 구현)

	// 5. 최종 갱신
	Subsystem->AddMappingContext(InputMappingContext, 1);
}

void ARPGPlayerController::PreSave(FObjectPreSaveContext SaveContext)
{
	OtherPawnViewRot = GetControlRotation();
}

void ARPGPlayerController::ActorLoaded_Implementation()
{
	if (OtherPawnRef.IsValid())
	{
		StoreDefaultPawn();
		PossessOtherPawn(OtherPawnRef.Get());
		SetControlRotation(OtherPawnViewRot);
	}
}

void ARPGPlayerController::StoreDefaultPawn()
{
	DefaultPawnRef = UGameplayStatics::GetPlayerPawn(GetWorld(),0);
}

void ARPGPlayerController::PossessOtherPawn(APawn* OtherPawn)
{
	StoreDefaultPawn();
	Possess(OtherPawn);
	OtherPawnRef = OtherPawn;
}

void ARPGPlayerController::PossessOriginalPawn()
{
	if (DefaultPawnRef.IsValid())
	{
		Possess(DefaultPawnRef.Get());
		OtherPawnRef = nullptr;
	}
}
void ARPGPlayerController::HandleSaveCompleted()
{
	UE_LOG(LogTemp, Log, TEXT("저장 성공!"));
}

void ARPGPlayerController::HandleSaveFailed()
{
	UE_LOG(LogTemp, Error, TEXT("저장 실패!"));
}

void ARPGPlayerController::HandleLoadCompleted()
{
	UE_LOG(LogTemp, Warning, TEXT("=== GAME LOAD COMPLETE ==="));
}

#pragma region DialogueInterface

void ARPGPlayerController::StartDialogue_Implementation(UDialogue* Dialogue, AActor* NPCActor)
{
	if (!DialogueManagerComponent || !NPCActor)
	{
		return;
	}

	if (UDialogueProviderComponent* Provider = NPCActor->FindComponentByClass<UDialogueProviderComponent>())
	{
		DialogueManagerComponent->StartDialogue(Provider);
	}
}

void ARPGPlayerController::EndDialogue_Implementation()
{
	if (DialogueManagerComponent)
	{
		DialogueManagerComponent->EndDialogue(true);
	}
}

void ARPGPlayerController::DisplayNPCDialogue_Implementation(const FDialogueNode& Node, AActor* NPCActor)
{
	FaceNPC_Implementation(NPCActor);
}

void ARPGPlayerController::DisplayPlayerOptions_Implementation(const TArray<FDialogueNode>& Options)
{
}

void ARPGPlayerController::HideDialogueUI_Implementation()
{
}

void ARPGPlayerController::OnPlayerSelectOption_Implementation(int32 NodeId)
{
	if (!DialogueManagerComponent)
	{
		return;
	}

	if (UDialogueViewModel* DialogueViewModel = DialogueManagerComponent->GetDialogueViewModel())
	{
		DialogueViewModel->SelectOption(NodeId);
	}
}

bool ARPGPlayerController::IsInDialogue_Implementation() const
{
	return DialogueManagerComponent && DialogueManagerComponent->IsInDialogue();
}

bool ARPGPlayerController::CanStartDialogue_Implementation() const
{
	return !IsInDialogue_Implementation() && GetPawn() != nullptr;
}

void ARPGPlayerController::SwitchToDialogueCamera_Implementation(AActor* NPCActor)
{
	if (DialogueCameraDirectorComponent)
	{
		DialogueCameraDirectorComponent->EnterDialogueCamera(NPCActor);
	}
}

void ARPGPlayerController::RestoreGameplayCamera_Implementation()
{
	if (DialogueCameraDirectorComponent)
	{
		DialogueCameraDirectorComponent->ExitDialogueCamera();
	}
}

void ARPGPlayerController::SetDialogueInputMode_Implementation(bool bDialogueMode)
{
	if (bDialogueMode)
	{
		FInputModeGameAndUI InputMode;
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
	else
	{
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		bShowMouseCursor = false;
	}
}

void ARPGPlayerController::FaceNPC_Implementation(AActor* NPCActor)
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn || !NPCActor)
	{
		return;
	}

	FVector ToNPC = NPCActor->GetActorLocation() - ControlledPawn->GetActorLocation();
	ToNPC.Z = 0.f;
	if (!ToNPC.IsNearlyZero())
	{
		ControlledPawn->SetActorRotation(ToNPC.Rotation());
	}
}

void ARPGPlayerController::SetCharacterMovementEnabled_Implementation(bool bEnabled)
{
	ACharacter* ControlledCharacter = Cast<ACharacter>(GetPawn());
	if (!ControlledCharacter)
	{
		return;
	}

	UCharacterMovementComponent* MoveComponent = ControlledCharacter->GetCharacterMovement();
	if (!MoveComponent)
	{
		return;
	}

	if (bEnabled)
	{
		MoveComponent->SetMovementMode(MOVE_Walking);
	}
	else
	{
		MoveComponent->DisableMovement();
	}
}

void ARPGPlayerController::AddToDialogueHistory_Implementation(const FDialogueNode& Node, bool bIsPlayerChoice)
{
}

bool ARPGPlayerController::CanSkipDialogue_Implementation() const
{
	return true;
}

void ARPGPlayerController::ApplyDialogueCinematicCue_Implementation(const FDialogueCinematicCue& Cue, AActor* NPCActor)
{
	if (DialogueCameraDirectorComponent)
	{
		DialogueCameraDirectorComponent->ApplyCue(Cue, NPCActor);
	}
}

#pragma endregion DialogueInterface
