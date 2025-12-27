// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MinimapData.h"
#include "Blueprint/UserWidget.h"
#include "MinimapWidget.generated.h"

class UMinimapDistantMarkerWidget;
class UMinimapMarkerWidget;
class URetainerBox;
class UOverlay;
class UImage;
class UBorder;
class USizeBox;
/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UMinimapWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UMinimapWidget(const FObjectInitializer& ObjectInitializer);
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	
	void InitMinimapData(const FMinimapInitData& Data);
	
public:
	void ShowMinimap(FMiniMapInfo Info);
	void HideMinimap();
	void UpdateMiniMap(float NewZoomValue);
	void AddOrUpdateMarker(UObject* Marker,bool ScaleWithZoom, FMiniMapInfo Info);
	
	// 마커 제거
	void RemoveMarker(UObject* Marker);
	
	// 모든 마커 제거
	void RemoveAllMinimapMarker();

	// UFUNCTION(BlueprintImplementableEvent)
	// void OnAnimationFinishedCallback();
	
private:
	// [설정 값]
	UPROPERTY(EditDefaultsOnly ,BlueprintReadOnly, meta = (AllowPrivateAccess))
	float MapSize; // 맵 텍스처의 크기

	UPROPERTY(EditDefaultsOnly ,BlueprintReadOnly, meta = (AllowPrivateAccess))
	float ZoomInterpSpeed; // 줌 속도

	UPROPERTY(EditDefaultsOnly ,BlueprintReadOnly, meta = (AllowPrivateAccess))
	float ZoomDivider; // 줌 비율 계산용

	// 에디터에서 설정 가능한 기준 액터들
	UPROPERTY(EditDefaultsOnly ,BlueprintReadOnly, meta = (AllowPrivateAccess))
	TSubclassOf<AActor> MinimapBottomLeftActor;

	UPROPERTY(EditDefaultsOnly ,BlueprintReadOnly, meta = (AllowPrivateAccess))
	TSubclassOf<AActor> MinimapTopRightActor;

	// [상태 변수]
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess))
	bool IsActive; // 활성화 여부
	
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess))
	float CurrentZoomValue; // 현재 줌 배율

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess))
	FMiniMapInfo MiniMapInfo; // 현재 맵 정보
	
	// [좌표 캐싱] 매번 계산하지 않고 저장해둠
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess))
	FVector2D BottomLeft;
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess))
	FVector2D BLCache;
	
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess))
	FVector2D TopRight;
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess))
	FVector2D TRCache;
	
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess))
	FVector2D PointerMinMaxScale;
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess))
	FVector2D MinMaxScale;
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess))
	bool ScalePointerWithZoom;
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess))
	bool RotateMinimapWithPlayer;
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess))
	bool IsFirstPerson;
	
protected:
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* Img_Pointer;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USizeBox* SB_Content;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* Img_Map;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* Img_Frame;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UOverlay* OV_MapMarkers;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UOverlay* OV_DistantMarker;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UOverlay* OV_Map;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UOverlay* OV_PointerAnim;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UOverlay* OV_CanvasPanelMap;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBorder* B_MapMarkers;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USizeBox* SB_MapMarkers;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	URetainerBox* RB_Minimap;

	// 위젯 애니메이션 (페이드 등)
	UPROPERTY()
	TObjectPtr<UWidgetAnimation> FadeMapAnimation;

	// [마커 관리 컨테이너]
	UPROPERTY()
	TMap<UObject*,UMinimapMarkerWidget*> MarkerMap; // 일반 마커 맵

	UPROPERTY()
	TObjectPtr<UMinimapMarkerWidget> CurrentMarker; // 현재 처리 중인 마커 (임시)
	
	UPROPERTY()
	TMap<UMinimapMarkerWidget*,UMinimapDistantMarkerWidget*> DistantMarkerMap; // 원거리 마커 연결 맵

	UPROPERTY()
	TObjectPtr<UUserWidget> CurrentDistantMarker; // 현재 처리 중인 원거리 마커

	// 마커 생성을 위한 위젯 클래스 참조
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMinimapMarkerWidget> MinimapMarkerClass;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMinimapDistantMarkerWidget> MinimapDistantMarkerClass;

protected:
	// 페이드 인/아웃 효과 처리
	void HandleFadeInMinimap(float FadeInSpeedPointer = 3.f,float FadeInSpeedMap = 4.f);
	void HandleFadeOutMinimap(float FadeInSpeedPointer = 3.f,float FadeInSpeedMap = 4.f);
	
	// 맵 초기 설정 (좌표 계산 등)
	void SetupMap();
	// 마커들에게 맵 정보 전파
	void SetMinimapInfoToMarker();
	
public:	
	void SetupMinimapInfo(FMiniMapInfo& Info);
	
	/* [핵심 계산 함수들] */
	APawn* GetPlayerPawn() const; // 플레이어 폰 가져오기
	FVector2D GetPlayerLocation2D() const; // 플레이어의 월드 좌표 (X, Y)
	FVector2D GetBottomLeftLocation2D() const; // 맵 좌하단 월드 좌표
	FVector2D GetTopRightLocation2D() const; // 맵 우상단 월드 좌표
	
	// 월드 좌표를 미니맵 UI 좌표로 변환 (가장 중요한 함수)
	FVector2D FindPlayer2DLocationOnMap(FVector2D InValue) const;
	
	// 캔버스 슬롯상에서 맵 이미지의 위치/크기 제어
	FVector2D GetCanvasMapPosition() const;
	float GetCanvasMapSize() const;
	void SetCanvasMapPosition(FVector2D InPosition) const;
	void SetCanvasMapSize(float InSize) const;
	
	float GetMapTextureSize() const; // 텍스처 실제 해상도 가져오기
	
	// 줌에 따른 종횡비 및 크기 계산
	void FindZoomAspectRatio(float Value, float& AspectRatio, float& ZoomSize) const;

	// [Getter/Setter]
	UFUNCTION()
	void SetMinimapSize(float InSize);

	UFUNCTION()
	void SetPointerSize(float InSize);

	UFUNCTION()
	void SetPointerIconAndColor(UObject* InIcon, FLinearColor InColor);

	UFUNCTION()
	void SetBlendMask(UTexture2D* InTexture);

	UFUNCTION()
	void SetMinimapTint(FLinearColor InColor);

	UFUNCTION()
	float FindAngleByDirection(int32 InAngle); // 방향 각도 계산

	UFUNCTION()
	float GetPlayerPointerAngle(); // 플레이어 회전 각도 계산 (화살표 회전용)
	
	UFUNCTION()
	bool MarkerObjectInUse(UObject* InMarkerObject); // 해당 오브젝트의 마커가 이미 존재하는지 확인

	
private:
	UObject* ValidateMarkerObject(UObject* InObject); // 유효한 마커 대상인지 확인
	bool IsMarkerObjectInUse(UObject* InObject);
	UMinimapMarkerWidget* FindMarker(UObject* InObject); // 마커 찾기
	UMinimapDistantMarkerWidget* FindDistantMarker(UMinimapMarkerWidget* InMarkerWidget); // 원거리 마커 찾기
	
	// 부드러운 이동/줌을 위한 보간(Interpolation) 함수
	void InterpolateCanvasMapPosition(float InInterpSpeed,bool InInterpConstant);
	void InterpolateCanvasMapZoom(float InZoomValue, float InInterpSpeed, bool InInterpConstant);
	
	// 마커 생성 또는 찾기 로직 처리
	void HandleCreateOrFindMarker(UObject* InObject);
};
