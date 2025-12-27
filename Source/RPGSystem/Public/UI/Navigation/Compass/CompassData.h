#pragma once

#include "CoreMinimal.h"
#include "Slate/WidgetTransform.h"
#include "CompassData.generated.h"

UENUM()
enum class ERotationType
{
	ControlRotation,
	CameraRotation
};

USTRUCT(BlueprintType)
struct FCompassMarkerInfo
{
	GENERATED_BODY()
	
	UPROPERTY()
	UObject* MarkerIcon;

	UPROPERTY()
	float MarkerScale;

	UPROPERTY()
	FLinearColor MarkerColor;

	UPROPERTY()
	FLinearColor HighlightColor;

	UPROPERTY()
	bool PingMarker;

	UPROPERTY()
	bool ShowMarkerDistance;

	UPROPERTY()
	int32 MarkerVisibilityRadius;

	UPROPERTY()
	bool MarkerVisibilitySmoothTransition;

	UPROPERTY()
	bool ScreenPersistance;
};

USTRUCT(BlueprintType)
struct FCompassInitData
{
	GENERATED_BODY()

	UPROPERTY()
	FLinearColor BackgroundColor;

	UPROPERTY()
	float BackgroundOpacity;

	UPROPERTY()
	UTexture2D* CompassMask;

	UPROPERTY()
	FLinearColor CompassTint;

	UPROPERTY()
	UObject* PointerTexture;

	UPROPERTY()
	float PointerScale;

	UPROPERTY()
	float PointerYTranslation;

	UPROPERTY()
	FLinearColor PointerTint;

	UPROPERTY()
	float MarkerYTranslation;

	UPROPERTY()
	UObject* BackgroundTexture;
	
	UPROPERTY()
	FWidgetTransform BackgroundTransform;
};