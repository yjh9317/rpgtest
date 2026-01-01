
#pragma once

#include "CoreMinimal.h"
#include "MenuEnum.generated.h"

UENUM(BlueprintType)
enum class EMenuTab : uint8
{
	None        UMETA(DisplayName = "None"),
	Inventory   UMETA(DisplayName = "Inventory"),
	Quest       UMETA(DisplayName = "Quest Journal"),
	Map         UMETA(DisplayName = "Map"),
	Skills      UMETA(DisplayName = "Skills"),
	Crafting    UMETA(DisplayName = "Crafting"),
	System      UMETA(DisplayName = "System")
};