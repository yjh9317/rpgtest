// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/EquipmentActor.h"

// Sets default values
AEquipmentActor::AEquipmentActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AEquipmentActor::InitializeEquipment(USkeletalMesh* SkelMesh, UStaticMesh* StatMesh)
{
}

void AEquipmentActor::OnEquip()
{
}

void AEquipmentActor::OnUnequip()
{
}

