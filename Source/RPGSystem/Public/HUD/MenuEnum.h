
#pragma once

#include "CoreMinimal.h"
#include "MenuEnum.generated.h"

UENUM(BlueprintType)
enum class EMainMenuTab : uint8
{
	Inventory,
	Character,
	Quest,
	WorldMap,
	Skill,
	Journal,
	Crafting,
	System,
	Count
};