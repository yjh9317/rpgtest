// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "ActionComponent.generated.h"

class UDataAsset_ActionConfig;
class UBaseAction;
class ABaseCharacter;
enum class EActionEndReason : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionExecuted, const FGameplayTag&, ActionTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionCompleted, const FGameplayTag&, ActionTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionInterrupted, const FGameplayTag&, ActionTag);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RPGSYSTEM_API UActionComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UActionComponent();
    
protected:
    // === 액션 데이터 ===
    UPROPERTY(EditDefaultsOnly, Category = "Action")
    TObjectPtr<UDataAsset_ActionConfig> DefaultActionSet;

    UPROPERTY(BlueprintReadOnly, Category = "Action State")
    TMap<FGameplayTag, UBaseAction*> ActionInstances;

    UPROPERTY(BlueprintReadOnly, Category = "Action State")
    TArray<UBaseAction*> ActiveActions;
    
    UPROPERTY(Transient)
    TArray<UBaseAction*> TickingActions;
    
    // === 태그 시스템 ===
    UPROPERTY(BlueprintReadOnly, Category = "Tags")
    FGameplayTagContainer ActiveTags;
public:
    UPROPERTY(BlueprintAssignable, Category = "Action|Events")
    FOnActionExecuted OnActionExecutedEvent;

    UPROPERTY(BlueprintAssignable, Category = "Action|Events")
    FOnActionCompleted OnActionCompletedEvent;

    UPROPERTY(BlueprintAssignable, Category = "Action|Events")
    FOnActionInterrupted OnActionInterruptedEvent;

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === Action 관리 ===
    void AddAction(AActor* Instigator, TSubclassOf<UBaseAction> ActionClass, UObject* SourceObject);
    void RemoveActionsBySource(UObject* SourceObject);
    
    void RegisterAction(const FGameplayTag& ActionTag, TSubclassOf<UBaseAction> ActionClass);
    void UnregisterAction(const FGameplayTag& ActionTag);
    UBaseAction* GetAction(const FGameplayTag& ActionTag) const;

    // === Action 실행 ===
    bool ExecuteAction(const FGameplayTag& ActionTag);
    bool ExecuteActionInstance(UBaseAction* Action);
    void InterruptAction(const FGameplayTag& ActionTag);
    void InterruptAllActions();
    
    // === 태그 관리 ===
    void AddTags(const FGameplayTagContainer& Tags) { ActiveTags.AppendTags(Tags); }
    void RemoveTags(const FGameplayTagContainer& Tags) { ActiveTags.RemoveTags(Tags); }
    FGameplayTagContainer GetActiveTags() const { return ActiveTags; }
    bool HasTag(const FGameplayTag& Tag) const { return ActiveTags.HasTag(Tag); }
    bool HasAllTags(const FGameplayTagContainer& Tags) const { return ActiveTags.HasAll(Tags); }
    bool HasAnyTags(const FGameplayTagContainer& Tags) const { return ActiveTags.HasAny(Tags); }
    bool CheckTagRequirements(const UBaseAction* Action) const;
    void InterruptActionsWithTags(const FGameplayTagContainer& Tags);

    // === 쿼리 ===
    bool IsActionActive(const FGameplayTag& ActionTag) const;
    TArray<UBaseAction*> GetActiveActions() const { return ActiveActions; }
    TArray<FGameplayTag> GetRegisteredActionTags() const;
    TArray<UBaseAction*> GetActionsByCategory(const FGameplayTag& CategoryTag) const;
    int32 GetActiveActionCount() const { return ActiveActions.Num(); }
    bool HasAnyActiveActions() const { return ActiveActions.Num() > 0; }
    void GetActiveActionTags(TArray<FGameplayTag>& OutTags) const;

    // === 디버깅 ===
    void DebugLogActiveActions() const;
    void DebugLogAllActions() const;
    FString GetDebugString() const;

protected:
    void CreateActionInstances();
    void UpdateActiveActions(float DeltaTime);
    void OnActionCompleted(UBaseAction* Action, EActionEndReason EndReason);

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
};


