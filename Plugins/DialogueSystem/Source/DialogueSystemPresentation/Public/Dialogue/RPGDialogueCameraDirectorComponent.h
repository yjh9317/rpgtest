#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DialogueRuntimeTypes.h"
#include "RPGDialogueCameraDirectorComponent.generated.h"

class ACameraActor;
class APlayerController;

UCLASS(ClassGroup=(Dialogue), meta=(BlueprintSpawnableComponent))
class DIALOGUESYSTEMPRESENTATION_API URPGDialogueCameraDirectorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URPGDialogueCameraDirectorComponent();

	UFUNCTION(BlueprintCallable, Category = "Dialogue Camera")
	void EnterDialogueCamera(AActor* Speaker);

	UFUNCTION(BlueprintCallable, Category = "Dialogue Camera")
	void ApplyCue(const FDialogueCinematicCue& Cue, AActor* Speaker);

	UFUNCTION(BlueprintCallable, Category = "Dialogue Camera")
	void ExitDialogueCamera();

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Camera")
	float DefaultBlendTime = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Camera")
	float DefaultDistance = 240.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Camera")
	float DefaultHeight = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Camera")
	float DefaultSideOffset = 45.0f;

private:
	UPROPERTY()
	TObjectPtr<ACameraActor> DialogueCameraActor;

	TWeakObjectPtr<AActor> CachedViewTarget;
	FRotator CachedControlRotation = FRotator::ZeroRotator;

	APlayerController* ResolveOwningPC() const;
	void EnsureCameraActor();
	FTransform BuildShotTransform(const FDialogueCinematicCue& Cue, AActor* Speaker) const;
};

