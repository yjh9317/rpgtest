// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AnimNotifies/AnimNotifyState_DisableJump.h"
#include "Player/RPGPlayerCharacter.h"

void UAnimNotifyState_DisableJump::NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		if (ARPGPlayerCharacter* Player = Cast<ARPGPlayerCharacter>(MeshComp->GetOwner()))
		{
			Player->SetLandRecovery(true);
		}
	}
}

void UAnimNotifyState_DisableJump::NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		if (ARPGPlayerCharacter* Player = Cast<ARPGPlayerCharacter>(MeshComp->GetOwner()))
		{
			Player->SetLandRecovery(false);
		}
	}
}