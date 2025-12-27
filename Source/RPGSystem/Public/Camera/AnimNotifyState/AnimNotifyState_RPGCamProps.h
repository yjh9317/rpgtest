// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_RPGCamProps.generated.h"

/**
 * Anim Notify State to send requests to the FOVAnimNotifyModifier to change the FOV during animations.
 */
UCLASS()
class RPGSYSTEM_API URPGFOVRequestAnimNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

protected:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
	virtual FLinearColor GetEditorColor() override;

protected:
	/** Horizontal FOV to set. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float TargetFOV = 90.f;

	/** Time needed for the FOV value to ease into the TargetValue. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float BlendInDuration = 0.5f;

	/** Controls the blending in. The curve has to be normalized (going from 0 to 1). Leave empty or use Hermit if unsure. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	TObjectPtr<UCurveFloat> BlendInCurve = nullptr;

	/** Time needed for the FOV value to ease out from the TargetValue into the normal gameplay value. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float BlendOutDuration = 0.5f;

	/** Controls the blending Out. The curve has to be normalized (going from 0 to 1). Leave empty or use Hermit if unsure. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	TObjectPtr<UCurveFloat> BlendOutCurve = nullptr;

#if WITH_EDITORONLY_DATA
	/** Whether the name of the notify should include the request Id. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	bool bShowRequestIdInName = false;
#endif

protected:
	FGuid RequestId = FGuid::NewGuid();
};

/**
 * Anim Notify State to send requests to the ArmOffsetAnimNotifyModifier to change the Arm Offsets during animations.
 */
UCLASS()
class RPGSYSTEM_API URPGArmOffsetRequestAnimNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

protected:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
	virtual FLinearColor GetEditorColor() override;

protected:
	/** Whether to change socket offset. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|SocketOffset")
	bool bModifySocketOffset = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|SocketOffset", meta = (EditCondition="bModifySocketOffset"))
	FVector TargetSocketOffset = FVector::Zero();

	/** Time needed for the Socket Offset value to ease into the TargetValue. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|SocketOffset", meta = (EditCondition = "bModifySocketOffset"))
	float SocketOffsetBlendInDuration = 0.5f;

	/** Controls the blending in. The curve has to be normalized (going from 0 to 1). Leave empty or use Hermit if unsure. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|SocketOffset", meta = (EditCondition = "bModifySocketOffset"))
	TObjectPtr<UCurveFloat> SocketOffsetBlendInCurve = nullptr;

	/** Time needed for the Socket Offset value to ease out from the TargetValue into the normal gameplay value. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|SocketOffset", meta = (EditCondition = "bModifySocketOffset"))
	float SocketOffsetBlendOutDuration = 0.5f;

	/** Controls the blending Out. The curve has to be normalized (going from 0 to 1). Leave empty or use Hermit if unsure. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|SocketOffset", meta = (EditCondition = "bModifySocketOffset"))
	TObjectPtr<UCurveFloat> SocketOffsetBlendOutCurve = nullptr;

	/** Whether to change socket offset. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|TargetOffset")
	bool bModifyTargetOffset = false;

	/** Target Offset to set. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|TargetOffset", meta = (EditCondition = "bModifyTargetOffset"))
	FVector TargetTargetOffset = FVector::Zero();

	/** Time needed for the Target Offset value to ease into the TargetValue. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|TargetOffset", meta = (EditCondition = "bModifyTargetOffset"))
	float TargetOffsetBlendInDuration = 0.5f;

	/** Controls the blending in. The curve has to be normalized (going from 0 to 1). Leave empty or use Hermit if unsure. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|TargetOffset", meta = (EditCondition = "bModifyTargetOffset"))
	TObjectPtr<UCurveFloat> TargetOffsetBlendInCurve = nullptr;

	/** Time needed for the Target Offset value to ease out from the TargetValue into the normal gameplay value. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|TargetOffset", meta = (EditCondition = "bModifyTargetOffset"))
	float TargetOffsetBlendOutDuration = 0.5f;

	/** Controls the blending Out. The curve has to be normalized (going from 0 to 1). Leave empty or use Hermit if unsure. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|TargetOffset", meta = (EditCondition = "bModifyTargetOffset"))
	TObjectPtr<UCurveFloat> TargetOffsetBlendOutCurve = nullptr;

#if WITH_EDITORONLY_DATA
	/** Whether the name of the notify should include the request Id. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	bool bShowRequestIdInName = false;
#endif

protected:
	FGuid RequestId = FGuid::NewGuid();
};

/**
 * Anim Notify State to send requests to the ArmLengthAnimNotifyModifier to change the Arm Length during animations.
 */
UCLASS()
class RPGSYSTEM_API URPGArmLengthRequestAnimNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

protected:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
	virtual FLinearColor GetEditorColor() override;

protected:
	/** ArmLength to set. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float TargetArmLength = 0.f;

	/** Time needed for the ArmLength value to ease into the TargetValue. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float BlendInDuration = 0.5f;

	/** Controls the blending in. The curve has to be normalized (going from 0 to 1). Leave empty or use Hermit if unsure. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	TObjectPtr<UCurveFloat> BlendInCurve = nullptr;

	/** Time needed for the ArmLength value to ease out from the TargetValue into the normal gameplay value. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float BlendOutDuration = 0.5f;

	/** Controls the blending Out. The curve has to be normalized (going from 0 to 1). Leave empty or use Hermit if unsure. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	TObjectPtr<UCurveFloat> BlendOutCurve = nullptr;

#if WITH_EDITORONLY_DATA
	/** Whether the name of the notify should include the request Id. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	bool bShowRequestIdInName = false;
#endif

protected:
	FGuid RequestId = FGuid::NewGuid();
};