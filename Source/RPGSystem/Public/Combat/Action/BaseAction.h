// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionTypes.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"
#include "BaseAction.generated.h"

class UBaseAction;
class UActionComponent;


DECLARE_DELEGATE_OneParam(FOnActionEnded, UBaseAction*);

UCLASS()
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
	float TimeActive = 0.0f;
	float LastExecutionTime = -999.0f;

public:
	virtual void Initialize(AActor* NewActionOwner, UObject* NewSourceObject = nullptr);
	virtual bool ProcessInput() { return false; }
	void Execute();
    void Tick(float DeltaTime);
    void Interrupt();
	void Complete();

    UFUNCTION(BlueprintPure, Category = "Action")
    virtual bool CanExecute() const;

    UFUNCTION(BlueprintPure, Category = "Action")
    virtual bool HasRequiredResources() const;

    UFUNCTION(BlueprintPure, Category = "Action")
    bool IsActive() const { return bIsActive; }

    UFUNCTION(BlueprintPure, Category = "Action")
    float GetTimeActive() const { return TimeActive; }

    UFUNCTION(BlueprintPure, Category = "Action")
    float GetCooldownRemaining() const;
	
    UFUNCTION(BlueprintPure, Category = "Action")
    FGameplayTag GetActionTag() const { return ActionTag; }

    UFUNCTION(BlueprintPure, Category = "Action")
    ACharacter* GetOwnerCharacter() const { return OwnerCharacter; }

    UFUNCTION(BlueprintPure, Category = "Action")
    APlayerController* GetOwnerController() const;

protected:
    virtual void ConsumeResources();
    virtual void RefundResources(float Percentage = 1.0f);
    virtual void ApplyGrantedTags();
    virtual void RemoveGrantedTags();


#pragma region InternalFunction
    virtual void OnInitialized(){};
    virtual void OnExecute(){};
    virtual void OnTick(float DeltaTime){};
    virtual void OnInterrupt(){};
    virtual void OnComplete() {};
#pragma endregion
	
	virtual void StartCooldown();
    virtual bool OnHasRequiredResources() const { return true; }
public:
	UObject* GetSourceObject() const { return SourceObject; }
};
