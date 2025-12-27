// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RPGSYSTEM_API IInteractableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void Initialize();
	virtual void EndInteraction(AActor* Interactor);
	virtual void Interaction(AActor* Interactor); // Server	실제 게임 로직 처리
	virtual void RemoveInteraction();
	virtual bool CanBeInteractedWith();
	virtual void ClientPrepareInteraction() {}; // Client	상호작용 준비/전처리
	virtual void ClientBeginInteraction(AActor* Interactor) {}; // Client	시각/청각적 피드백 시작
	virtual void ClientEndInteraction(AActor* Interactor) {}; // Client	피드백 종료/정리
	
};
