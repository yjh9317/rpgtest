// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CrosshairWidget.generated.h"

class UOverlay;

UENUM()
enum class ECrosshairType
{
	ECrosshair_None,
	ECrosshair_Cross,
	ECrosshair_Circle,
};

/** Please add a struct description */
USTRUCT(BlueprintType)
struct FCrosshairInfo
{
	GENERATED_BODY()
public:
	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Overall Size", MakeStructureDefaultValue="(X=96.000000,Y=96.000000)"))
	FVector2D OverallSize;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Detail Size", MakeStructureDefaultValue="12.000000"))
	float DetailSize;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Color And Opacity"))
	FLinearColor ColorAndOpacity;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Stretch XY", MakeStructureDefaultValue="(X=1.000000,Y=1.000000)"))
	FVector2D StretchXY;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="MovementStrength"))
	float MovementStrength;
};


UCLASS()
class RPGSYSTEM_API UCrosshairWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void CreateCrosshairWidget();
	void UpdateCrosshair();
	void RemoveCrosshair();
	void AddCrosshair();
	void HitFeedback();
	void Recoil();
	
	void FindCrosshair();
	UWidget* GetCurrentCrosshair();
	
public:
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	TObjectPtr<UOverlay> OV_Content;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	ECrosshairType CrosshairType = ECrosshairType::ECrosshair_None;
};
