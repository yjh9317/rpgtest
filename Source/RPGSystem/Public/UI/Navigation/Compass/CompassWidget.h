// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CompassData.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "CompassWidget.generated.h"


class USizeBox;
class UCompassMarkerWidget;
class UOverlay;
class URetainerBox;
class UImage;
/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UCompassWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativePreConstruct() override; 
	virtual void NativeConstruct() override;    
	virtual bool Initialize() override;         
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	void InitCompassData(const FCompassInitData& Data);
	// [핵심 로직] 플레이어의 회전값(Yaw)을 0~1 사이(혹은 -1~1)의 정규화된 값으로 변환합니다.
	// 이 값을 머티리얼에 전달해 나침반 띠 이미지를 좌우로 스크롤(UV Offset)시킵니다.
	// UFUNCTION()
	// float GetPlayerRotationNormalized(ERotationType _Type);
    
	// [마커 등록/갱신] 퀘스트 목표, 적, 웨이포인트 등을 나침반에 표시해달라고 요청하는 함수입니다.
	// 이미 있는 마커라면 위치를 갱신하고, 없다면 새로 만듭니다.
	void AddOrUpdateCompassMarker(UObject* MarkerObject, UObject* PingIcon1, UObject* PingIcon2);

protected:
	bool IsActive;      // 나침반이 현재 활성화되어 화면에 보이는지 여부
	float Orientation;  // 나침반의 기준 방향 (플레이어의 정면)
	float RotationValue;// 계산된 현재 회전 값

	// [마커 저장소] 
	// 관리 중인 모든 마커를 저장합니다. (Key: 대상 액터/오브젝트, Value: 생성된 마커 위젯)
	UPROPERTY()
	TMap<UObject*,UCompassMarkerWidget*> MarkerMap;

private:
	// 실제 방위(N, S, E, W)가 그려진 이미지. 머티리얼을 통해 UV를 이동시킵니다.
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UImage* Image_Compass;

	// 나침반 배경 이미지
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UImage* Image_CompassBackground;

	// 방위 눈금이나 방향선이 그려진 이미지
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UImage* Image_DirectionLines;
	
	// 나침반의 광택/반사 효과 이미지 (장식용)
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UImage* Image_Shine;
	
	// 부드러운 광택 효과 이미지 (장식용)
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UImage* Image_SoftShine;

	// 조명 효과 이미지 (장식용)
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UImage* Image_Lighting;

	// 나침반 양옆을 흐리게(Masking) 처리하여 둥글게 보이거나 끝을 부드럽게 만드는 리테이너 박스
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess))
	URetainerBox* RetainerBox_Compass;

	// 중앙 화살표(포인터)의 크기를 제어하는 박스
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess))
	USizeBox* SizeBox_Pointer;

	// 중앙 화살표(플레이어 위치 표시) 이미지
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UImage* Image_Pointer;

	// 마커 위젯들이 실제로 생성되어 자식으로 들어가는 부모 패널 (Overlay)
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UOverlay* Overlay_MarkerParent;
	
public:
	FORCEINLINE UMaterialInstanceDynamic* GetCompassMaterial() const { return Image_DirectionLines->GetDynamicMaterial(); }

	// 회전값(RotationNormalized)을 머티리얼에 전달하여 이미지를 이동(Offset)시키는 핵심 함수
	void SetCompassOffset(float RotationNormalized, bool InvertCompass, float Orientation);
	
	// 리테이너 박스의 마스크 텍스처 설정
	void SetBlendMask(UTexture2D* InTexture);
	
	// 나침반 자체의 마스크 텍스처 설정
	void SetCompassMask(UTexture2D* InTexture);
	
	// 배경색 변경
	void SetBackgroundColor(FColor InColor);
	
	// 배경 투명도 변경
	void SetBackgroundOpacity(float InOpacity);
	
	// 나침반 눈금/장식들의 색조(Tint) 변경
	void SetCompassTint(FLinearColor InColorAndOpacity);
	
	// 중앙 포인터 크기 설정
	void SetPointerScale(float SizeScale);
	
	// 중앙 포인터 위치 이동 (미세 조정)
	void SetPointerTranslation(FVector2D InTranslation);
	
	// 중앙 포인터 색상 변경
	void SetPointerTint(FLinearColor InPointerTint);
	
	// 마커들의 전체 위치 이동 (Y축 높이 조절 등)
	void SetMarkerTranslation(FVector2D InTranslation);
	
	// 배경 텍스처 교체
	void SetBackgroundTexture(UObject* InBrush);
	
	// 배경의 변환(크기, 위치) 설정
	void SetBackgroundTransform(FWidgetTransform InTransform);
	
	// 모든 마커의 업데이트 허용 여부를 일괄 설정 (최적화용)
	void SetAllMarkersCanUpdate(bool InCanUpdate);
};