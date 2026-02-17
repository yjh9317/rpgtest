// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/InteractableComponent.h"

#include "EnhancedInputSubsystems.h"
#include "Event/GlobalEventHandler.h"
#include "GameplayTagAssetInterface.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "RPGSystemCollisionChannels.h"
#include "RPGSystemGameplayTags.h"
#include "Components/WidgetComponent.h"
#include "Interaction/Interface/InteractableInterface.h"
#include "Interaction/Interface/InteractorInterface.h"
#include "Interaction/UI/InteractionPromptWidget.h"
#include "Quest/Components/QuestManagerComponent.h"

namespace
{
	UQuestManagerComponent* ResolveQuestManagerFromInteractor(AActor* Interactor)
	{
		if (!Interactor)
		{
			return nullptr;
		}

		if (UQuestManagerComponent* QuestManager = Interactor->FindComponentByClass<UQuestManagerComponent>())
		{
			return QuestManager;
		}

		if (const APawn* Pawn = Cast<APawn>(Interactor))
		{
			if (AController* Controller = Pawn->GetController())
			{
				if (UQuestManagerComponent* QuestManager = Controller->FindComponentByClass<UQuestManagerComponent>())
				{
					return QuestManager;
				}
			}
		}
		else if (const AController* Controller = Cast<AController>(Interactor))
		{
			if (APawn* ControlledPawn = Controller->GetPawn())
			{
				if (UQuestManagerComponent* QuestManager = ControlledPawn->FindComponentByClass<UQuestManagerComponent>())
				{
					return QuestManager;
				}
			}
		}

		return nullptr;
	}

	FGameplayTag ResolveInteractionTag(AActor* OwnerActor, const FGameplayTag& ConfiguredTag)
	{
		if (ConfiguredTag.IsValid())
		{
			return ConfiguredTag;
		}

		if (const IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(OwnerActor))
		{
			FGameplayTagContainer OwnedTags;
			TagInterface->GetOwnedGameplayTags(OwnedTags);
			for (const FGameplayTag& OwnedTag : OwnedTags)
			{
				return OwnedTag;
			}
		}

		if (OwnerActor && OwnerActor->Tags.Num() > 0)
		{
			return FGameplayTag::RequestGameplayTag(OwnerActor->Tags[0], false);
		}

		return FGameplayTag();
	}
}

// Sets default values for this component's properties
UInteractableComponent::UInteractableComponent(const FObjectInitializer& ObjectInitializer)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UInteractableComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

// Called when the game starts
void UInteractableComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* OwnerActor = GetOwner();
	if(!OwnerActor) return;
	
	IInteractableInterface* InteractableOwner = Cast<IInteractableInterface>(OwnerActor);
	if (InteractableOwner)
	{
		InteractableOwner->Initialize();
	}
}

void UInteractableComponent::AssociatedActorInteraction(AActor* Interactor)
{
	CurrentInteractor = Interactor;

	TArray<AActor*> Actors;
	AssociatedInteractableActors.GetKeys(Actors);

	for(const auto* Actor : Actors)
	{
		if(!Actor) continue;

		if (UInteractableComponent* InteractableComponent = Actor->FindComponentByClass<UInteractableComponent>())
		{
			InteractableComponent->CheckForInteractionWithAssociate(Interactor);
		}
	}
}

void UInteractableComponent::CheckForInteractionWithAssociate(AActor* Interactor)
{
	if (!Interactor)
	{
		return;
	}

	CurrentInteractor = Interactor;
	
	// 연결된 액터 체크가 활성화되고 목표 값이 맞는 경우
	if (CheckForAssociatedActors && IsTargetInteractableValue())
	{
		OnInteraction(CurrentInteractor);
		
		TArray<AActor*> Actors;
		AssociatedInteractableActors.GetKeys(Actors);
		
		// OnlyOnce 또는 완료 시 제거 옵션이 활성화된 경우
		if (RemoveAssociatedInteractableOnComplete || InteractionResponse == EInteractionResponse::OnlyOnce)
		{
			for (const auto Actor : Actors)
			{
				if (UInteractableComponent* Comp = Actor->FindComponentByClass<UInteractableComponent>())
				{
					OnRemoveInteraction();
					Comp->InteractionResponse = EInteractionResponse::OnlyOnce;
				}
			}
		}
		// Temporary 응답인 경우
		else if (InteractionResponse == EInteractionResponse::Temporary)
		{
			for (const auto Actor : Actors)
			{
				if (UInteractableComponent* Comp = Actor->FindComponentByClass<UInteractableComponent>())
				{
					OnRemoveInteraction();
					Comp->ToggleCanBeReInitialized(false);
				}
			}
		}
	}
}

