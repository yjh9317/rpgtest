// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Data/Fragment/ItemFragment.h"
#include "Item/Data/ItemInstance.h" 
#include "Components/ActorComponent.h"


UItemInstance* UItemFragment::GetOwnerInstance() const
{
	return Cast<UItemInstance>(GetOuter());
}

AActor* UItemFragment::GetOwnerActor() const
{
	if (UItemInstance* OwnerInstance = GetOwnerInstance())
	{
		if (UActorComponent* OwnerComponent = Cast<UActorComponent>(OwnerInstance->GetOuter()))
		{
			return OwnerComponent->GetOwner();
		}
	}
	return nullptr;
}