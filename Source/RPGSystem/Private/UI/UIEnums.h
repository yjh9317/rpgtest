#pragma once

#include "CoreMinimal.h"
#include "Slate/WidgetTransform.h"
#include "UIEnums.generated.h"

USTRUCT(BlueprintType)
struct FTextInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	bool UseManualFontInfo;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	FSlateFontInfo FontInfo;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	float Scale;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	FLinearColor ColorAndOpacity;
};

USTRUCT(BlueprintType)
struct FIconInfoV1
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	UObject* IconTexture;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	float Size;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	FLinearColor IconColor;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	FLinearColor HighlightColor;
};


USTRUCT(BlueprintType)
struct FIconInfoV2
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	UObject* IconTexture;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	FWidgetTransform Transform;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	FLinearColor IconColor; 
};


USTRUCT(BlueprintType)
struct FItemIconInfo
{
	GENERATED_BODY()
	
};

USTRUCT(BlueprintType)
struct FProgressBarInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	bool UseProgressBar;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	float MinValue;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	float MaxValue;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	float LastValue;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	float NewValue;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	FLinearColor BackgroundColor;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	FLinearColor FillColor;
};


USTRUCT(BlueprintType)
struct FToolTipEntryInfo
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	FText Text;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	FTextInfo TextInfo;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	UObject* Icon;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	float IconScale;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	FLinearColor IconColor;
};

USTRUCT(BlueprintType)
struct FToolTipBaseInfo
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	float WidthSize;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	FToolTipEntryInfo AppendToolTips;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	FTextInfo ToolTipTextInfo;
};


USTRUCT(BlueprintType)
struct FTask
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	UObject* Object;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	FText ObjectName;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	FText Description;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	FTextInfo DescriptionFontInfo;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	bool IsOptionalTask;
};

USTRUCT(BlueprintType)
struct FTaskBaseInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	float PaddingLeft;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	float PaddingTop;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	float TasksSpacing;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	FIconInfoV1 IconEmpty;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	FIconInfoV1 IconChecked;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	FIconInfoV1 IconFailed;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	FIconInfoV1 IconActive;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	FIconInfoV1 TextEmpty;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	FIconInfoV1 TextChecked;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	FIconInfoV1 TextFailed;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	FIconInfoV1 TextActive;
	
};

UENUM(BlueprintType)
enum EDistanceUnit
{
	Miles,
	Kilometers
};

USTRUCT(BlueprintType)
struct FDistanceInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	bool UseManualDistanceInfo;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	TEnumAsByte<EDistanceUnit> DistanceUnit;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	int32 DistanceSteps_km;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	int32 DistanceSteps_m;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	int32 DistanceSteps_mi;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	int32 DistanceSteps_ya;
};



USTRUCT(BlueprintType)
struct FDistantMarkerInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	bool UseDistantMarker;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	UObject* Icon;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	float Scale;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	FLinearColor Tint;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	bool ShowDistanceText;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	FTextInfo DistanceTextInfo;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	int32 DistanceTextVisibilityRadius;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	int32 MarkerVisibilityRadius;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	bool MarkerVisibilitySmoothTransition;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	bool UsePointerArrow;
};