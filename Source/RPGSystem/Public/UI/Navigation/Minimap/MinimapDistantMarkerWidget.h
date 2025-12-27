// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/UIEnums.h"
#include "MinimapDistantMarkerWidget.generated.h"

class UTextBlock;
class UBorder;

UCLASS()
class RPGSYSTEM_API UMinimapDistantMarkerWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UMinimapDistantMarkerWidget(const FObjectInitializer& ObjectInitializer);
	void UpdateDistantMarker(bool _IsMarkerInMapRadius, float _ZoomValue, float _ZoomDivider);
	void ShowMarker();
	void HideMarker();
	
	bool IsFirstPerson; // 시점 정보
	bool RotateMinimapWithPlayer; // 회전 여부
	bool ScaleDistantMarkerWithZoom; // 줌에 따라 크기 변경 여부
	FVector2D DistantMarkerMinMaxScale; // 크기 제한

private:
	// [UI 컴포넌트]
	UPROPERTY(meta= (BindWidget))
	UTextBlock* DistanceValue; // 거리 텍스트 (예: "150m")
	
	UPROPERTY(meta= (BindWidget))
	UBorder* MarkerVisibility; // 가시성 제어용 보더

	// [애니메이션] 페이드 효과 등
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* Fade;

	// [거리 표시 설정]
	UPROPERTY(EditAnywhere, Category="Distance")
	TEnumAsByte<EDistanceUnit> DistanceUnit = EDistanceUnit::Miles; // 거리 단위
	UPROPERTY(EditAnywhere, Category="Distance")
	int32 MarkerDistanceYards;
	UPROPERTY(EditAnywhere, Category="Distance")
	int32 MarkerDistanceMeters;
	UPROPERTY(EditAnywhere, Category="Distance")
	bool ShowMarkerDistance = true; // 거리 표시 여부
	
	int32 DistanceStepsYa;
	int32 DistanceStepsKM;
	int32 DistanceStepsM;
	int32 DistanceStepsMi;

	// [상태 변수들]
	FDistanceInfo DistanceInfo;
	FDistantMarkerInfo DistantMarkerInfo;
	bool IsActive;
	
	UPROPERTY()
	UObject* MarkerObject; // 추적 대상
	
	FLinearColor MarkerColor;
	
	UPROPERTY()
	UObject* Icon;
	
	bool HideMarkerSmoothly;
	bool UseDistantMarkerArrow; // 화살표 모양 사용 여부
	int32 TextVisibilityRadius;
	int32 HideMarkerAtDistance; // 너무 멀어지면 아예 숨길 거리
	bool IsVisible;
	float ZoomDivider;
	float ZoomValue;
	float Tolerance;
	bool IsMarkerInMapRadius; // 맵 반경 내 진입 여부
	int32 MapBorderDistance; // 가장자리 거리 오프셋

	UPROPERTY()
	UWidgetAnimation* WidgetAnimation;

	// 애니메이션 재생 헬퍼 함수
	void PlayWidgetAnimation(UWidgetAnimation* WidgetAnim, bool FadeIn, bool Loop, float PlaybackSpeed);
	void AnimFade(bool FadeIn, float PlaybackSpeed);

private:
	// [거리 계산 및 텍스트 변환 함수들]
	UFUNCTION(BlueprintPure, Category= "Distance")
	int32 GetYardDistanceToMarker();
	UFUNCTION(BlueprintPure, Category= "Distance")
	int32 GetMeterDistanceToMarker();
	UFUNCTION(BlueprintPure, Category= "Distance")
	FString GetDistanceValueFromText();
	
	UFUNCTION(BlueprintPure, Category= "Distance")
	bool IsInStepRange(int32 Value, int32 Steps);
	
	// ... (단위 변환 함수들) ...
	UFUNCTION(BlueprintPure, Category= "Distance")
	FString FindYards();
	UFUNCTION(BlueprintPure, Category= "Distance")
	FString GetDigitFromInteger(int32 Integer, int32 Index);
	UFUNCTION(BlueprintPure, Category= "Distance")
	FString FindUnder10MiInt();
	UFUNCTION(BlueprintPure, Category= "Distance")
	FString FindOver10MiInt();
	UFUNCTION(BlueprintPure, Category= "Distance")
	FString FindMeters();
	UFUNCTION(BlueprintPure, Category= "Distance")
	FString FindUnder10KmInt();
	UFUNCTION(BlueprintPure, Category= "Distance")
	FString FindOver10KmInt();
	UFUNCTION(BlueprintPure, Category= "Distance")
	FString FindKilometers();
	UFUNCTION(BlueprintPure, Category= "Distance")
	FString FindMiles();
	UFUNCTION(BlueprintCallable, Category= "Distance")
	void SetDistanceText(FString InString);

	APawn* GetPlayerPawn() const;
	void InterpolateRenderOpacity(UUserWidget* Input, float Target, float InterpSpeed);
	UFUNCTION(BlueprintPure, Category= "Utility")
	FLinearColor FindDistantMarkerColor();
	UFUNCTION(BlueprintPure, Category= "Utility")
	UObject* FindDistantMarkerIcon();
	UFUNCTION(BlueprintPure, Category= "Utility")
	int32 FindMarkerTypeIndex();
	UFUNCTION(BlueprintPure, Category= "Utility")
	FVector2D FindZoomScaleValue();
	UFUNCTION(BlueprintPure, Category= "Utility")
	FLinearColor FindColor(FLinearColor InColor, FLinearColor StylesheetColor);
	UFUNCTION(BlueprintPure, Category= "Utility")
	FTextInfo GetDistanceTextInfo();
	UFUNCTION(BlueprintPure, Category= "Utility")
	FLinearColor FindDistanceTextColor(FLinearColor StylesheetColor);
	
	// [위치 계산]
	UFUNCTION(BlueprintPure, Category= "Location")
	FVector GetMarkerLocation() const; // 대상의 월드 위치
	
	UFUNCTION(BlueprintPure, Category= "Location")
	FVector GetPlayerLocation() const; // 플레이어 월드 위치
	
	UFUNCTION(BlueprintPure, Category= "Location")
	float GetPlayerDistanceToMarker() const; // 거리 계산

	UFUNCTION(BlueprintPure, Category= "Location")
	int32 GetPlayerDistanceToMarkerInM();
	
	void HandleRenderOpacity(float InterpSpeedIcon, float InterpSpeedText);
};