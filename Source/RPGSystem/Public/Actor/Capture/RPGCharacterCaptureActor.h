// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RPGCharacterCaptureActor.generated.h"

class UTextRenderComponent;

UCLASS()
class RPGSYSTEM_API ARPGCharacterCaptureActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARPGCharacterCaptureActor();

	virtual void BeginPlay() override;
	
	void StartWindComponent();
	void EndWindComponent();
	void StartCapture();
	void EndCapture();
	void AddCaptureRotation(FRotator InRotator);
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Capture")
	TObjectPtr<USkeletalMeshComponent> PreviewMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Capture")
	TObjectPtr<USceneCaptureComponent2D> CaptureComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Capture")
	TObjectPtr<UWindDirectionalSourceComponent> WindDirectionalSourceComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Capture")
	TObjectPtr<UTextRenderComponent> WindTextRenderComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Capture")
	float WindSpeed = 0.5f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Capture")
	float WindStrength = 0.5f;
public:
	UFUNCTION(BlueprintCallable, Category = "Capture")
	void UpdateAppearance(USkeletalMesh* NewMesh, UMaterialInterface* NewMaterial = nullptr);
};


