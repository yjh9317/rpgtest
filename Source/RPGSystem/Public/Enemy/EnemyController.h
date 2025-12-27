// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "EnemyController.generated.h"

UENUM(BlueprintType)
enum class EEnemyAIState : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Patrol		UMETA(DisplayName = "Patrol"),
	Chase		UMETA(DisplayName = "Chase"),
	Attack		UMETA(DisplayName = "Attack"),
	Stunned		UMETA(DisplayName = "Stunned"),
	Dead		UMETA(DisplayName = "Dead")
};

class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UAISenseConfig_Damage;
/**
 * 
 */
UCLASS()
class RPGSYSTEM_API AEnemyController : public AAIController
{
	GENERATED_BODY()
	
protected:
	AEnemyController();
	virtual void OnPossess(APawn* InPawn) override;
	virtual void BeginPlay() override;

	// --- AI Perception Components ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UAISenseConfig_Damage> DamageConfig;

	// --- AI State & Behavior ---
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	EEnemyAIState CurrentState;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	AActor* TargetActor;

	// 감각 업데이트 델리게이트 함수
	UFUNCTION()
	void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);

	// 상태 변경 함수
	void SetState(EEnemyAIState NewState);

	// 상태별 행동 처리 (Tick에서 호출하거나 타이머로 관리)
	void HandleIdle();
	void HandleChase();
	void HandleAttack();
	
public:
	UFUNCTION(BlueprintCallable, Category = "AI")
	EEnemyAIState GetCurrentState() const { return CurrentState; }
	
	UFUNCTION(BlueprintCallable, Category = "AI")
	AActor* GetTargetActor() const { return TargetActor; }
};