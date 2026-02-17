#pragma once

#include "CoreMinimal.h"
#include "Character/RPGCharacterBase.h"
#include "Interaction/Interface/InteractableInterface.h"
#include "NPCCharacter.generated.h"

class UDialogueProviderComponent;
class UInteractableComponent;
class UQuestGiverComponent;
class USphereComponent;
class UWidgetComponent;

UCLASS()
class RPGSYSTEM_API ANPCCharacter : public ARPGCharacterBase, public IInteractableInterface
{
	GENERATED_BODY()

public:
	ANPCCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Initialize() override;
	virtual void EndInteraction(AActor* Interactor) override;
	virtual void Interaction(AActor* Interactor) override;
	virtual void RemoveInteraction() override;
	virtual bool CanBeInteractedWith() override;
	virtual void ClientBeginInteraction(AActor* Interactor) override;
	virtual void ClientEndInteraction(AActor* Interactor) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UInteractableComponent> InteractableComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UQuestGiverComponent> QuestGiverComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UDialogueProviderComponent> DialogueProviderComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> InteractionCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UWidgetComponent> InteractionWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	int32 StencilValue = 1;
};
