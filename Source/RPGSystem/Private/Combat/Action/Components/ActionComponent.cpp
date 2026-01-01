// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Action/Components/ActionComponent.h"
#include "Combat/Action/BaseAction.h"
#include "Combat/Action/DataAsset_ActionConfig.h"

UActionComponent::UActionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UActionComponent::BeginPlay()
{
    Super::BeginPlay();
    CreateActionInstances();
}

void UActionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    for (int32 i = TickingActions.Num() - 1; i >= 0; --i)
    {
        UBaseAction* Action = TickingActions[i];
        if (Action && Action->IsActive())
        {
            Action->Tick(DeltaTime);
        }
        else
        {
            // 비활성화되었는데 리스트에 남아있다면 제거
            TickingActions.RemoveAt(i);
        }
    }
}

void UActionComponent::AddAction(AActor* Instigator, TSubclassOf<UBaseAction> ActionClass, UObject* SourceObject)
{
    if (!ActionClass) return;

    const UBaseAction* DefaultActionObj = ActionClass->GetDefaultObject<UBaseAction>();
    if (!DefaultActionObj || !DefaultActionObj->GetActionTag().IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("AddAction: Invalid Action Tag in %s"), *GetNameSafe(ActionClass));
        return;
    }

    FGameplayTag ActionTag = DefaultActionObj->GetActionTag();

    // 2. 이미 해당 태그의 액션이 있다면 제거 (교체)
    if (ActionInstances.Contains(ActionTag))
    {
        UBaseAction* OldAction = ActionInstances[ActionTag];
        if (OldAction)
        {
            // 실행 중이면 강제 중단
            if (OldAction->IsActive())
            {
                OldAction->Interrupt();
            }
            // 맵에서 제거 (GC가 나중에 메모리 해제)
            ActionInstances.Remove(ActionTag);
        }
    }

    UBaseAction* NewAction = NewObject<UBaseAction>(this, ActionClass);
    NewAction->Initialize(Instigator, SourceObject);
    ActionInstances.Add(ActionTag, NewAction);

    UE_LOG(LogTemp, Log, TEXT("Action Added: [%s] from Source: [%s]"),*ActionTag.ToString(), *GetNameSafe(SourceObject));
}

void UActionComponent::RemoveActionsBySource(UObject* SourceObject)
{
    if (!SourceObject) return;

    TArray<FGameplayTag> TagsToRemove;

    for (auto It = ActionInstances.CreateIterator(); It; ++It)
    {
        UBaseAction* Action = It.Value();
        if (Action && Action->GetSourceObject() == SourceObject)
        {
            TagsToRemove.Add(It.Key());
        }
    }

    for (const FGameplayTag& Tag : TagsToRemove)
    {
        UBaseAction* ActionToRemove = ActionInstances[Tag];
        if (ActionToRemove && ActionToRemove->IsActive())
        {
            ActionToRemove->Interrupt();
        }
        ActionInstances.Remove(Tag);

        if (DefaultActionSet)
        {
            for (const FActionDefinition& Def : DefaultActionSet->DefaultActions)
            {
                if (Def.ActionTag == Tag && Def.ActionClass)
                {
                    UBaseAction* NewAction = NewObject<UBaseAction>(this, Def.ActionClass);
                    NewAction->Initialize(GetOwner()); // SourceObject 없음 (기본으로 복귀)
                    NewAction->ActionTag = Def.ActionTag;
                    
                    ActionInstances.Add(Tag, NewAction);
                    
                    UE_LOG(LogTemp, Log, TEXT(" -> Default Action Restored: [%s]"), *Tag.ToString());
                    break;
                }
            }
        }
    }
}

void UActionComponent::RegisterAction(const FGameplayTag& ActionTag, TSubclassOf<UBaseAction> ActionClass)
{
    if (!ActionTag.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("ActionComponent::RegisterAction - Invalid ActionTag"));
        return;
    }

    if (!ActionClass)
    {
        UE_LOG(LogTemp, Error, TEXT("ActionComponent::RegisterAction - Invalid ActionClass for tag %s"), 
               *ActionTag.ToString());
        return;
    }

    if (ActionInstances.Contains(ActionTag))
    {
        UnregisterAction(ActionTag);
    }

    UBaseAction* NewAction = NewObject<UBaseAction>(this, ActionClass);
    NewAction->Initialize(GetOwner());
    NewAction->ActionTag = ActionTag;
    
    ActionInstances.Add(ActionTag, NewAction);
    
}

