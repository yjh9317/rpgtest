// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/Action/BaseAction.h"
#include "AIBaseAction.generated.h"


class AEnemyController;
/**
 * AI 액션 실행 컨텍스트
 * BT에서 Action 실행 시 필요한 정보 전달
 */
USTRUCT(BlueprintType)
struct FAIActionContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AActor> Target;

	UPROPERTY(BlueprintReadWrite)
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	float DistanceToTarget = 0.f;

	UPROPERTY(BlueprintReadWrite)
	bool bHasLineOfSight = false;

	bool IsValid() const { return Target.IsValid(); }
};

/**
 * AI 액션 범위 조건
 */
USTRUCT(BlueprintType)
struct FAIActionRangeCondition
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MinRange = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxRange = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bRequiresLineOfSight = true;

	bool IsInRange(float Distance) const
	{
		return Distance >= MinRange && Distance <= MaxRange;
	}
};
/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UAIBaseAction : public UBaseAction
{
	GENERATED_BODY()
	
public:
	UAIBaseAction(const FObjectInitializer& ObjectInitializer);

protected:
	// === AI 전용 설정 ===
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Conditions")
	FAIActionRangeCondition RangeCondition;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Conditions")
	bool bRequiresTarget = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Conditions")
	float PreferredDistance = 100.f;  // AI가 이 거리에서 사용하려 함

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Behavior")
	float AIExecutionWeight = 1.f;  // AI 선택 가중치 (높을수록 우선)

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Behavior")
	bool bCanUseWhileMoving = false;

	// === 런타임 데이터 ===
	UPROPERTY(BlueprintReadOnly, Category = "AI|Runtime")
	FAIActionContext CurrentContext;

	UPROPERTY()
	TWeakObjectPtr<AEnemyController> CachedAIController;

	// UPROPERTY()
	// TWeakObjectPtr<UTargetingComponent> CachedTargetingComp;

public:
	// === 초기화 ===
	virtual void Initialize(AActor* NewActionOwner, UObject* NewSourceObject = nullptr) override;

	// === 조건 체크 ===
	virtual bool CanExecute() const override;

	UFUNCTION(BlueprintPure, Category = "AI|Action")
	virtual bool CanExecuteWithContext(const FAIActionContext& Context) const;

	UFUNCTION(BlueprintPure, Category = "AI|Action")
	bool IsTargetInRange() const;

	UFUNCTION(BlueprintPure, Category = "AI|Action")
	bool HasValidTarget() const;

	// === 컨텍스트 ===
	UFUNCTION(BlueprintCallable, Category = "AI|Action")
	void SetContext(const FAIActionContext& NewContext);

	UFUNCTION(BlueprintCallable, Category = "AI|Action")
	void UpdateContextFromTargeting();

	UFUNCTION(BlueprintPure, Category = "AI|Action")
	const FAIActionContext& GetContext() const { return CurrentContext; }

	// === 접근자 ===
	UFUNCTION(BlueprintPure, Category = "AI|Action")
	AEnemyController* GetAIController() const;

	// UFUNCTION(BlueprintPure, Category = "AI|Action")
	// UTargetingComponent* GetTargetingComponent() const;

	UFUNCTION(BlueprintPure, Category = "AI|Action")
	AActor* GetTarget() const { return CurrentContext.Target.Get(); }

	// === AI 평가용 ===
	UFUNCTION(BlueprintPure, Category = "AI|Evaluation")
	float GetAIWeight() const { return AIExecutionWeight; }

	UFUNCTION(BlueprintPure, Category = "AI|Evaluation")
	float GetPreferredDistance() const { return PreferredDistance; }

	UFUNCTION(BlueprintPure, Category = "AI|Evaluation")
	virtual float EvaluateUtility(const FAIActionContext& Context) const;

protected:
	virtual void OnExecute_Implementation() override;
	virtual void OnComplete_Implementation() override;

	UFUNCTION(BlueprintNativeEvent, Category = "AI|Action")
	void OnAIExecute(const FAIActionContext& Context);
	virtual void OnAIExecute_Implementation(const FAIActionContext& Context) {}

	UFUNCTION(BlueprintNativeEvent, Category = "AI|Action")
	void OnTargetChanged(AActor* NewTarget);
	virtual void OnTargetChanged_Implementation(AActor* NewTarget) {}

	float GetDistanceToTarget() const;
	bool HasLineOfSightToTarget() const;
};
