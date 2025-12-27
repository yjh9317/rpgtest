// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/Action/ActionOwner.h"
#include "Components/ActorComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "InteractionComponent.generated.h"

class UInteractableComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnNewInteractableAssigned, UInteractableComponent* Comp);
DECLARE_MULTICAST_DELEGATE(FOnRemovedCurrentInteractable);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RPGSYSTEM_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()


public:
	UInteractionComponent(const FObjectInitializer& ObjectInitializer);
	void TryInteract();

	UFUNCTION(Client, Reliable)
	void InitializeInteraction(APlayerController* PlayerController);
	
	FOnNewInteractableAssigned OnNewInteractableAssigned;
	FOnRemovedCurrentInteractable OnRemovedCurrentInteractable;
protected:
	// ====================
	// 초기화 및 업데이트
	// ====================
	
	/** 상호작용 컴포넌트 초기화 */


	/** 클라이언트 측 상호작용 업데이트 */
	UFUNCTION()
	void ClientUpdateInteraction();

	/** 상호작용 대상 갱신 로직 */
	void UpdateInteraction();
	
	/** 현재 상호작용 대상의 유효성 체크 */
	void UpdateCurrentInteractable();
	
	// ====================
	// 상호작용 관리
	// ====================
	
	/** 현재 상호작용 제거 */
	void RemoveInteractionFromCurrent();
	
	/** 새로운 상호작용 대상 할당 */
	void AssignInteractionToLocal(UInteractableComponent* InteractableComponent);
	
	// ====================
	// 트레이스 설정
	// ====================
	UPROPERTY(EditDefaultsOnly, Category="Interaction|Trace")
	float CameraTraceLength = 500.f;

	UPROPERTY(EditDefaultsOnly, Category="Interaction|Trace")
	float OwnerTraceLength = 250.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Interaction|Trace")
	float InteractionDistance = 110.f;

	// ====================
	// 디버그 설정
	// ====================
	UPROPERTY(EditDefaultsOnly, Category="Interaction|Debug")
	TEnumAsByte<EDrawDebugTrace::Type> DebugTrace = EDrawDebugTrace::ForOneFrame;

	UPROPERTY(EditDefaultsOnly, Category="Interaction|Debug", meta=(EditCondition="DebugTrace != EDrawDebugTrace::None"))
	FLinearColor TraceColor = FLinearColor::Red;

	UPROPERTY(EditDefaultsOnly, Category="Interaction|Debug", meta=(EditCondition="DebugTrace != EDrawDebugTrace::None"))
	FLinearColor TraceHitColor = FLinearColor::Green;

	UPROPERTY(EditDefaultsOnly, Category="Interaction|Debug")
	float DebugDrawTime = 0.1f;
	
	// ====================
	// 타이밍 설정
	// ====================
	UPROPERTY(EditDefaultsOnly, Category="Interaction|Timing")
	float InteractionDelay = 0.1f;
	
	// ====================
	// UI 설정
	// ====================
	UPROPERTY(EditDefaultsOnly, Category="Interaction|UI")
	TSubclassOf<UUserWidget> InteractionWidgetClass;
	
	// ====================
	// 런타임 데이터 (GC 관리용)
	// ====================
	UPROPERTY()
	TObjectPtr<UInteractableComponent> CurrentInteractable;
	
	UPROPERTY()
	TObjectPtr<APawn> ControlledPawnRef;
	
	UPROPERTY()
	TObjectPtr<AController> ControllerRef;
	
	// ====================
	// 런타임 상태 (UPROPERTY 불필요)
	// ====================
	FTimerHandle InteractionTimer;
	
private:
	// ====================
	// 트레이스 헬퍼 함수
	// ====================
	
	/** 캡슐 트레이스 수행 */
	bool PerformInteractionTrace(
		TArray<FHitResult>& OutHits, 
		const FVector& StartLocation, 
		const FVector& EndLocation,
		const TArray<AActor*>& IgnoredActors,
		float CapsuleRadius = 50.0f, 
		float CapsuleHalfHeight = 100.0f
	);

	/** 전방 벡터 기반 트레이스로 상호작용 대상 찾기 */
	UInteractableComponent* FindInteractableViaForwardTrace();
	
	/** 카메라 시선 기반 트레이스로 상호작용 대상 찾기 */
	UInteractableComponent* FindInteractableViaTrace();
	
	/** 상호작용 가능한 컴포넌트인지 유효성 검사 */
	bool IsValidInteractable(UInteractableComponent* Component) const;

	// ====================
	// 디버그
	// ====================
#if WITH_EDITOR
	/** 디버그 정보 화면 출력 */
	void DebugDrawInteractionStatus(
		bool bBothConditionsMet, 
		UInteractableComponent* CameraTraceResult, 
		UInteractableComponent* ForwardTraceResult
	) const;
#endif
	
public:
	UInteractableComponent* GetCurrentInteractable() const { return CurrentInteractable; }
};

