// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/Interface/InteractableInterface.h"

// Add default functionality here for any IInteractableInterface functions that are not pure virtual.
void IInteractableInterface::Initialize()
{
}

void IInteractableInterface::EndInteraction(AActor* Interactor)
{
}

void IInteractableInterface::Interaction(AActor* Interactor)
{
}

void IInteractableInterface::RemoveInteraction()
{
}

bool IInteractableInterface::CanBeInteractedWith()
{
	return true;
}
