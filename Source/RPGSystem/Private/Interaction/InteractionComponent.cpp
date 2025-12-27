// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/InteractionComponent.h"

#include "RPGSystemCollisionChannels.h"
#include "Camera/CameraComponent.h"
#include "Components/ShapeComponent.h"
#include "Interaction/InteractableComponent.h"
#include "Interaction/Interface/InteractableInterface.h"
#include "Interaction/Interface/InteractorInterface.h"

UInteractionComponent::UInteractionComponent(const FObjectInitializer& ObjectInitializer)
{
}

void UInteractionComponent::ClientUpdateInteraction()
{
	UpdateInteraction();
}

void UInteractionComponent::UpdateInteraction()
{
	// 현재 상호작용 대상 유효성 체크
	UpdateCurrentInteractable();
	
	// 컨트롤러의 폰 가져오기
	APawn* ControllerPawn = ControllerRef ? ControllerRef->GetPawn() : nullptr;
	if (!ControllerPawn)
	{
		return;
	}
	ControlledPawnRef = ControllerPawn;

	// 두 가지 방식으로 상호작용 대상 탐색
	UInteractableComponent* CameraTraceResult = FindInteractableViaTrace();
	UInteractableComponent* ForwardTraceResult = FindInteractableViaForwardTrace();

	// 현재 상호작용 중인 객체가 없는 경우
	if (!CurrentInteractable)
	{
		// 두 트레이스가 모두 같은 대상을 감지한 경우에만 상호작용 시작
		if (CameraTraceResult && ForwardTraceResult && 
			CameraTraceResult == ForwardTraceResult)
		{
			AssignInteractionToLocal(CameraTraceResult);
		}
	}
	// 현재 상호작용 중인 객체가 있는 경우
	else
	{
		// 상호작용 유지 조건 체크
		bool bCameraDetectsCurrent = (CameraTraceResult == CurrentInteractable);
		bool bForwardDetectsCurrent = (ForwardTraceResult == CurrentInteractable);
		
		// 둘 중 하나라도 현재 객체를 감지하면 유지
		if (bCameraDetectsCurrent || bForwardDetectsCurrent)
		{
			return;
		}
		
		// 둘 다 현재 객체를 감지하지 못하는 경우 상호작용 제거
		RemoveInteractionFromCurrent();
		
		// 새로운 동일한 객체를 감지했다면 즉시 새 상호작용 시작
		if (CameraTraceResult && ForwardTraceResult && 
			CameraTraceResult == ForwardTraceResult)
		{
			AssignInteractionToLocal(CameraTraceResult);
		}
	}
}

void UInteractionComponent::InitializeInteraction_Implementation(APlayerController* PlayerController)
{
	ControllerRef = PlayerController;
	
	if (UWorld* World = GetWorld())
	{
		// 기존 타이머 정리
		World->GetTimerManager().ClearTimer(InteractionTimer);
		
		// 상호작용 업데이트 타이머 시작
		World->GetTimerManager().SetTimer(
			InteractionTimer,
			this,
			&UInteractionComponent::ClientUpdateInteraction,
			InteractionDelay,
			true,
			1.f
		);
	}
}

void UInteractionComponent::RemoveInteractionFromCurrent()
{
	AActor* OwnerActor = GetOwner();
	if (!CurrentInteractable || !OwnerActor) return;

	// 하이라이트 제거
	CurrentInteractable->ToggleHighlight(false, ControllerRef);
	
	// ObjectInteraction 인터페이스를 통해 종료 처리
	if (IInteractorInterface* ObjectInteraction = Cast<IInteractorInterface>(OwnerActor))
	{
		ObjectInteraction->EndInteractionWithObject(CurrentInteractable);
		CurrentInteractable = nullptr;
		OnRemovedCurrentInteractable.Broadcast();
	}
}


void UInteractionComponent::UpdateCurrentInteractable()
{
	if (!CurrentInteractable)
	{
		return;
	}
	
	// 상호작용 인터페이스 유효성 체크
	if(IInteractableInterface* InteractableInterface = Cast<IInteractableInterface>(CurrentInteractable->GetOwner()))
	{
		// 더 이상 상호작용 불가능한 경우 제거
		if (InteractableInterface->CanBeInteractedWith())
		{
			RemoveInteractionFromCurrent();
		}
	}
}

void UInteractionComponent::AssignInteractionToLocal(UInteractableComponent* InteractableComponent)
{
	// 이미 현재 상호작용 중인 대상과 동일한 경우 무시
	if (InteractableComponent == CurrentInteractable)
	{
		return;		
	}
	
	// 기존 상호작용 제거
	RemoveInteractionFromCurrent();

	// 새로운 상호작용 대상 설정
	InteractableComponent->ToggleHighlight(true, ControllerRef);
	CurrentInteractable = InteractableComponent;
	OnNewInteractableAssigned.Broadcast(CurrentInteractable);
}

bool UInteractionComponent::PerformInteractionTrace(TArray<FHitResult>& OutHits, const FVector& StartLocation,
                                                    const FVector& EndLocation, const TArray<AActor*>& IgnoredActors, float CapsuleRadius, float CapsuleHalfHeight)
{
	// 상호작용 가능한 오브젝트 타입 설정
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesToQuery;
	ObjectTypesToQuery.Add(UEngineTypes::ConvertToObjectType(ECO_Interactable));
	
	bool bIsHit = UKismetSystemLibrary::CapsuleTraceMultiForObjects(
		GetWorld(),
		StartLocation,
		EndLocation,
		CapsuleRadius,
		CapsuleHalfHeight,
		ObjectTypesToQuery,
		false,
		IgnoredActors,
		DebugTrace,
		OutHits,
		true,
		TraceColor,
		TraceHitColor,
		DebugDrawTime
	);
    
	return bIsHit;
}

