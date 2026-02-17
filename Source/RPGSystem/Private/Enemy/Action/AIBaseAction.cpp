// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Action/AIBaseAction.h"

UAIBaseAction::UAIBaseAction(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	bEnableGestureDetection = false;
	bEnableDoubleClick = false;
}

void UAIBaseAction::Initialize(AActor* NewActionOwner, UObject* NewSourceObject)
{
	Super::Initialize(NewActionOwner, NewSourceObject);
}

bool UAIBaseAction::CanExecute() const
{
	return Super::CanExecute();
}

bool UAIBaseAction::CanExecuteWithContext(const FAIActionContext& Context) const
{
	return true;
}

bool UAIBaseAction::IsTargetInRange() const
{
	return true;
}

bool UAIBaseAction::HasValidTarget() const
{
	return true;
}

void UAIBaseAction::SetContext(const FAIActionContext& NewContext)
{
}

void UAIBaseAction::UpdateContextFromTargeting()
{
}

AEnemyController* UAIBaseAction::GetAIController() const
{
	return nullptr;
}

float UAIBaseAction::EvaluateUtility(const FAIActionContext& Context) const
{
	return 0.f;
}

void UAIBaseAction::OnExecute_Implementation()
{
	Super::OnExecute_Implementation();
}

void UAIBaseAction::OnComplete_Implementation()
{
	Super::OnComplete_Implementation();
}

float UAIBaseAction::GetDistanceToTarget() const
{
	return 0.f;
}

bool UAIBaseAction::HasLineOfSightToTarget() const
{
	return true;
}
