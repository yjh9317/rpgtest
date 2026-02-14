// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionTypes.h"
#include "GameplayTagContainer.h"
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

public:
	virtual void Initialize(AActor* NewActionOwner, UObject* NewSourceObject = nullptr);
	virtual bool ProcessInput() { return false; }
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
#pragma endregion
	
	virtual void StartCooldown();
    virtual bool OnHasRequiredResources() const { return true; }
	void EndAction(EActionEndReason EndReason);
public:
	UObject* GetSourceObject() const { return SourceObject; }
};
