// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CompassData.h"
#include "Blueprint/UserWidget.h"
#include "CompassMarkerWidget.generated.h"

class UCompassWidget;
/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UCompassMarkerWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 추적 대상 오브젝트 (적, 퀘스트 지점 등)
	UPROPERTY()
	TObjectPtr<UObject> MarkerObject;

	// 마커 위치 계산의 정밀도 보정값
	UPROPERTY()
	float MarkerPrecision = 1.f;

	// 위젯의 지오메트리 정보 (크기, 위치 계산용)
	UPROPERTY()
	FGeometry Geometry;

	// 플레이어가 현재 이 대상을 정면으로 바라보고 있는지 여부
	UPROPERTY()
	bool IsFacingToObject = false;

	// 핑(Ping) 효과 활성화 여부 (깜빡임 등)
	UPROPERTY()
	bool PingMarker = false;

	// 화면 중앙에서 벗어났을 때 무작위 변위값 (겹침 방지용으로 추정)
	UPROPERTY()
	FVector2D RandomSideTranslation;

	// 마커 밑에 거리 텍스트를 표시할지 여부
	UPROPERTY()
	bool ShowMarkerDistance;

	// 거리가 유효한 범위 내에 있어 표시 중인지 여부
	UPROPERTY()
	bool DistanceActive = false;

	// 마커 업데이트(Tick 로직) 수행 여부
	UPROPERTY()
	bool CanUpdate = false;

	// 이 반경(m) 안에 들어와야 마커가 보임
	UPROPERTY()
	int32 MarkerVisibilityRadius = false;;

	// 가시성 변경 시 부드럽게 페이드할지 여부
	UPROPERTY()
	bool MarkerVisibilitySmoothTransition = false;

	// 거리 텍스트 가시성 변경 속도
	UPROPERTY()
	float DistanceVisibilityInterpSpeed = 5.f;

	// 부드러운 가시성 전환을 위한 나눗셈 계수
	UPROPERTY()
	float SmoothVisibilityDivider = 1.5f;

	// 거리 기반 가시성 제어 기능을 사용할지 여부
	UPROPERTY()
	bool UseDistanceVisibility = false;

	// 마커의 디자인 정보 (아이콘, 색상 등)
	UPROPERTY()
	FCompassMarkerInfo MarkerInfo;

	// 부모 위젯(나침반 메인) 참조
	UPROPERTY()
	TObjectPtr<UCompassWidget> Parent;

	// 화면 밖으로 나가도 계속 표시할지 여부
	UPROPERTY()
	bool ScreenPersistance = false;

	// 핑 효과 재생 속도
	UPROPERTY()
	float PingPlaybackSpeed = 3.f;

	// 핑 효과 지속 시간 또는 지연 시간
	UPROPERTY()
	float PingDelay = 2.f;
};