void UInteractableComponent::OnInteraction(AActor* Interactor)
{
	CurrentInteractor = Interactor;
	AlreadyInteracted = true;

	AActor* OwnerActor = GetOwner();
	if(!OwnerActor) return;

	IInteractableInterface* InteractableOwner = Cast<IInteractableInterface>(OwnerActor);

	if (InteractableOwner)
	{
		InteractableOwner->Interaction(CurrentInteractor);
	}

	const FGameplayTag InteractionSuccessTag = RPGGameplayTags::Event_Interaction_Success.GetTag();
	if (InteractionSuccessTag.IsValid() && ResolveQuestManagerFromInteractor(CurrentInteractor))
	{
		if (UGlobalEventHandler* EventHandler = UGlobalEventHandler::Get(this))
		{
			TArray<FString> Metadata;
			Metadata.Add(FString::Printf(TEXT("EventTag=%s"), *InteractionSuccessTag.ToString()));

			const FGameplayTag InteractTag = ResolveInteractionTag(OwnerActor, QuestInteractionTag);
			if (InteractTag.IsValid())
			{
				Metadata.Add(FString::Printf(TEXT("InteractTag=%s"), *InteractTag.ToString()));
			}

			EventHandler->CallGlobalEventByGameplayTag(this, InteractionSuccessTag, OwnerActor, Metadata);
		}
	}
	
	RemoveInteractionByResponse();
}

void UInteractableComponent::ClientInteraction(AActor* Interactor)
{
	AActor* OwnerActor = GetOwner();
	if(!OwnerActor) return;
	IInteractableInterface* InteractableOwner = Cast<IInteractableInterface>(OwnerActor);

	if (InteractableOwner)
	{
		InteractableOwner->ClientBeginInteraction(Interactor);
	}
}

void UInteractableComponent::ClientRemoveInteraction()
{
	AActor* OwnerActor = GetOwner();
	if(!OwnerActor) return;
	
	IInteractableInterface* InteractableOwner = Cast<IInteractableInterface>(OwnerActor);
	if (InteractableOwner)
	{
		InteractableOwner->RemoveInteraction();
	}
	// 상호작용 불가 상태로 변경
	bIsInteractable = false;
	InteractableArea = nullptr;
	
	// 픽업 후 파괴 옵션이 활성화된 경우
	if(DestroyAfterPickup)
	{
		GetOwner()->Destroy();
	}
}

void UInteractableComponent::AssociatedActorEndInteraction()
{
	TArray<AActor*> Keys;
	AssociatedInteractableActors.GetKeys(Keys);

	for (auto* Key : Keys)
	{
		if (!Key) continue;

		if (UInteractableComponent* InteractableComponent = Key->FindComponentByClass<UInteractableComponent>())
		{
			InteractableComponent->OnEndInteraction(CurrentInteractor);
		}
	}
}

void UInteractableComponent::ToggleHighlight(bool Highlight, AActor* NewInteractor)
{
	if (InteractionWidget)
	{
		InteractionWidget->SetVisibility(Highlight, false);
	}
	
	for (UPrimitiveComponent* Component : ObjectsToHighlight)
	{
		if (Component)
		{
			Component->SetRenderCustomDepth(Highlight);

			if (Highlight)
			{
				Component->SetCustomDepthStencilValue(CustomDepthStencilValue);
			}
		}
	}
	
	if (APlayerController* OwningPC = Cast<APlayerController>(NewInteractor))
	{
		Server_SetWidgetLocalOwner(OwningPC);
	}
}

void UInteractableComponent::SetupInteractableReferences(UShapeComponent* Area, UWidgetComponent* Widget,
	TSet<UPrimitiveComponent*> HighlightableObjects)
{
	if (Area) 
	{
		InteractableArea = Area;
	}
	
	if (Widget)
	{
		InteractionWidget = Widget;
	}
	
	// 하이라이트 대상 오브젝트 배열 변환 및 설정
	ObjectsToHighlight.Reserve(HighlightableObjects.Num());
	for (const auto& Object : HighlightableObjects)
	{
		if (Object)
		{
			ObjectsToHighlight.Add(Object);
			Object->SetCollisionResponseToChannel(ECO_Interactable, ECR_Block);
		}
	}
}

