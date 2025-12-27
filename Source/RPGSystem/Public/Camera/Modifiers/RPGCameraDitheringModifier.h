// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/Data/RPGCameraDataAsset.h"
#include "Camera/Modifiers/RPGCameraModifier.h"
#include "RPGCameraDitheringModifier.generated.h"

UENUM()
enum class EDitherType : uint8
{
	None,
	BlockingLOS,
	OverlappingCamera
};

USTRUCT(BlueprintType)
struct FDitheredActorState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RPG|Modifiers|Camera Dithering")
	TObjectPtr<AActor> Actor;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "RPG|Modifiers|Camera Dithering")
	float CurrentOpacity = 1.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "RPG|Modifiers|Camera Dithering")
	float CollisionTime = 0.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "RPG|Modifiers|Camera Dithering")
	bool bIsDitheringIn = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "RPG|Modifiers|Camera Dithering")
	bool bIsDitheringOut = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "RPG|Modifiers|Camera Dithering")
	EDitherType DitherType = EDitherType::None;

	bool IsValid() const { return Actor != nullptr && DitherType != EDitherType::None; }

	void StartDithering(AActor* InActor, EDitherType InDitherType);

	void Invalidate();

	friend bool operator==(FDitheredActorState const& lhs, FDitheredActorState const& rhs)
	{
		return lhs.Actor == rhs.Actor;
	}

	void ComputeOpacity(float DeltaTime, float DitherInTime, float DitherOutTime, float DitherMin);
};


UCLASS()
class RPGSYSTEM_API URPGCameraDitheringModifier : public URPGCameraModifier
{
	GENERATED_BODY()
public:
	URPGCameraDitheringModifier();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "RPG|Modifiers|Dithering")
	void ResetDitheredActors();

protected:
	virtual bool ModifyCamera(float DeltaTime, FMinimalViewInfo& InOutPOV) override;

	virtual void ApplyDithering(float DeltaTime, FDitheredActorState& DitherState);

private:
#if ENABLE_DRAW_DEBUG
	void RPGDebugDithering(FDitheredActorState& DitherState, float DeltaTime, float DitherMin);
#endif

protected:

	/** Material Parameter Collection for everything dithering-related */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RPG|Modifiers|Dithering")
	TSoftObjectPtr<class UMaterialParameterCollection> DitheringMPC;

	UPROPERTY(Transient)
	TSoftObjectPtr<class UMaterialParameterCollectionInstance> DitheringMPCInstance;

	UPROPERTY(Transient)
	TArray<FDitheredActorState> DitheredActorStates;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RPG|Modifiers|Dithering")
	FCameraDitheringSettings DitheringSettings;
};