UInteractableComponent* UInteractionComponent::FindInteractableViaForwardTrace()
{
	if (!ControlledPawnRef || !GetWorld())
	{
		return nullptr;
	}

	// 폰의 전방 벡터 기반 트레이스
	FVector PawnLocation = ControlledPawnRef->GetActorLocation();
	FVector PawnForward = ControlledPawnRef->GetActorForwardVector();
	FVector StartLocation = PawnLocation;
	FVector EndLocation = PawnLocation + PawnForward * OwnerTraceLength;

	TArray<AActor*> IgnoredActors{ ControlledPawnRef };
	TArray<FHitResult> OutHits;
	
	if (!PerformInteractionTrace(OutHits, StartLocation, EndLocation, IgnoredActors))
	{
		return nullptr;
	}

	// 히트한 액터들 중 유효한 상호작용 컴포넌트 찾기
	for (const FHitResult& HitResult : OutHits)
	{
		AActor* HitActor = HitResult.GetActor();
		if (!HitActor)
		{
			continue;
		}

		UInteractableComponent* FoundInteractableComp = HitActor->FindComponentByClass<UInteractableComponent>();
		if (FoundInteractableComp && IsValidInteractable(FoundInteractableComp))
		{
			return FoundInteractableComp;
		}
	}

	return nullptr;
}

UInteractableComponent* UInteractionComponent::FindInteractableViaTrace()
{
	// 카메라 컴포넌트 가져오기
	UCameraComponent* PlayerCamera = ControlledPawnRef->FindComponentByClass<UCameraComponent>();
	if (!PlayerCamera)
	{
		return nullptr;
	}

	// 카메라 시선 방향 기반 트레이스
	const FVector PawnLocation = ControlledPawnRef->GetActorLocation();
	const FVector CameraForward = PlayerCamera->GetForwardVector();
	const FVector StartLocation = PawnLocation + FVector(0.f, 0.f, InteractionDistance);
	const FVector EndLocation = StartLocation + CameraForward * CameraTraceLength;

	TArray<FHitResult> OutHits;
	const TArray<AActor*> IgnoredActors{ ControlledPawnRef };
	
	// 카메라 트레이스는 좁은 캡슐 사용 (정확한 조준)
	if (!PerformInteractionTrace(OutHits, StartLocation, EndLocation, IgnoredActors, 35.f, 0.f))
	{
		return nullptr;
	}

	// 히트한 액터들 중 유효한 상호작용 컴포넌트 찾기
	for (const FHitResult& Hit : OutHits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!IsValid(HitActor))
		{
			continue;
		}

		UInteractableComponent* FoundComponent = HitActor->FindComponentByClass<UInteractableComponent>();
		if (FoundComponent && IsValidInteractable(FoundComponent))
		{
			return FoundComponent;
		}
	}

	return nullptr;
}

bool UInteractionComponent::IsValidInteractable(UInteractableComponent* Component) const
{
	if (!Component)
	{
		return false;
	}
	AActor* OwnerActor = Component->GetOwner();
	IInteractableInterface* InteractableOwner = Cast<IInteractableInterface>(OwnerActor);
	if (!OwnerActor || !ControlledPawnRef || !InteractableOwner)
	{
		return false;
	}

	bool bInterface = OwnerActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass());
	bool bCanInteraction = InteractableOwner->CanBeInteractedWith();
	if (!bInterface || !bCanInteraction)
	{
		return false;
	}

	UShapeComponent* Area = Component->InteractableArea;
	if (!IsValid(Area))
	{
		return false;
	}
	
	return true;
}

void UInteractionComponent::DebugDrawInteractionStatus(bool bBothConditionsMet,
	UInteractableComponent* CameraTraceResult, UInteractableComponent* ForwardTraceResult) const
{
	if (DebugTrace != EDrawDebugTrace::None && GEngine)
	{
		// 현재 상태 텍스트 생성
		FString CurrentStatus = CurrentInteractable ? 
			FString::Printf(TEXT("Current: %s"), *CurrentInteractable->GetOwner()->GetName()) : 
			TEXT("Current: None");
	
		FString CameraStatus = CameraTraceResult ? 
			FString::Printf(TEXT("Camera: %s"), *CameraTraceResult->GetOwner()->GetName()) : 
			TEXT("Camera: None");
	
		FString ForwardStatus = ForwardTraceResult ? 
			FString::Printf(TEXT("Forward: %s"), *ForwardTraceResult->GetOwner()->GetName()) : 
			TEXT("Forward: None");
	
		FString DebugMessage = FString::Printf(
			TEXT("%s | %s | %s | Both Met: %s"),
			*CurrentStatus,
			*CameraStatus,
			*ForwardStatus,
			bBothConditionsMet ? TEXT("YES") : TEXT("NO")
		);
	
		// 상태에 따라 색상 결정
		FColor MessageColor = FColor::Yellow;
		if (CurrentInteractable)
		{
			// 상호작용 유지 여부에 따른 색상
			bool bMaintained = (CameraTraceResult == CurrentInteractable) || 
							  (ForwardTraceResult == CurrentInteractable);
			MessageColor = bMaintained ? FColor::Green : FColor::Orange;
		}
		else if (bBothConditionsMet)
		{
			// 새로운 상호작용 가능
			MessageColor = FColor::Cyan;
		}
		else
		{
			// 상호작용 불가
			MessageColor = FColor::Red;
		}
	
		GEngine->AddOnScreenDebugMessage(-1, 0.1f, MessageColor, DebugMessage);
	}
}

void UInteractionComponent::TryInteract()
{
	if (CurrentInteractable)
	{
		CurrentInteractable->OnPreInteraction(GetOwner());
	}
}