void UActionComponent::UnregisterAction(const FGameplayTag& ActionTag)
{
    if (!ActionTag.IsValid())
        return;
    
    InterruptAction(ActionTag);

    if (UBaseAction* Action = ActionInstances.FindRef(ActionTag))
    {
        ActiveActions.Remove(Action);
        ActionInstances.Remove(ActionTag);
        
        Action->ConditionalBeginDestroy();
    }

    UE_LOG(LogTemp, Log, TEXT("Unregistered action: %s"), *ActionTag.ToString());
}

UBaseAction* UActionComponent::GetAction(const FGameplayTag& ActionTag) const
{
    if (!ActionTag.IsValid())
        return nullptr;

    return ActionInstances.FindRef(ActionTag);
}

bool UActionComponent::ExecuteAction(const FGameplayTag& ActionTag)
{
    if (!ActionTag.IsValid()) return false;

    UBaseAction* Action = GetAction(ActionTag);
    if (!Action)
    {
        return false;
    }

    if (Action->IsActive())
    {
        // 활성 상태라면 콤보(입력) 처리를 시도한다.
        // ProcessInput이 true를 반환하면 콤보가 성공한 것
        return Action->ProcessInput(); 
    }

    // 활성 상태가 아니라면 새로 실행
    ActiveActions.AddUnique(Action);
    Action->Execute();

    return true;
}

bool UActionComponent::ExecuteActionInstance(UBaseAction* Action)
{
    if (ActiveActions.Contains(Action))
    {
        Action->Execute();
        return true;
    }
    
    // 새로운 액션
    if (!Action->CanExecute())
        return false;

    ActiveActions.AddUnique(Action);
    Action->Execute();
    
    if (Action->bWantsTick)
    {
        TickingActions.AddUnique(Action);
    }
       
    return true;
}

void UActionComponent::InterruptAction(const FGameplayTag& ActionTag)
{
    if (!ActionTag.IsValid())
        return;

    UBaseAction* Action = ActionInstances.FindRef(ActionTag);
    if (!Action || !Action->IsActive())
        return;

    Action->Interrupt();
    ActiveActions.Remove(Action);

    UE_LOG(LogTemp, Log, TEXT("Interrupted action: %s"), *ActionTag.ToString());
}

void UActionComponent::InterruptAllActions()
{
    // 역순으로 순회하면서 중단 (안전한 제거를 위해)
    for (int32 i = ActiveActions.Num() - 1; i >= 0; --i)
    {
        UBaseAction* Action = ActiveActions[i];
        if (Action && Action->IsActive())
        {
            Action->Interrupt();
        }
    }
    
    ActiveActions.Empty();
}

bool UActionComponent::CheckTagRequirements(const UBaseAction* Action) const
{
    // 필수 태그 체크
    if (Action->RequiredTags.Num() > 0 && !ActiveTags.HasAll(Action->RequiredTags))
        return false;
    
    // 차단 태그 체크
    if (Action->BlockedTags.Num() > 0 && ActiveTags.HasAny(Action->BlockedTags))
        return false;
    
    return true;
}

void UActionComponent::InterruptActionsWithTags(const FGameplayTagContainer& Tags)
{
    for (int32 i = ActiveActions.Num() - 1; i >= 0; --i)
    {
        UBaseAction* ActiveAction = ActiveActions[i];
        if (ActiveAction && ActiveAction->bInterruptible)
        {
            // 활성 액션의 GrantedTags와 새 액션의 BlockedTags 비교
            if (ActiveAction->GrantedTags.HasAny(Tags))
            {
                ActiveAction->Interrupt();
            }
        }
    }
}

bool UActionComponent::IsActionActive(const FGameplayTag& ActionTag) const
{
    if (!ActionTag.IsValid())
        return false;

    UBaseAction* Action = ActionInstances.FindRef(ActionTag);
    return Action && Action->IsActive();
}

