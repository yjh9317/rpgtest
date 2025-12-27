// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EquipmentActor.generated.h"

UCLASS()
class RPGSYSTEM_API AEquipmentActor : public AActor
{
	GENERATED_BODY()
	
public:    
	AEquipmentActor();

	// 무기/방어구 메쉬 (상황에 따라 둘 중 하나 사용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<USkeletalMeshComponent> MainMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	/** 초기화: Fragment에서 정보를 받아 외형 설정 */
	virtual void InitializeEquipment(USkeletalMesh* SkelMesh, UStaticMesh* StatMesh);
	
	// Collision , Sound
	UFUNCTION()
	void OnEquip();
	
	UFUNCTION()
	void OnUnequip();
};
