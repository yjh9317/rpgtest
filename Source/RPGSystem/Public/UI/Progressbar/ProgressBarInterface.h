// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ProgressBarInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UProgressBarInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RPGSYSTEM_API IProgressBarInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// =================================================================
	// 1. Progress Bar General (기본 설정)
	// =================================================================

	/** 전체 크기 설정 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|General")
	void PB_SetSize(FVector2D InSize);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|General")
	FVector2D PB_GetSize() const;

	/** 현재 퍼센트 설정 (0.0 ~ 1.0) */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|General")
	void PB_SetPercent(float InPercent);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|General")
	float PB_GetPercent() const;

	/** 목표 퍼센트 설정 (보간 이동용) */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|General")
	void PB_SetTargetPercent(float InTargetPercent);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|General")
	float PB_GetTargetPercent() const;

	/** 목표 퍼센트 사용 여부 설정 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|General")
	void PB_SetUseTargetPercent(bool bEnable);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|General")
	bool PB_GetUseTargetPercent() const;

	/** 진행 방식 설정 (Fill, Scale 등) - Enum을 uint8로 대체 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|General")
	void PB_SetProgressMethod(uint8 InMethod);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|General")
	uint8 PB_GetProgressMethod() const;

	/** 현재 값 보간 시간 설정 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|General")
	void PB_SetInterpTimeCurrent(float InTime);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|General")
	float PB_GetInterpTimeCurrent() const;

	/** 목표 값 보간 시간 설정 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|General")
	void PB_SetInterpTimeTarget(float InTime);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|General")
	float PB_GetInterpTimeTarget() const;

	/** 블렌드 마스크 설정 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|General")
	void PB_SetBlendMask(UTexture2D* InMask);

	/** 채우기 타입 설정 (Left to Right, Radial 등) */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|General")
	void PB_SetFillType(uint8 InType);

	/** 중앙 채우기 간격 설정 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|General")
	void PB_SetFillFromCenterSpacing(float InSpacing);


	// =================================================================
	// 2. Progress Bar Fill Layer (채우기 레이어)
	// =================================================================

	/** 채우기 색상 설정 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Fill Layer")
	void PB_SetFillColor(FLinearColor InColor);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Fill Layer")
	FLinearColor PB_GetFillColor() const;

	/** 그라데이션 사용 여부 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Fill Layer")
	void PB_SetUseGradientFillColor(bool bEnable);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Fill Layer")
	bool PB_GetUseGradientFillColor() const;

	/** 그라데이션 강도 설정 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Fill Layer")
	void PB_SetFillColorGradientPower(float InPower);

	/** 브러시 타일링 설정 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Fill Layer")
	void PB_SetFillColorBrushTiling(FVector2D InTiling);

	/** 채우기 마스크 설정 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Fill Layer")
	void PB_SetFillColorMask(UTexture2D* InMask);

	/** 그라데이션 타입 설정 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Fill Layer")
	void PB_SetFillColorGradientType(uint8 InType);


	// =================================================================
	// 3. Progress Bar Target Layer (목표값 레이어 - 예상 데미지/회복 등)
	// =================================================================

	/** 긍정적 목표 색상 (회복 등) */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Target Layer")
	void PB_SetTargetFillColor_Positive(FLinearColor InColor);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Target Layer")
	FLinearColor PB_GetTargetFillColor_Positive() const;

	/** 부정적 목표 색상 (데미지 등) */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Target Layer")
	void PB_SetTargetFillColor_Negative(FLinearColor InColor);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Target Layer")
	FLinearColor PB_GetTargetFillColor_Negative() const;


	// =================================================================
	// 4. Progress Bar Background Layer (배경)
	// =================================================================

	/** 두께 설정 (원형 바 등에서 사용) */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Background")
	void PB_SetThickness(float InThickness);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Background")
	float PB_GetThickness() const;

	/** 배경 색상 설정 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Background")
	void PB_SetBackgroundColor(FLinearColor InColor);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Background")
	FLinearColor PB_GetBackgroundColor() const;

	/** 배경 마스크 설정 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Background")
	void PB_SetBackgroundColorMask(UTexture2D* InMask);

	/** 배경 타일링 설정 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Background")
	void PB_SetBackgroundBrushTiling(FVector2D InTiling);

	/** 배경 블러 강도 설정 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Background")
	void PB_SetBackgroundBlurStrength(float InStrength);


	// =================================================================
	// 5. Progress Bar Marquee (마키/흐르는 효과)
	// =================================================================

	/** 커스텀 마키 사용 여부 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Marquee")
	void PB_SetIsCustomMarquee(bool bEnable);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Marquee")
	bool PB_GetIsCustomMarquee() const;

	/** 마키 효과 사용 여부 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Marquee")
	void PB_SetIsMarquee(bool bEnable);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Marquee")
	bool PB_GetIsMarquee() const;

	/** 커스텀 마키 이미지 설정 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Marquee")
	void PB_SetCustomMarqueeImage(UTexture2D* InImage);

	/** 기본 마키 이미지 설정 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Marquee")
	void PB_SetMarqueeImage(UTexture2D* InImage);

	/** 커스텀 마키 마스크 타입 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Marquee")
	void PB_SetCustomMarqueeMaskType(uint8 InType);


	// =================================================================
	// 6. Progress Bar Separation (구분선)
	// =================================================================

	/** 구분선 단계 수 설정 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Separation")
	void PB_SetSeparationSteps(int32 InSteps);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Separation")
	int32 PB_GetSeparationSteps() const;

	/** 구분선 간격 설정 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Separation")
	void PB_SetSeparationStepsSpacing(float InSpacing);

	/** 절대 채우기 사용 여부 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Separation")
	void PB_SetSeparationAbsoluteFill(bool bEnable);


	// =================================================================
	// 7. Progress Bar Effects (이펙트)
	// =================================================================

	/** 이펙트 목록 설정 (타입이 불분명하여 UObject 배열로 가정) */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Effects")
	void PB_SetEffects(const TArray<UObject*>& InEffects);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Effects")
	TArray<UObject*> PB_GetEffects() const;

	/** 특정 이펙트 활성화/비활성화 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Effects")
	void PB_SetEffectEnabled(UObject* InEffect, bool bEnabled);

	/** 모든 이펙트 활성화/비활성화 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Effects")
	void PB_SetAllEffectsEnabled(bool bEnabled);

	/** 이펙트 추가 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Effects")
	void PB_AddEffect(UObject* InEffect);

	/** 이펙트 제거 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Progress Bar|Effects")
	void PB_RemoveEffect(UObject* InEffect);
};
