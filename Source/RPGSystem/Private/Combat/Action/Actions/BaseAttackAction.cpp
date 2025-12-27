// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Action/Actions/BaseAttackAction.h"

#include "Combat/Components/CombatComponent.h"
#include "GameFramework/Character.h"


UBaseAttackAction::UBaseAttackAction(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // 예: 태그 기본값
    // ActionTag = FGameplayTag::RequestGameplayTag(TEXT("Action.Attack.Base"));
}

void UBaseAttackAction::Initialize(AActor* NewActionOwner, UObject* NewSourceObject)
{
    Super::Initialize(NewActionOwner, NewSourceObject);
    
    if (!OwnerCharacter)
    {
        return;
    }

    // OwnerCharacter나 Controller에서 CombatComponent를 가져오는 패턴
    // if (IComponentManager* ComponentManager = Cast<IComponentManager>(OwnerCharacter))
    // {
    //     CachedCombatComponent = ComponentManager->GetCombatComponent();
    // }
    // else if (OwnerCharacter->GetController())
    // {
    //     if (IComponentManager* ControllerManager = Cast<IComponentManager>(OwnerCharacter->GetController()))
    //     {
    //         CachedCombatComponent = ControllerManager->GetCombatComponent();
    //     }
    // }
}

UCombatComponent* UBaseAttackAction::GetCombatComponent() const
{
    if (CachedCombatComponent)
    {
        return CachedCombatComponent;
    }

    if (!OwnerCharacter)
    {
        return nullptr;
    }

    return OwnerCharacter->FindComponentByClass<UCombatComponent>();
}

bool UBaseAttackAction::HasAttackResources() const
{
    // TODO: StatsComponent와 연결해서 스태미너 체크
    // 일단은 BaseAction의 HasRequiredResources() 결과와 동일하게 처리
    return HasRequiredResources();
}

void UBaseAttackAction::ConsumeAttackResources()
{
    // TODO: 실제 스태미너 감소
    // 여기서 BaseStaminaCost 사용해서 StatsComponent 감소시키면 됨
}



bool UBaseAttackAction::CanExecute() const
{
    if (!Super::CanExecute())
    {
        return false;
    }

    UCombatComponent* CombatComp = const_cast<UBaseAttackAction*>(this)->GetCombatComponent();
    if (!CombatComp)
    {
        return false;
    }

    if (!CombatComp->IsAlive())
    {
        return false;
    }

    if (!HasAttackResources())
    {
        return false;
    }

    return true;
}

float UBaseAttackAction::ApplyAttackToCurrentTarget()
{
    UCombatComponent* CombatComp = GetCombatComponent();
    if (!CombatComp)
    {
        return 0.f;
    }

    AActor* Target = CombatComp->GetCurrentTarget();
    if (!Target)
    {
        return 0.f;
    }

    ConsumeAttackResources();

    FDamageInfo DamageInfo;
    DamageInfo.BaseDamage = BaseDamage;
    DamageInfo.DamageTags = DamageTags;
    DamageInfo.SourceActor = OwnerCharacter;

    return CombatComp->ApplyDamage(Target, DamageInfo);
}