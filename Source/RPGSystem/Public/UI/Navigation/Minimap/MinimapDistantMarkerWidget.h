// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/UIEnums.h" // FDistanceInfo, FDistantMarkerInfo, FTextInfo 등의 정의가 포함되어 있다고 가정
#include "MinimapDistantMarkerWidget.generated.h"

// 전방 선언 (컴파일 속도 향상)
class UWidgetSwitcher;
class USpacer;
class USizeBox;
class UOverlay;
class UImage;
class UTextBlock;
class UBorder;
class UWidgetAnimation;

UCLASS()
class RPGSYSTEM_API UMinimapDistantMarkerWidget : public UUserWidget
{
    GENERATED_BODY()
    
public:
    UMinimapDistantMarkerWidget(const FObjectInitializer& ObjectInitializer);

    // virtual void NativeConstruct() override;
    // virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Minimap")
    void UpdateDistantMarker(bool bInMarkerInMapRadius, float InZoomValue, float InZoomDivider);

    UFUNCTION(BlueprintCallable, Category = "Minimap")
    void ShowMarker();

    UFUNCTION(BlueprintCallable, Category = "Minimap")
    void HideMarker();

#pragma region Configuration
public:
    // [Exposed] 외부에서 주입받거나 설정하는 변수들
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exposed")
    bool bIsFirstPerson;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exposed")
    bool bRotateMinimapWithPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exposed")
    bool bScaleDistantMarkerWithZoom;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exposed")
    FVector2D DistantMarkerMinMaxScale;

    // [Settings] 내부 동작 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bIsVisible; // blsVisible 중복 제거 및 표준화

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bHideMarkerSmoothly;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bUseDistantMarkerArrow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bIsMarkerInMapRadius; // blsMarkerInMapRadius 중복 제거 및 표준화

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 TextVisibilityRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 HideMarkerAtDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float ZoomDivider;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float ZoomValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float Tolerance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 MapBorderDistance;
#pragma endregion

    // =================================================================
    // 런타임 데이터 (Data & State)
    // =================================================================
#pragma region RuntimeData
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
    bool IsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
    UObject* MarkerObject;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
    UObject* Icon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
    FLinearColor MarkerColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
    FDistanceInfo DistanceInfo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
    FDistantMarkerInfo DistantMarkerInfo;

    // Distance Related State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance")
    TEnumAsByte<EDistanceUnit> DistanceUnit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance")
    bool bShowMarkerDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance")
    bool IsConstructing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance")
    int32 MarkerDistanceYards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance")
    int32 MarkerDistanceMeters;

    // Unit Conversion Steps
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance|Steps")
    int32 DistanceStepsYards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance|Steps")
    int32 DistanceStepsKM;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance|Steps")
    int32 DistanceStepsM;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance|Steps")
    int32 DistanceStepsMiles;
#pragma endregion

    // =================================================================
    // UI 컴포넌트 바인딩 (Components)
    // =================================================================
#pragma region Components
protected:
    UPROPERTY(Transient, meta = (BindWidgetAnim))
    TObjectPtr<UWidgetAnimation> Fade;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UImage> Img_Arrow;
    
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UBorder> B_MarkerVisibility;
    
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UTextBlock> Txt_DistanceValue;
    
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UImage> Img_MarkerIcon;
    
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UOverlay> OV_Base;
    
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UOverlay> OV_BaseRot;
    
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UOverlay> OV_DistanceValue;
    
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UOverlay> OV_DistanceZoomScale;
    
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UOverlay> OV_DistantMarker;
    
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UOverlay> OV_MarkerZoomScale;
    
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UOverlay> OV_Rot;
    
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<USizeBox> SB_DistantMarker;
    
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<USizeBox> SB_DistantMarkerSize;
    
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<USizeBox> SB_Marker;
    
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UImage> Img_Shine;
    
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UWidgetSwitcher> WS_Type;
#pragma endregion

