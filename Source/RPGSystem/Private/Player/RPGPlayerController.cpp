// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/RPGPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "HUD/RPGHUDWidget.h"
#include "Interaction/Interface/InteractableInterface.h"
#include "Inventory/InventoryCoreComponent.h"
#include "Inventory/UI/PlayerInventoryWidget.h"
#include "Kismet/GameplayStatics.h"
#include "SaveSystem/RPGSaveFunctionLibrary.h"
#include "SaveSystem/Async/RPGAsyncLoadGame.h"
#include "SaveSystem/Async/RPGAsyncSaveGame.h"
#include "SaveSystem/SaveGame/InfoSaveGame.h"
#include "SaveSystem/Subsystem/RPGSaveSubsystem.h"
#include "Status/StatsComponent.h"
#include "Status/StatsViewModel.h"

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
		HUDWidgetInstance = CreateWidget<URPGHUDWidget>(this, HUDWidgetClass);
		if (HUDWidgetInstance)
		{
			HUDWidgetInstance->AddToViewport(); // Z-Order를 낮게 설정 (인벤토리보다 뒤에)
			HUDWidgetInstance->InitializeHUD(GetPawn());
		}
	}
	
	// 2. ViewModel 연결 (StatsComponent가 Pawn에 있다고 가정)
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn)
	{
		UStatsComponent* StatsComp = ControlledPawn->FindComponentByClass<UStatsComponent>();
		if (StatsComp)
		{
			StatsViewModel = NewObject<UStatsViewModel>(this);
			StatsViewModel->Initialize(StatsComp);

			if (HUDWidgetInstance)
			{
				HUDWidgetInstance->BindStatsModel(StatsViewModel);
			}
		}
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
	
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->InitializeHUD(InPawn);
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
