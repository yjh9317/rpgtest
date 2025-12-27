// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MinimapData.h"
#include "Blueprint/UserWidget.h"
#include "MinimapMarkerWidget.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UMinimapMarkerWidget : public UUserWidget
{
	GENERATED_BODY()
public:

	// (주석 처리됨) 추후 사용될 수 있는 멤버들
	// UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	// UObject* MarkerObject;
	//
	// UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	// UMinimapWidget* MinimapWidget;

	// [크기 및 정보] 부모 위젯에서 계산된 값을 받아 렌더링에 사용합니다.
	float TextureMapSize;   // 원본 맵 텍스처 크기
	float MinimapSize;      // 미니맵 위젯 크기
	FVector2D MinMaxScale;  // 마커 최소/최대 크기
	float ZoomDivide;       // 줌 배율에 따른 크기 보정값
	FMiniMapInfo MiniMapInfo; // 전체 맵 정보 참조
};
