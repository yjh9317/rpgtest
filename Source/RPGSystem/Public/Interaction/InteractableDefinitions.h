#pragma once

#include "CoreMinimal.h"
#include "InteractableDefinitions.generated.h"

UENUM()
enum class EInteractionInputType
{
	Single,
	Holding,
	MultipleAndMashing
};

UENUM()
enum class EInteractionResponse
{
	Persistent,
	OnlyOnce,
	Temporary
};

UENUM()
enum class EInteractableType : uint8
{
	Pickup UMETA(DisplayName = "Pickup"),
	NonPlayerCharacter UMETA(DisplayName = "NPC"),
	Device UMETA(DisplayName = "Device"),
	Toggle UMETA(DisplayName = "Toggle"),
	Container UMETA(DisplayName = "Container")
};

UENUM()
enum class EOperationStatus
{
	None,
	Complete,
	Failed,
	Reset
};

USTRUCT()
struct FInteractableData
{
	GENERATED_USTRUCT_BODY()

	FInteractableData() :
	InteractableType(EInteractableType::Pickup),
	Name(FText::GetEmpty()),
	Action(FText::GetEmpty()),
	Quantity(0),
	InteractionDuration(0.0f)
	{};

	UPROPERTY(EditInstanceOnly)
	EInteractableType InteractableType;

	UPROPERTY(EditInstanceOnly)
	FText Name;

	UPROPERTY(EditInstanceOnly)
	FText Action;

	UPROPERTY(EditInstanceOnly)
	int8 Quantity;

	UPROPERTY(EditInstanceOnly)
	float InteractionDuration;
};