void UInteractableComponent::DurationPress()
{
	GetWorld()->GetTimerManager().SetTimer(
		KeyDownTimer,
		this,
		&UInteractableComponent::IsKeyDown,
		DurationInputDelay,
		true);

	FKey InteractionKey{};
	if(GetPressedKeyByAction(InteractionAction,InteractionKey))
	{
		PressedInteractionKey = InteractionKey;
	}
	else
	{
		PressedInteractionKey = EKeys::E;
	}
}

void UInteractableComponent::IsKeyDown()
{
	IInteractorInterface* InteractorInterface = Cast<IInteractorInterface>(CurrentInteractor);
	if (!InteractorInterface)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Interactable] IsKeyDown Failed: Interactor Interface is Invalid"));
		return;
	}
	
	AActor* CurrentInteraction = InteractorInterface->GetCurrentInteractableObject();
    
	// 현재 상호작용 중인 오브젝트가 이 컴포넌트의 소유자인 경우
	if (CurrentInteraction && CurrentInteraction == GetOwner())
	{
		const EOperationStatus InputState = HoldingInput();
        
		if (InputState != EOperationStatus::None)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Interactable] IsKeyDown Status: %d (1:Complete, 2:Failed, 3:Reset)"), (int32)InputState);
		}
		
		if (InputState == EOperationStatus::Complete)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Interactable] Hold Complete! Starting Interaction."));
			GetWorld()->GetTimerManager().ClearTimer(KeyDownTimer);
			InteractorInterface->StartInteractionWithObject(this);
		}
		else if (InputState == EOperationStatus::Reset)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Interactable] Key Released. Resetting Hold Progress."));
			GetWorld()->GetTimerManager().ClearTimer(KeyDownTimer);
			OnUpdateHoldingValue.Broadcast(0.05f);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Interactable] IsKeyDown Aborted: Target Changed or Invalid. Current Target: %s"), *GetNameSafe(CurrentInteraction));
		GetWorld()->GetTimerManager().ClearTimer(KeyDownTimer);
	}
}

void UInteractableComponent::MultiplePress()
{
	EOperationStatus MashHoldState = MashingInput(10);
	
	if (EOperationStatus::None == MashHoldState)
	{
		// [Debug] 진행 상황 로그 (값이 변할 때마다 출력되므로 Verbose 권장, 여기선 Log로 표시)
		UE_LOG(LogTemp, Log, TEXT("[Interactable] MultiplePress Progress: %f / 1.0"), CurrentMashingValue);
        
		OnUpdateMashingValue.Broadcast(CurrentMashingValue);    
	}
	else if (EOperationStatus::Complete == MashHoldState)
	{
		UE_LOG(LogTemp, Log, TEXT("[Interactable] MultiplePress Complete! Starting Interaction..."));
		OnUpdateMashingValue.Broadcast(CurrentMashingValue);
        
		FTimerHandle UnusedHandle;
		GetWorld()->GetTimerManager().SetTimer(UnusedHandle, [this]()
		{
			IInteractorInterface* InteractorInterface = Cast<IInteractorInterface>(CurrentInteractor);
			if (InteractorInterface)
			{
				UE_LOG(LogTemp, Log, TEXT("[Interactable] Executing StartInteractionWithObject via MultiplePress"));
				InteractorInterface->StartInteractionWithObject(this);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[Interactable] MultiplePress Finished but CurrentInteractor does not implement IInteractorInterface."));
			}
		}, 0.2f, false);
	}
	else if (EOperationStatus::Failed == MashHoldState || EOperationStatus::Reset == MashHoldState)
	{
		// [Debug] 실패/리셋 로그 (필요시)
		UE_LOG(LogTemp, Log, TEXT("[Interactable] MultiplePress Reset or Failed."));
	}
}

