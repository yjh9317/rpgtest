// ActionBTUtils.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActionComponent.h"
#include "GameFramework/Actor.h"


namespace ActionBTUtils
{
	inline UActionComponent* GetActionComponentFromActor(AActor* Actor)
	{
		if (!Actor) return nullptr;
		
		if (UActionComponent* AC = Actor->FindComponentByClass<UActionComponent>())
		{
			return AC;
		}

		if (APawn* Pawn = Cast<APawn>(Actor))
		{
			if (AController* Controller = Pawn->GetController())
			{
				if (UActionComponent* AC = Controller->FindComponentByClass<UActionComponent>())
				{
					return AC;
				}
			}
		}

		return nullptr;
	}
}