void UActionComponent::CreateActionInstances()
{
    if (!DefaultActionSet)
    {
        UE_LOG(LogTemp, Error, TEXT("ActionComponent: DefaultActionSet is missing!"));
        return;
    }

    for (const FActionDefinition& ActionDef : DefaultActionSet->DefaultActions)
    {
        if (!ActionDef.ActionClass || !ActionDef.ActionTag.IsValid()) continue;
        if (ActionInstances.Contains(ActionDef.ActionTag)) continue;

        UBaseAction* NewAction = NewObject<UBaseAction>(this, ActionDef.ActionClass);
        NewAction->Initialize(GetOwner()); // SourceObject는 nullptr (기본 스킬이므로)
        NewAction->ActionTag = ActionDef.ActionTag;

        ActionInstances.Add(ActionDef.ActionTag, NewAction);
    }
}

void UActionComponent::OnActionCompleted(UBaseAction* Action)
{
    if (!Action) return;

    ActiveActions.Remove(Action);
    
    if (Action->bWantsTick)
    {
        TickingActions.Remove(Action);
    }
}


// 추가 유틸리티 함수들
TArray<FGameplayTag> UActionComponent::GetRegisteredActionTags() const
{
    TArray<FGameplayTag> Tags;
    ActionInstances.GetKeys(Tags);
    return Tags;
}

TArray<UBaseAction*> UActionComponent::GetActionsByCategory(const FGameplayTag& CategoryTag) const
{
    TArray<UBaseAction*> CategoryActions;
    
    for (const auto& ActionPair : ActionInstances)
    {
        UBaseAction* Action = ActionPair.Value;
        if (Action && Action->GetActionTag().MatchesTag(CategoryTag))
        {
            CategoryActions.Add(Action);
        }
    }
    
    return CategoryActions;
}

void UActionComponent::GetActiveActionTags(TArray<FGameplayTag>& OutTags) const
{
    OutTags.Empty();
    
    for (const UBaseAction* Action : ActiveActions)
    {
        if (Action && Action->IsActive())
        {
            OutTags.Add(Action->GetActionTag());
        }
    }
}

// 디버깅용 유틸리티 함수들
void UActionComponent::DebugLogActiveActions() const
{
    UE_LOG(LogTemp, Log, TEXT("=== Active Actions Debug ==="));
    UE_LOG(LogTemp, Log, TEXT("Active Actions Count: %d"), ActiveActions.Num());
    
    for (int32 i = 0; i < ActiveActions.Num(); ++i)
    {
        UBaseAction* Action = ActiveActions[i];
        if (Action)
        {
            UE_LOG(LogTemp, Log, TEXT("[%d] %s - Active: %s, TimeActive: %.2f"), 
                   i, 
                   *Action->GetActionTag().ToString(), 
                   Action->IsActive() ? TEXT("TRUE") : TEXT("FALSE"),
                   Action->GetTimeActive());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[%d] NULL ACTION"), i);
        }
    }
    UE_LOG(LogTemp, Log, TEXT("=== End Debug ==="));
}

void UActionComponent::DebugLogAllActions() const
{
    UE_LOG(LogTemp, Log, TEXT("=== All Actions Debug ==="));
    UE_LOG(LogTemp, Log, TEXT("Registered Actions Count: %d"), ActionInstances.Num());
    
    for (const auto& ActionPair : ActionInstances)
    {
        const FGameplayTag& Tag = ActionPair.Key;
        UBaseAction* Action = ActionPair.Value;
        
        if (Action)
        {
            UE_LOG(LogTemp, Log, TEXT("%s - Active: %s, Cooldown: %.2f"), 
                   *Tag.ToString(), 
                   Action->IsActive() ? TEXT("TRUE") : TEXT("FALSE"),
                   Action->GetCooldownRemaining());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("%s - NULL ACTION"), *Tag.ToString());
        }
    }
    UE_LOG(LogTemp, Log, TEXT("=== End Debug ==="));
}

FString UActionComponent::GetDebugString() const
{
    return {};
}


void UActionComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
}

bool UActionComponent::CanEditChange(const FProperty* InProperty) const
{
    return Super::CanEditChange(InProperty);
}
