// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionTypes.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"
#include "UObject/NoExportTypes.h"
#include "BaseAction.generated.h"

class UBaseAction;
class UActionComponent;

UENUM(BlueprintType)
enum class EActionEndReason : uint8
{
	Completed,
	Interrupted
};

UENUM(BlueprintType)
enum class EActionInputPhase : uint8
{
	Pressed,
	Released,
	Held
};

DECLARE_DELEGATE_TwoParams(FOnActionEnded, UBaseAction*, EActionEndReason);

UCLASS(Blueprintable,BlueprintType)
class RPGSYSTEM_API UBaseAction : public UObject
{
	GENERATED_BODY()
	
public:
	UBaseAction(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action")
	FGameplayTag ActionTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action")
	FText ActionName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action")
	UTexture2D* ActionIcon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tags")
	FGameplayTagContainer RequiredTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tags")
	FGameplayTagContainer BlockedTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tags")
	FGameplayTagContainer GrantedTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tags")
	FGameplayTagContainer RemoveTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cost")
	TArray<FActionCost> ResourceCosts;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cost")
	float CooldownDuration = 0.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Conditions")
	bool bCanExecuteWhileMoving = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Conditions")
	bool bCanExecuteInAir = false;
	
	UPROPERTY(EditDefaultsOnly, Category = "Action")
	bool bWantsTick = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Conditions")
	bool bInterruptible = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Gesture")
	bool bEnableGestureDetection = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Gesture")
	bool bAutoDisableGestureForNonPlayerControlled = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Gesture", meta = (ClampMin = "0.01"))
	float ShortPressThreshold = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Gesture", meta = (ClampMin = "0.01"))
	float LongPressThreshold = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Gesture")
	bool bEnableDoubleClick = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Gesture", meta = (ClampMin = "0.01"))
	float DoubleClickThreshold = 0.3f;
		
	FOnActionEnded OnActionEnded;

protected:
	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;

	UPROPERTY()
	TObjectPtr<UActionComponent> CachedActionComponent;
	
	UPROPERTY()
	TObjectPtr<UObject> SourceObject;

	bool bIsActive = false;
	float ActivationTime = 0.0f;
	float LastExecutionTime = -999.0f;
	bool bInputPressed = false;
	bool bLongPressTriggered = false;
	bool bPendingSingleClick = false;
	float InputPressedTime = -1.0f;
	FTimerHandle SingleClickTimerHandle;

public:
	virtual void Initialize(AActor* NewActionOwner, UObject* NewSourceObject = nullptr);
	virtual bool ProcessInput() { return false; }
	virtual bool HandleInput(EActionInputPhase InputPhase, float InputValue = 1.0f);
	void Execute();
    void Tick(float DeltaTime);
	UFUNCTION(BlueprintCallable, Category = "Action")
    void Interrupt();
	UFUNCTION(BlueprintCallable, Category = "Action")
	void Complete();

    UFUNCTION(BlueprintPure, Category = "Action")
    virtual bool CanExecute() const;

    UFUNCTION(BlueprintPure, Category = "Action")
    virtual bool HasRequiredResources() const;

    UFUNCTION(BlueprintPure, Category = "Action")
    bool IsActive() const { return bIsActive; }

	UFUNCTION(BlueprintPure, Category = "Action")
	float GetTimeActive() const
    {
    	if (!bIsActive) return 0.0f;
    	return GetWorld()->GetTimeSeconds() - ActivationTime;
    }

	UFUNCTION(BlueprintPure, Category = "Action|Input")
	bool IsInputPressed() const { return bInputPressed; }

	UFUNCTION(BlueprintPure, Category = "Action|Input")
	float GetCurrentInputHoldTime() const;

    UFUNCTION(BlueprintPure, Category = "Action")
    float GetCooldownRemaining() const;
	
    UFUNCTION(BlueprintPure, Category = "Action")
    FGameplayTag GetActionTag() const { return ActionTag; }

    UFUNCTION(BlueprintPure, Category = "Action")
    ACharacter* GetOwnerCharacter() const { return OwnerCharacter; }

    UFUNCTION(BlueprintPure, Category = "Action")
    APlayerController* GetOwnerController() const;

	// UFUNCTION()
	// virtual void OnGameEvent(FGameplayTag EventTag, const FRPGEventData& EventData);
	// virtual void HandleGameEvent(FGameplayTag EventTag, const FRPGEventData& EventData) {}
protected:
    virtual void ConsumeResources();
    virtual void RefundResources(float Percentage = 1.0f);
    virtual void ApplyGrantedTags();
    virtual void RemoveGrantedTags();


#pragma region InternalFunction
	UFUNCTION(BlueprintNativeEvent, Category = "Action|Event")
	void OnInitialized();
	virtual void OnInitialized_Implementation() {} 

	UFUNCTION(BlueprintNativeEvent, Category = "Action|Event")
	void OnExecute();
	virtual void OnExecute_Implementation() {} 

	UFUNCTION(BlueprintNativeEvent, Category = "Action|Event")
	void OnTick(float DeltaTime);
	virtual void OnTick_Implementation(float DeltaTime) {}

	UFUNCTION(BlueprintNativeEvent, Category = "Action|Event")
	void OnInterrupt();
	virtual void OnInterrupt_Implementation() {}

	UFUNCTION(BlueprintNativeEvent, Category = "Action|Event")
	void OnComplete();
	virtual void OnComplete_Implementation() {}

	UFUNCTION(BlueprintNativeEvent, Category = "Action|Input")
	void OnInputPressedEvent(float InputValue);
	virtual void OnInputPressedEvent_Implementation(float InputValue) {}

	UFUNCTION(BlueprintNativeEvent, Category = "Action|Input")
	void OnInputHeldEvent(float HoldDuration, float InputValue);
	virtual void OnInputHeldEvent_Implementation(float HoldDuration, float InputValue) {}

	UFUNCTION(BlueprintNativeEvent, Category = "Action|Input")
	void OnInputReleasedEvent(float HoldDuration, float InputValue);
	virtual void OnInputReleasedEvent_Implementation(float HoldDuration, float InputValue) {}

	UFUNCTION(BlueprintNativeEvent, Category = "Action|Input")
	void OnShortPressEvent(float HoldDuration);
	virtual void OnShortPressEvent_Implementation(float HoldDuration) {}

	UFUNCTION(BlueprintNativeEvent, Category = "Action|Input")
	void OnLongPressEvent(float HoldDuration);
	virtual void OnLongPressEvent_Implementation(float HoldDuration) {}

	UFUNCTION(BlueprintNativeEvent, Category = "Action|Input")
	void OnSingleClickEvent();
	virtual void OnSingleClickEvent_Implementation() {}

	UFUNCTION(BlueprintNativeEvent, Category = "Action|Input")
	void OnDoubleClickEvent();
	virtual void OnDoubleClickEvent_Implementation() {}
#pragma endregion
	
	virtual void StartCooldown();
    virtual bool OnHasRequiredResources() const { return true; }
	void EndAction(EActionEndReason EndReason);
	void TrackInputGesture(EActionInputPhase InputPhase, float InputValue);
	void HandleSingleClickTimeout();
	void ClearPendingSingleClick();
	float GetWorldTimeSafe() const;
public:
	UObject* GetSourceObject() const { return SourceObject; }
};
