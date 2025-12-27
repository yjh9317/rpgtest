#pragma once

#include "CoreMinimal.h"
#include "Slate/WidgetTransform.h"
#include "MinimapData.generated.h"

/**
 * 미니맵 초기 설정 정보를 담는 구조체입니다.
 * 월드 크기와 미니맵 텍스처를 매핑하기 위한 좌표 정보가 핵심입니다.
 */
USTRUCT(BlueprintType)
struct FMiniMapInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	TSubclassOf<AActor> BottomLeftActor{}; // 월드 좌하단(시작점) 위치를 잡기 위한 액터 클래스
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	FVector2D BottomLeftCoordinates{};     // 월드 좌하단 좌표 (액터가 없을 경우 직접 입력)
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	TSubclassOf<AActor> TopRightActor{};   // 월드 우상단(끝점) 위치를 잡기 위한 액터 클래스
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	FVector2D TopRightCoordinates {};      // 월드 우상단 좌표 (액터가 없을 경우 직접 입력)
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	UObject* MinimapTexture{};             // 미니맵 배경으로 쓸 텍스처 이미지
};

/**
 * 미니맵 위젯의 디자인 및 동작 옵션을 초기화하기 위한 데이터입니다.
 */
USTRUCT(BlueprintType)
struct FMinimapInitData
{
	GENERATED_BODY()

	UPROPERTY()
	float Size{}; // 미니맵 위젯의 크기

	UPROPERTY()
	FVector2D MinMaxMarkerScale{}; // 마커 크기의 최소/최대 제한

	UPROPERTY()
	float ZoomInterpSpeed{}; // 줌 변경 시 부드러운 전환 속도

	UPROPERTY()
	UObject* PointerIcon{}; // 플레이어 화살표 아이콘

	UPROPERTY()
	FLinearColor PointerColor{}; // 플레이어 화살표 색상

	UPROPERTY()
	float PointerSize{}; // 플레이어 화살표 크기

	UPROPERTY()
	bool ScalePointerWithZoom{}; // 줌 레벨에 따라 화살표 크기도 같이 변할지 여부

	UPROPERTY()
	FVector2D PointerMinMaxScale{}; // 화살표 크기의 최소/최대 제한

	UPROPERTY()
	bool RotateMinimapWithPlayer{}; // 플레이어 회전에 맞춰 맵도 회전할지 여부
	
	UPROPERTY()
	bool IsFirstPerson; // 1인칭 시점 여부 (회전 방식에 영향 줄 수 있음)

	UPROPERTY()
	FLinearColor MinimapTint{}; // 미니맵 전체 색조
	
	UPROPERTY()
	UObject* MinimapFrame{}; // 미니맵 테두리 이미지
	
	UPROPERTY()
	FLinearColor MinimapFrameTint{}; // 테두리 색상
	
	UPROPERTY()
	FVector2D MinimapFrameScale{}; // 테두리 크기 배율
};

USTRUCT(BlueprintType)
struct FMinimapMarkerInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	UObject* MarkerIcon; // 아이콘 이미지
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	FLinearColor MarkerColor; // 색상

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	float MarkerScale; // 크기
};