void UInteractableComponent::OnPreInteraction(AActor* NewInteractor)
{
	if (!NewInteractor)
	{
		UE_LOG(LogTemp, Error, TEXT("[Interactable] OnPreInteraction Failed: NewInteractor is null"));
		return;
	}

	CurrentInteractor = NewInteractor;

	IInteractableInterface* InteractableOwner = Cast<IInteractableInterface>(CurrentInteractor);
	if (InteractableOwner)
	{
		InteractableOwner->ClientPrepareInteraction();
	}
    
	switch (InputType)
	{
	case EInteractionInputType::Single:
		{
			UE_LOG(LogTemp, Log, TEXT("[Interactable] InputType is Single. Executing Immediate Interaction."));
			IInteractorInterface* InteractorInterface = Cast<IInteractorInterface>(CurrentInteractor);
			if (InteractorInterface)
			{
				InteractorInterface->StartInteractionWithObject(this);
			}
			break;
		}
       
	case EInteractionInputType::Holding:
		{
			UE_LOG(LogTemp, Log, TEXT("[Interactable] InputType is Holding. Starting Duration Press Check."));
			DurationPress();
			break;
		}
       
	case EInteractionInputType::MultipleAndMashing:
		{
			UE_LOG(LogTemp, Log, TEXT("[Interactable] InputType is Mashing. Starting Multiple Press Check."));
			MultiplePress();
			break;
		}
	}
}

void UInteractableComponent::OnEndInteraction(AActor* NewInteractor)
{
	AActor* OwnerActor = GetOwner();
	if(!OwnerActor) return;
	
	IInteractableInterface* InteractableOwner = Cast<IInteractableInterface>(OwnerActor);
	if (InteractableOwner)
	{
		InteractableOwner->EndInteraction(NewInteractor);
	}
}

void UInteractableComponent::Reinitialize()
{
	if (InteractionResponse == EInteractionResponse::Temporary)
	{
		if (CanBeReInitialized)
		{
			IInteractableInterface* InteractableOwner = Cast<IInteractableInterface>(GetOwner());
			if (InteractableOwner)
			{
				InteractableOwner->Initialize();
			}
		}
		ToggleIsInteractable(true);
		// if (InteractionWidgetRef)
		// {
		// 	InteractionWidgetRef->SetVisibility(ESlateVisibility::Visible);
		// }
	}
}

void UInteractableComponent::ReInitializeAssociatedActors()
{
	TArray<AActor*> Keys;
	AssociatedInteractableActors.GetKeys(Keys);

	for (const auto* Key : Keys)
	{
		if (!Key) continue;

		if (UInteractableComponent* InteractableComponent = Key->FindComponentByClass<UInteractableComponent>())
		{
			InteractableComponent->ToggleCanBeReInitialized(true);
			Reinitialize();
		}
	}
}

void UInteractableComponent::OnClientEndInteraction(AActor* Interactor)
{
	IInteractableInterface* InteractableOwner = Cast<IInteractableInterface>(GetOwner());

	if (InteractableOwner)
	{
		InteractableOwner->ClientEndInteraction(Interactor);
	}
}

void UInteractableComponent::ToggleIsInteractable(bool Condition)
{
	if(bIsInteractable != Condition)
	{
		bIsInteractable = Condition;
	}
}

void UInteractableComponent::SetInteractionWidgetVisible(bool Condition)
{
	// if(bIsInteractable && InteractionWidgetRef)
	// {
	// 	InteractionWidgetRef->SetVisibility(
	// 		Condition ? ESlateVisibility::Hidden : ESlateVisibility::SelfHitTestInvisible);
	// }
}

void UInteractableComponent::RemoveInteractionByResponse()
{
	// OnlyOnce 또는 Temporary 응답인 경우 상호작용 제거
	if (InteractionResponse == EInteractionResponse::OnlyOnce || 
		InteractionResponse == EInteractionResponse::Temporary)
	{
		OnRemoveInteraction();
	}
}

void UInteractableComponent::ChangeInteractableValue(bool Increment)
{
	if(Increment)
	{
		if(++InteractableValue > InteractableLimitValue)
		{
			InteractableValue = 0;
		}
	}
	else
	{
		if(--InteractableValue < 0)
		{
			InteractableValue = InteractableLimitValue;
		}
	}
}

void UInteractableComponent::ToggleCanBeReInitialized(bool Condition)
{
	if(CanBeReInitialized != Condition)
	{
		CanBeReInitialized = Condition;
	}
}

