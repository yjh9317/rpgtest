#pragma once

#include "CoreMinimal.h"
#include "Dialogue.h"
#include "DialogueRuntimeTypes.generated.h"

UENUM(BlueprintType)
enum class EDialogueOptionTone : uint8
{
	Neutral,
	Friendly,
	Aggressive,
	Deceptive,
	Investigative
};

UENUM(BlueprintType)
enum class EDialogueCameraShotType : uint8
{
	None,
	TwoShot,
	OverShoulderPlayer,
	OverShoulderNPC,
	CloseUpPlayer,
	CloseUpNPC,
	Wide
};

USTRUCT(BlueprintType)
struct FDialogueOptionMeta
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Option")
	EDialogueOptionTone Tone = EDialogueOptionTone::Neutral;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Option")
	bool bHideIfUnavailable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Option")
	bool bSelectableWhenUnavailable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Option")
	FText UnavailableReason;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Check")
	bool bHasSkillCheck = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Check")
	FName SkillCheckId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Check")
	int32 SkillCheckDC = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Check")
	int32 PreviewRoll = -1;
};

USTRUCT(BlueprintType)
struct FDialogueRuntimeOption
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Option")
	FDialogueNode Node;

	UPROPERTY(BlueprintReadOnly, Category = "Option")
	FDialogueOptionMeta Meta;

	UPROPERTY(BlueprintReadOnly, Category = "Option")
	int32 OptionIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Option")
	bool bAvailable = true;

	UPROPERTY(BlueprintReadOnly, Category = "Option")
	bool bSelectable = true;

	UPROPERTY(BlueprintReadOnly, Category = "Option")
	bool bVisible = true;

	UPROPERTY(BlueprintReadOnly, Category = "Option")
	FText DisplayText;

	UPROPERTY(BlueprintReadOnly, Category = "Option")
	FText DecoratedPrefix;

	UPROPERTY(BlueprintReadOnly, Category = "Option")
	FText UnavailableReason;
};

USTRUCT(BlueprintType)
struct FDialogueSpeakerPayload
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Speaker")
	FDialogueNode Node;

	UPROPERTY(BlueprintReadOnly, Category = "Speaker")
	TObjectPtr<AActor> SpeakerActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Speaker")
	FText SpeakerName;

	UPROPERTY(BlueprintReadOnly, Category = "Speaker")
	TObjectPtr<UTexture2D> Portrait = nullptr;
};

USTRUCT(BlueprintType)
struct FDialogueCinematicCue
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	EDialogueCameraShotType ShotType = EDialogueCameraShotType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float BlendTime = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float HoldTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float Distance = 240.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float Height = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float SideOffset = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float YawOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control")
	bool bLockPlayerMovement = true;
};

USTRUCT(BlueprintType)
struct FDialogueHistoryEntryEx
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "History")
	FDialogueNode Node;

	UPROPERTY(BlueprintReadOnly, Category = "History")
	bool bPlayerChoice = false;

	UPROPERTY(BlueprintReadOnly, Category = "History")
	FText DisplayText;

	UPROPERTY(BlueprintReadOnly, Category = "History")
	FDateTime Timestamp;
};
