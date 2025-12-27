#pragma once

#include "CoreMinimal.h"

/*
	Project Settings -> Collision -> Trace Channels 설정과 일치해야 함
	DefaultEngine.ini에서 확인가능
	[/Script/Engine.CollisionProfile]
	; ... (다른 설정들)
	DefaultChannelResponses=(Channel=ECC_GameTraceChannel8, Name="Interactable", DefaultResponse=ECR_Ignore, bTraceType=True, bStaticObject=False)
*/

// Trace Channel
#define ECC_Interactable    ECC_GameTraceChannel1 
#define ECC_Weapon          ECC_GameTraceChannel2
// Object Channel
#define ECO_Interactable    ECC_GameTraceChannel8 