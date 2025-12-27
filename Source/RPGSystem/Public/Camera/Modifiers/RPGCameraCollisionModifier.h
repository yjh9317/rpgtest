// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/Data/RPGCameraDataAsset.h"
#include "Camera/Modifiers/RPGCameraModifier.h"
#include "Containers/StaticBitArray.h"
#include "RPGCameraCollisionModifier.generated.h"

/**
 * 
 */
UCLASS(abstract, ClassGroup = "RPG Camera Modifiers", meta = (Deprecated = 5.5))
class RPGSYSTEM_API URPGCameraCollisionModifier : public URPGCameraModifier
{
	GENERATED_BODY()
		
public:
	URPGCameraCollisionModifier();
	// Force collision modifier to use a single ray by another modifier. Do not use this if you're not familiar with it.
	UFUNCTION(BlueprintCallable, Category = "RPG|Modifiers|Collision")
	void AddSingleRayOverrider(UCameraModifier const* OverridingModifier) { if (OverridingModifier) SingleRayOverriders.AddUnique(OverridingModifier); }

	// Remove single ray modifier override. Do not use this if you're not familiar with it.
	UFUNCTION(BlueprintCallable, Category = "RPG|Modifiers|Collision")
	void RemoveSingleRayOverrider(UCameraModifier const* OverridingModifier) { if (OverridingModifier) SingleRayOverriders.Remove(OverridingModifier); }

protected:
	virtual bool ModifyCamera(float DeltaTime, FMinimalViewInfo& InOutPOV) override;

	void UpdatePreventPenetration(float DeltaTime, FMinimalViewInfo& InOutPOV);

	void PreventCameraPenetration(class AActor const& ViewTarget, FVector const& SafeLoc, FVector& OutCameraLoc, float const& DeltaTime, float& OutDistBlockedPct, bool bSingleRayOnly);

	FVector GetTraceSafeLocation(FMinimalViewInfo const& POV);

	void ResetSingleRayOverriders() { SingleRayOverriders.Reset(); }

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RPG|Modifiers|Collision")
	FCameraCollisionSettings CollisionSettings;

	// If you don't want the camera to start close to the character and smoothly pan out once your character is spawned, default-initialize this variable to 1.f.
	UPROPERTY(Transient)
	float AimLineToDesiredPosBlockedPct = 1.f;

	UPROPERTY(Transient)
	TArray<TObjectPtr<const AActor>> DebugActorsHitDuringCameraPenetration;

protected:
	TArray<UCameraModifier const*> SingleRayOverriders;
	TStaticBitArray<128u> CollidingFeelers;

#if ENABLE_DRAW_DEBUG
	mutable float LastDrawDebugTime = -MAX_FLT;
#endif
};