    // =================================================================
    // 로직 핸들러 (Logic Handlers)
    // =================================================================
#pragma region Logic Handlers
public:
    UFUNCTION(BlueprintCallable, Category = "Logic")
    void HandleRotation();

    UFUNCTION(BlueprintCallable, Category = "Logic")
    void HandleDistanceVisibility(float InterpSpeed);

    UFUNCTION(BlueprintCallable, Category = "Logic")
    void HandleMarkerData(FLinearColor InMarkerColor, UObject* InIcon, FDistanceInfo InDistanceInfo);

    UFUNCTION(BlueprintCallable, Category = "Logic")
    void HandleSetFontInfo();

    UFUNCTION(BlueprintCallable, Category = "Logic")
    void HandleDistanceText();

    UFUNCTION(BlueprintCallable, Category = "Logic")
    void HandleZoomScale();

    UFUNCTION(BlueprintCallable, Category = "Logic")
    void HandleRenderOpacity(float InterpSpeedIcon, float InterpSpeedText);

    UFUNCTION(BlueprintCallable, Category = "Logic")
    void HandleShowHideMarker();

protected:
    // 애니메이션 헬퍼
    void PlayWidgetAnimation(UWidgetAnimation* WidgetAnim, bool bFadeIn, bool bLoop, float PlaybackSpeed);
    void AnimFade(bool bFadeIn, float PlaybackSpeed);
#pragma endregion

    // =================================================================
    // 내부 유틸리티 및 계산 (Private Helpers)
    // =================================================================
#pragma region Private Helpers
private:
    // --- 거리 및 텍스트 ---
    UFUNCTION(BlueprintPure, Category = "Distance")
    int32 GetYardDistanceToMarker();

    UFUNCTION(BlueprintPure, Category = "Distance")
    int32 GetMeterDistanceToMarker();

    UFUNCTION(BlueprintPure, Category = "Distance")
    FString GetDistanceValueFromText();
    
    UFUNCTION(BlueprintPure, Category = "Distance")
    bool IsInStepRange(int32 Value, int32 Steps);
    
    UFUNCTION(BlueprintCallable, Category = "Distance")
    void SetDistanceText(FString InString);

    // 단위 변환 포맷팅
    FString FindYards();
    FString FindMeters();
    FString FindKilometers();
    FString FindMiles();
    FString GetDigitFromInteger(int32 Integer, int32 Index);
    FString FindUnder10MiInt();
    FString FindOver10MiInt();
    FString FindUnder10KmInt();
    FString FindOver10KmInt();

    // --- 유틸리티 ---
    APawn* GetPlayerPawn() const;
    void InterpolateRenderOpacity(UUserWidget* InputWidget, float TargetOpacity, float InterpSpeed);

    UFUNCTION(BlueprintPure, Category = "Utility")
    FLinearColor FindDistantMarkerColor();

    UFUNCTION(BlueprintPure, Category = "Utility")
    UObject* FindDistantMarkerIcon();

    UFUNCTION(BlueprintPure, Category = "Utility")
    int32 FindMarkerTypeIndex();

    UFUNCTION(BlueprintPure, Category = "Utility")
    FVector2D FindZoomScaleValue();

    UFUNCTION(BlueprintPure, Category = "Utility")
    FLinearColor FindColor(FLinearColor InColor, FLinearColor StylesheetColor);

    UFUNCTION(BlueprintPure, Category = "Utility")
    FTextInfo GetDistanceTextInfo();

    UFUNCTION(BlueprintPure, Category = "Utility")
    FLinearColor FindDistanceTextColor(FLinearColor StylesheetColor);
    
    // --- 위치 계산 ---
    UFUNCTION(BlueprintPure, Category = "Location")
    FVector GetMarkerLocation() const;

    UFUNCTION(BlueprintPure, Category = "Location")
    FVector GetPlayerLocation() const;

    UFUNCTION(BlueprintPure, Category = "Location")
    float GetPlayerDistanceToMarker() const;

    UFUNCTION(BlueprintPure, Category = "Location")
    int32 GetPlayerDistanceToMarkerInM();
#pragma endregion
};