void UInteractableComponent::SetWidgetLocalOwner(APlayerController* OwningPlayer)
{
	if(nullptr == OwningPlayer) return;

	// if (InteractionWidgetClass)
	// {
	// 	// 위젯 생성 및 초기화
	// 	InteractionWidgetRef = CreateWidget<UInteractionPromptWidget>(OwningPlayer, InteractionWidgetClass);
	// 	InteractionWidgetRef->InputType = InputType;
	// 	InteractionWidgetRef->InteractableComponent = this;
	// 	
	// 	// 위젯 컴포넌트 설정
	// 	InteractionWidget->SetWidget(InteractionWidgetRef);
	// 	InteractionWidget->SetOwnerPlayer(InteractionWidgetRef->GetOwningLocalPlayer());
	// 	InteractionWidget->SetWidgetSpace(EWidgetSpace::Screen);
	// 	InteractionWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// 	InteractionWidget->SetDrawAtDesiredSize(true);
	// }
}

EOperationStatus UInteractableComponent::HoldingInput()
{
	APlayerController* Controller = nullptr;

	if (APawn* InteractorPawn = Cast<APawn>(CurrentInteractor))
	{
		Controller = Cast<APlayerController>(InteractorPawn->GetController());
	}
	else 
	{
		Controller = Cast<APlayerController>(CurrentInteractor);
	}

	if (!Controller)
	{
		return EOperationStatus::None;
	}

	float KeyDownTime = Controller->GetInputKeyTimeDown(PressedInteractionKey);

	// 키를 뗀 경우
	if (KeyDownTime <= 0.0f)
	{
		return EOperationStatus::Reset;
	}

	OnUpdateHoldingValue.Broadcast(KeyDownTime);
	
	// 홀딩 완료
	if (KeyDownTime > MaxKeyTimeDown)
	{
		return EOperationStatus::Complete;
	}

	return EOperationStatus::None;
}

EOperationStatus UInteractableComponent::MashingInput(int32 MashingCount)
{
	CurrentMashingValue += (1.f / MashingCount);
	
	// 매싱 완료
	if (CurrentMashingValue >= 1.f)
	{
		CurrentMashingValue = 0.f;
		GetWorld()->GetTimerManager().ClearTimer(MashingTimer);
		OnUpdateHoldingValue.Broadcast(0.05f);
		return EOperationStatus::Complete;
	}
	
	// 일정 시간 내에 재입력하지 않으면 리셋
	GetWorld()->GetTimerManager().SetTimer(
		MashingTimer,
		this,
		&UInteractableComponent::ResetMashingProgress,
		MashingKeyRetriggerableTime,
		false
	);
	
	return EOperationStatus::None;
}

void UInteractableComponent::ResetMashingProgress()
{
	CurrentMashingValue = 0.f;
	OnUpdateHoldingValue.Broadcast(0.f);
}

bool UInteractableComponent::IsTargetInteractableValue()
{
	TArray<AActor*> Actors;
	AssociatedInteractableActors.GetKeys(Actors);

	// 연결된 액터들 중 목표 값과 일치하는 것이 있는지 체크
	for (const auto* Actor : Actors)
	{
		int32 Value = *AssociatedInteractableActors.Find(Actor);
		UInteractableComponent* Comp = Actor->FindComponentByClass<UInteractableComponent>();
		
		if (Actor && Comp && Comp->InteractableValue == Value) 
		{
			return true;
		}
	}
	return false;
}

bool UInteractableComponent::GetPressedKeyByAction(UInputAction* Action, FKey& OutKey)
{
	APlayerController* PC = Cast<APlayerController>(CurrentInteractor);
	if (!PC)
	{
		return false;
	}

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return false;
	}

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!InputSubsystem)
	{
		return false;
	}

	// 액션에 매핑된 모든 키 가져오기
	TArray<FKey> Keys = InputSubsystem->QueryKeysMappedToAction(Action);
	
	// 방금 눌린 키 찾기
	for (int i = 0; i < Keys.Num(); ++i)
	{
		FKey CurrentKey = Keys[i];
		if (PC->WasInputKeyJustPressed(CurrentKey))
		{
			OutKey = Keys[i];
			return true;
		}
	}

	return false;
}

void UInteractableComponent::OnRemoveInteraction_Implementation()
{
	ClientRemoveInteraction();
}

void UInteractableComponent::Server_SetWidgetLocalOwner_Implementation(APlayerController* OwningPlayer)
{
	SetWidgetLocalOwner(OwningPlayer);
}




