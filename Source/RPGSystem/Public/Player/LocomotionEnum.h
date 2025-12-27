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

UENUM(BlueprintType)
enum class EExtendedPose : uint8
{
    None      UMETA(DisplayName = "None"),
    Swimming  UMETA(DisplayName = "Swimming"),
    Flight    UMETA(DisplayName = "Flight"),
    Rider     UMETA(DisplayName = "Rider"),
    LightDart UMETA(DisplayName = "LightDart"),
};

UENUM(BlueprintType)
enum class EHandIKBones : uint8
{
    UpperArm UMETA(DisplayName = "UpperArm"),
    LowerArm UMETA(DisplayName = "LowerArm")
};

UENUM(BlueprintType)
enum class EOverrideType : uint8
{
    None                        UMETA(DisplayName = "None"),
    FullBody                    UMETA(DisplayName = "FullBody"),
    UpperBody                   UMETA(DisplayName = "UpperBody"),
    UpperBodyMask               UMETA(DisplayName = "UpperBodyMask"),
    UpperBodyRiderMask          UMETA(DisplayName = "UpperBodyRiderMask"),
    UpperBodyFlightMask         UMETA(DisplayName = "UpperBodyFlightMask"),
    LeftArm                     UMETA(DisplayName = "LeftArm"),
    LeftArmRotationBlend        UMETA(DisplayName = "LeftArmRotationBlend"),
    RightArm                    UMETA(DisplayName = "RightArm"),
    RightArmRotationBlend       UMETA(DisplayName = "RightArmRotationBlend"),
    Arms                        UMETA(DisplayName = "Arms"),
    ArmsRotationBlend           UMETA(DisplayName = "ArmsRotationBlend"),
    FingersMask                 UMETA(DisplayName = "FingersMask"),
    LeftFingersMask             UMETA(DisplayName = "LeftFingersMask"),
    RightFingersMask            UMETA(DisplayName = "RightFingersMask"),
    UpperBodyLowerBodySplitMask UMETA(DisplayName = "UpperBodyLowerBodySplitMask"),
    UpperBodyLeftWeapMask       UMETA(DisplayName = "UpperBodyLeftWeapMask"),
    UpperBodyRightWeapMask      UMETA(DisplayName = "UpperBodyRightWeapMask"),
    UpperBodyRightWeap2Mask     UMETA(DisplayName = "UpperBodyRightWeap2Mask"),
    UpperBodyRightWeap3Mask     UMETA(DisplayName = "UpperBodyRightWeap3Mask"),
    UpperBodyAimMask            UMETA(DisplayName = "UpperBodyAimMask"),
    UpperBodyCrouchMask         UMETA(DisplayName = "UpperBodyCrouchMask"),
};

UENUM(BlueprintType)
enum class ERootYawOffsetMode : uint8
{
    BlendOut   UMETA(DisplayName = "BlendOut"),
    Hold       UMETA(DisplayName = "Hold"),
    Accumulate UMETA(DisplayName = "Accumulate")
};

UENUM(BlueprintType)
enum class ERotationMode : uint8
{
    Strafing      UMETA(DisplayName = "Strafing"),
    ForwardFacing UMETA(DisplayName = "ForwardFacing")
};

