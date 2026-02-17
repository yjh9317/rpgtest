#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DialogueRuntimeTypes.h"
#include "DialogueManagerComponent.generated.h"

class UDialogueViewModel;
class UDialogueProviderComponent;
class UDialogueUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueStartedSignature, UDialogueProviderComponent*, Provider);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueEndedSignature, bool, bCompleted);

UCLASS(ClassGroup=(Dialogue), meta=(BlueprintSpawnableComponent))
class DIALOGUESYSTEMPRESENTATION_API UDialogueManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDialogueManagerComponent();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Dialogue UI")
	TSubclassOf<UDialogueUserWidget> DialogueWidgetClass;

	UPROPERTY()
	TObjectPtr<UDialogueViewModel> DialogueViewModel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Options")
	bool bBlockInputDuringDialogue = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Options")
	bool bPauseGameDuringDialogue = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Options")
	bool bShowMouseCursor = true;

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool StartDialogue(UDialogueProviderComponent* Provider);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void EndDialogue(bool bCompleted = false);

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool IsInDialogue() const { return bIsInDialogue; }

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	UDialogueProviderComponent* GetCurrentProvider() const { return CurrentProvider; }

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	UDialogueUserWidget* GetDialogueWidget() const { return DialogueWidget; }

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	UDialogueViewModel* GetDialogueViewModel() const { return DialogueViewModel; }

	UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
	FOnDialogueStartedSignature OnDialogueStarted;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
	FOnDialogueEndedSignature OnDialogueEnded;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY()
	TObjectPtr<UDialogueUserWidget> DialogueWidget;

	UPROPERTY()
	TObjectPtr<UDialogueProviderComponent> CurrentProvider;

	UPROPERTY()
	bool bIsInDialogue = false;

	void CreateDialogueWidget();
	void RemoveDialogueWidget();
	void SetupInputMode(bool bDialogueMode);
	void EndDialogueInternal(bool bCompleted, bool bNotifyViewModel);

	UFUNCTION()
	void OnViewModelDialogueClosed(bool bCompleted);

	UFUNCTION()
	void OnViewModelCinematicCue(const FDialogueCinematicCue& Cue);
};

