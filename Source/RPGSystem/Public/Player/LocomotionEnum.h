#pragma once

#include "CoreMinimal.h"
#include "LocomotionEnum.generated.h"

UENUM(BlueprintType)
enum class EPlayerMovementMode : uint8
{
    None     UMETA(DisplayName = "None"),
    Walk     UMETA(DisplayName = "Walk"),
    Jog      UMETA(DisplayName = "Jog"),
    Crouch   UMETA(DisplayName = "Crouch"),
    Fly      UMETA(DisplayName = "Fly"),
    Swimming UMETA(DisplayName = "Swimming"),
    Riding   UMETA(DisplayName = "Riding"),
    Custom   UMETA(DisplayName = "Custom")
};

UENUM(BlueprintType)
enum class ECardinalDirection : uint8
{
    Forward  UMETA(DisplayName = "Forward"),
    Backward UMETA(DisplayName = "Backward"),
    Left     UMETA(DisplayName = "Left"),
    Right    UMETA(DisplayName = "Right"),
    None     UMETA(DisplayName = "None")
};
