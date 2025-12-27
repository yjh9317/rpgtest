// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayerCaptureActor.generated.h"

class USpotLightComponent;
class AWindDirectionalSource;

UCLASS()
class RPGSYSTEM_API APlayerCaptureActor : public AActor
{
	GENERATED_BODY()
	
public:	
	APlayerCaptureActor();

	// UFUNCTION()
	// void StartCapture();
	// UFUNCTION()
	// void EndCapture();
protected:

	// UFUNCTION()
	// void AttachItem(const FItemData& InItemData);
	//
	// UFUNCTION()
	// void DetachItem(const FItemData& InItemData);
	//
	// UFUNCTION()
	// void AddCaptureRotation(FRotator Rotator);
	// UFUNCTION()
	// void InitializePlayerCapture(UEquipmentComponent* EquipComp);
	//
	// void SpawnWindComponent(float WindStrength = 1.f,float WindSpeed = 1.f);
private:
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Component",meta=(AllowPrivateAccess="true"))
	TObjectPtr<USceneComponent> DefaultScene;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Mesh",meta=(AllowPrivateAccess="true"))
	TObjectPtr<USceneComponent> RootOfCharacter;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Mesh",meta=(AllowPrivateAccess="true"))
	TObjectPtr<USkeletalMeshComponent> RootSkeletalMeshComponent;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Mesh",meta=(AllowPrivateAccess="true"))
	TObjectPtr<USkeletalMeshComponent> BootsMeshComponent;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Mesh",meta=(AllowPrivateAccess="true"))
	TObjectPtr<USkeletalMeshComponent> LegsMeshComponent;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Mesh",meta=(AllowPrivateAccess="true"))
	TObjectPtr<USkeletalMeshComponent> GlovesMeshComponent;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Mesh",meta=(AllowPrivateAccess="true"))
	TObjectPtr<USkeletalMeshComponent> ChestMeshComponent;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Mesh",meta=(AllowPrivateAccess="true"))
	TObjectPtr<USkeletalMeshComponent> HeadMeshComponent;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Component",meta=(AllowPrivateAccess="true"))
	TObjectPtr<USpotLightComponent> SpotLightComponent;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Component",meta=(AllowPrivateAccess="true"))
	TObjectPtr<USceneCaptureComponent2D> SceneCaptureComponent2D;

	// UPROPERTY()
	// TObjectPtr<UEquipmentComponent> PlayerEquipment;
	//
	// UPROPERTY()
	// TObjectPtr<UInventoryComponent> PlayerInventory;
	//
	// TMap<EItemSlot, ABaseEquippable*> EquippableRef;

	UPROPERTY()
	TObjectPtr<AWindDirectionalSource> WindActor;

};
