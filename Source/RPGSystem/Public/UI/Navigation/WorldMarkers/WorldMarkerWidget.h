// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/UIEnums.h"
#include "WorldMarkerWidget.generated.h"


class UVerticalBox;
class USizeBox;
class UOverlay;
class UTextBlock;
class UImage;
class UBorder;

/** Please add a struct description */
USTRUCT(BlueprintType)
struct FWorldMarkerValues
{
	GENERATED_BODY()
public:
	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="WorldMarkerScreenLimitation"))
	float WorldMarkerScreenLimitation;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="ArrowIconScale"))
	float ArrowIconScale;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="ArrowIconTexture"))
	TObjectPtr<UObject> ArrowIconTexture;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="PingIcon1"))
	TObjectPtr<UObject> PingIcon1;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="PingIcon2"))
	TObjectPtr<UObject> PingIcon2;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="SmoothVisibilityDivider"))
	float SmoothVisibilityDivider;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="DistanceVisibilityInterpSpeed"))
	float DistanceVisibilityInterpSpeed;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="PositionInterpSpeed"))
	float PositionInterpSpeed;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="PositionY_SwitchTolerance"))
	float PositionY_SwitchTolerance;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="ArrowVisibilityEdgeTolerance"))
	float ArrowVisibilityEdgeTolerance;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="ArrowVisibilityInterpSpeed"))
	float ArrowVisibilityInterpSpeed;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="TextVisibilityInterpSpeed"))
	float TextVisibilityInterpSpeed;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="TextVisibilityEdgeTolerance"))
	float TextVisibilityEdgeTolerance;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="TextVisibilityScreenCenterTolerance"))
	float TextVisibilityScreenCenterTolerance;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="MarkerNameVisibilityDuration"))
	float MarkerNameVisibilityDuration;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Ping Playback Speed"))
	float PingPlaybackSpeed;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Ping Delay"))
	float PingDelay;
};

/** Please add a struct description */
USTRUCT(BlueprintType)
struct FWorldMarkerInfo
{
	GENERATED_BODY()
public:
	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Marker Icon"))
	TObjectPtr<UObject> MarkerIcon;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Marker Scale", MakeStructureDefaultValue="1.000000"))
	float MarkerScale;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Marker Icon Color", MakeStructureDefaultValue="(R=0.000000,G=0.000000,B=0.000000,A=0.000000)"))
	FLinearColor MarkerIconColor;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Use Marker Arrow"))
	bool UseMarkerArrow;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Marker Name"))
	FText MarkerName;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Marker Name Font Info"))
	FTextInfo MarkerNameFontInfo;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Show Distance"))
	bool ShowDistance;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Distance Only on Edges"))
	bool DistanceOnlyOnEdges;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Distance Info"))
	FDistanceInfo DistanceInfo;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Distance Text Font Info"))
	FTextInfo DistanceTextFontInfo;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Marker Visibility Radius"))
	int32 MarkerVisibilityRadius;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Marker Visibility Smooth Transition"))
	bool MarkerVisibilitySmoothTransition;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Ping Marker"))
	bool PingMarker;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Screen Persistance", MakeStructureDefaultValue="True"))
	bool ScreenPersistance;
};


UCLASS()
class RPGSYSTEM_API UWorldMarkerWidget : public UUserWidget
{
	GENERATED_BODY()
	protected:
    // =================================================================
    // 1. 위젯 바인딩 (BindWidget)
    // 이름이 정확히 일치해야 합니다. 타입이 다를 경우 변경해주세요.
    // =================================================================

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> ArrowIcon;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UBorder> B_MarkerPing;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> B_MarkerShine;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> DistanceText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> Img_MarkerPing1;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> Img_MarkerPing2;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> MarkerIcon;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> MarkerNameDistanceText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> MarkerNameText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> MarkerShine;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UOverlay> OV_Arrow;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> OV_ArrowIcon;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UOverlay> OV_ArrowRot;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UOverlay> OV_Base;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> OV_MarkerText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UOverlay> OV_Texts;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<USizeBox> SB_Arrow; 

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<USizeBox> SB_Content;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<USizeBox> SB_Marker;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<USizeBox> SB_Ping;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UVerticalBox> VB_MainText;


public:
    // =================================================================
    // 2. 변수 (Variables)
    // 이미지에 보이는 타입과 속성을 그대로 매핑했습니다.
    // =================================================================

    // '지오메트리'는 보통 캐싱용이나 계산용으로 쓰이므로 BlueprintReadOnly로 설정하는 경우가 많습니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    FGeometry Geometry;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float MarkerScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    FVector2D IconSizeLocal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    bool bUseMarkerArrow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    FWorldMarkerInfo MarkerInfo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float TextInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float EdgeToleranceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float ScreenCenterTolerance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    bool bCanShowScreenCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    int32 HideMarkerAtDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    bool HideMarkerSmoothly;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    bool bUseDistanceVisibility;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    bool bPingMarker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    bool bIsVisible;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float PingPlaybackSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float PingDelay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    bool bScreenPersistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    FVector2D DesktopResolution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float InterpValue;
};
