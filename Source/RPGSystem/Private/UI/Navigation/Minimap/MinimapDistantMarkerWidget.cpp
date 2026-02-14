// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Navigation/Minimap/MinimapDistantMarkerWidget.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Animation/WidgetAnimation.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"
#include "UI/UIUtilityLibrary.h"

UMinimapDistantMarkerWidget::UMinimapDistantMarkerWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	IsActive = true;
	bIsVisible = false;
}

void UMinimapDistantMarkerWidget::UpdateDistantMarker(bool _IsMarkerInMapRadius, float _ZoomValue, float _ZoomDivider)
{
	if(IsActive)
	{
		bIsMarkerInMapRadius = _IsMarkerInMapRadius;
		ZoomValue = _ZoomValue;
		ZoomDivider = _ZoomDivider;

		if(bIsMarkerInMapRadius)
		{
			HideMarker();
		}
		else
		{
			ShowMarker();
		}
		
	}
}

void UMinimapDistantMarkerWidget::ShowMarker()
{
	if(!bIsVisible)
	{
		bIsVisible = true;
		AnimFade(true, 12.f);
	}
}

void UMinimapDistantMarkerWidget::HideMarker()
{
	if(bIsVisible)
	{
		bIsVisible = false;
		AnimFade(false, 12.f);
	}
}

void UMinimapDistantMarkerWidget::HandleRenderOpacity(float InterpSpeedIcon, float InterpSpeedText)
{
	UUIUtilityLibrary::InterpRenderOpacity(GetWorld(),B_MarkerVisibility,bIsMarkerInMapRadius ? 0.f : 1.f, InterpSpeedIcon,false);
	
	if (bShowMarkerDistance)
	{
		// 1. 텍스트 표시 반경 안에 들어왔는지 체크 (0.0 or 1.0)
		float DistanceCheckVal = (GetPlayerDistanceToMarkerInM() <= TextVisibilityRadius) ? 1.f : 0.f;
		// 2. 반경 설정이 0(무제한)이면 무조건 보이고, 아니면 거리 체크 값 사용
		float BaseDistOpacity = (TextVisibilityRadius == 0) ? 1.f : DistanceCheckVal;
		// 3. 마커가 미니맵 반경 내(가까움)에 있다면 텍스트 숨김(0), 아니면 위에서 구한 값 사용
		float TargetOpacity = bIsMarkerInMapRadius ? 0.f : BaseDistOpacity;

		UUIUtilityLibrary::InterpRenderOpacity(GetWorld(), OV_DistanceValue, TargetOpacity, InterpSpeedText, false);
	}
}

void UMinimapDistantMarkerWidget::HandleRotation()
{
	// 1. 플레이어에서 마커를 바라보는 절대 각도 (World Yaw)
	FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(GetPlayerLocation(), GetMarkerLocation());
	float TargetYaw = LookAtRot.Yaw;

	// 2. 최종적으로 UI에 적용할 각도 계산
	float FinalYaw = 0.f;

	if (bRotateMinimapWithPlayer)
	{
		// [회전 미니맵] : 플레이어가 도는 만큼 마커 위치도 보정해야 함 (상대 각도)
		// 공식: (목표 각도 - 내 카메라 각도)
		FRotator CameraRot = UGameplayStatics::GetPlayerCameraManager(this, 0)->GetCameraRotation();
        
		// NormalizeAxis: -180 ~ 180도로 깔끔하게 정리해줌
		FinalYaw = FRotator::NormalizeAxis(TargetYaw - CameraRot.Yaw);
	}
	else
	{
		// [고정 미니맵] : 절대 각도 그대로 사용 (북쪽이 위)
		FinalYaw = TargetYaw;
	}

	// 3. 화살표(Base) 회전 적용
	// (참고: UI 좌표계나 이미지 원본 방향에 따라 -1을 곱하거나 90도를 더해야 할 수도 있음)
	OV_BaseRot->SetRenderTransformAngle(FinalYaw);

	// 4. 아이콘 역회전 (아이콘이 같이 돌지 않고 항상 똑바로 서있게 함)
	// 부모가 FinalYaw만큼 돌았으니, 자식은 -FinalYaw만큼 돌려주면 0이 됨 (혹은 상쇄됨)
	float CounterRotation = -FinalYaw;
    
	Img_MarkerIcon->SetRenderTransformAngle(CounterRotation);
	Txt_DistanceValue->SetRenderTransformAngle(CounterRotation);
}

void UMinimapDistantMarkerWidget::HandleDistanceVisibility(float InterpSpeed)
{
	if (!bIsMarkerInMapRadius)
	{
		MapBorderDistance = GetPlayerDistanceToMarkerInM();
	}
    
	// 2. 분기 처리 (Branch)
	// 조건: 숨김 거리가 0이 아닌가? (True = 거리 제한 있음 / False = 거리 제한 없음)
	if (HideMarkerAtDistance != 0)
	{
		// [True 경로] 거리 계산 로직 실행
		float MeterDist = GetPlayerDistanceToMarkerInM();
		float TargetValue = 1.0f;
       
		// bHideMarkerSmoothly 변수를 사용하여 부드럽게 숨길지 결정한다고 가정
		if (bHideMarkerSmoothly)
		{
			// MapRangeClamped: MapBorderDistance ~ HideMarkerAtDistance 사이에서 1.0 -> 0.0 으로 변화
			TargetValue = UKismetMathLibrary::MapRangeClamped(MeterDist, MapBorderDistance, (float)HideMarkerAtDistance, 1.f, 0.f);
		}
		else
		{
			// 단순 비교: 설정 거리보다 멀면 0(숨김), 가까우면 1(보임)
			TargetValue = (MeterDist > HideMarkerAtDistance) ? 0.f : 1.f;
		}
       
		// 계산된 목표값으로 투명도 보간
		UUIUtilityLibrary::InterpRenderOpacity(this, OV_Base, TargetValue, InterpSpeed, false);
	}
	else
	{
		// [False 경로] HideMarkerAtDistance가 0일 때
		// 거리 제한 기능을 껐다는 의미이므로, 마커를 항상 보이게(1.0) 복구합니다.
       
		// 만약 0이 되자마자 부드럽게 나타나게 하려면:
		UUIUtilityLibrary::InterpRenderOpacity(this, OV_Base, 1.0f, InterpSpeed, false);

		// (옵션) 만약 즉시 나타나게 하려면 아래 주석을 사용하세요.
		// OV_Base->SetRenderOpacity(1.0f);
	}
}

void UMinimapDistantMarkerWidget::HandleMarkerData(FLinearColor InMarkerColor, UObject* InIcon, FDistanceInfo InDistanceInfo)
{
	// Set Variable
	MarkerColor = InMarkerColor;
	Icon = InIcon;
	DistanceInfo = InDistanceInfo;
	
	// HandleDistanceInfo
	bShowMarkerDistance = DistantMarkerInfo.ShowDistanceText;
	if (bShowMarkerDistance)
	{
		if (DistanceInfo.UseManualDistanceInfo)
		{
			DistanceUnit = DistanceInfo.DistanceUnit;
			DistanceStepsKM = DistanceInfo.DistanceSteps_km;
			DistanceStepsM = DistanceInfo.DistanceSteps_m;
			DistanceStepsMiles = DistanceInfo.DistanceSteps_mi;
			DistanceStepsYards = DistanceInfo.DistanceSteps_ya;
		}
		else
		{
			// DistanceUnit = DistanceInfo.DistanceUnit;
			// DistanceStepsKM = DistanceInfo.DistanceSteps_km;
			// DistanceStepsM = DistanceInfo.DistanceSteps_m;
			// DistanceStepsMiles = DistanceInfo.DistanceSteps_mi;
			// DistanceStepsYards = DistanceInfo.DistanceSteps_ya;
		}
	}
	
	// Font Info
	// Txt_DistanceValue->SetFont();
	// Txt_DistanceValue->SetColorAndOpacity();
	
	B_MarkerVisibility->SetRenderOpacity(0.f);
	OV_DistanceValue->SetRenderOpacity(0.f);
	// Marker Color
	
	// Marker Icon
	
	// Marker Scale
	
	// Handle Marker Use Pointer
	
	// Set Text Visibility Radius
	
	// Set Hide Marker Radius
	
	// Handle "Rotate Minimap With Player" Angles
}

void UMinimapDistantMarkerWidget::HandleSetFontInfo()
{
}

void UMinimapDistantMarkerWidget::HandleDistanceText()
{
}

void UMinimapDistantMarkerWidget::HandleZoomScale()
{
}

void UMinimapDistantMarkerWidget::HandleShowHideMarker()
{
}

void UMinimapDistantMarkerWidget::PlayWidgetAnimation(UWidgetAnimation* WidgetAnim, bool FadeIn, bool Loop, float PlaybackSpeed)
{
	int32 NumLoopsToPlay = Loop ? 0 : 1;
	EUMGSequencePlayMode::Type PlayMode = FadeIn ? EUMGSequencePlayMode::Forward : EUMGSequencePlayMode::Reverse;

	PlayAnimation(WidgetAnim, 0.f, NumLoopsToPlay, PlayMode, PlaybackSpeed);
}

void UMinimapDistantMarkerWidget::AnimFade(bool FadeIn, float PlaybackSpeed)
{
	if(nullptr != Fade) // 헤더의 Fade 위젯 애니메이션 바인딩 사용
	{
		PlayWidgetAnimation(Fade, FadeIn, false, PlaybackSpeed);
	}	
}

int32 UMinimapDistantMarkerWidget::GetYardDistanceToMarker()
{
	float Length = GetPlayerDistanceToMarker();
	if(Length <= 0.f)
	{
		return 0;
	}
	// 언리얼 단위(cm) -> Yard 변환 (100cm = 1m, 1m = 1.09361 yard)
	return FMath::TruncToInt32((Length / 100.f) * 1.09361f);
}

int32 UMinimapDistantMarkerWidget::GetMeterDistanceToMarker()
{
	int32 TruncValue = FMath::TruncToInt32(GetPlayerDistanceToMarker());
	return TruncValue / 100;
}

FString UMinimapDistantMarkerWidget::GetDistanceValueFromText()
{
	if(Txt_DistanceValue)
	{
		FString ResultStr, DummyStr;
		UKismetStringLibrary::Split(Txt_DistanceValue->GetText().ToString(), TEXT(" "), ResultStr, DummyStr);
		return ResultStr;
	}
	return FString();
}

bool UMinimapDistantMarkerWidget::IsInStepRange(int32 Value, int32 Steps)
{
	return (Value % Steps) == 0;
}

// ... 기타 단위 변환 함수 구현 (필요에 따라 Source의 로직을 채워넣거나 단순 반환 처리) ...
FString UMinimapDistantMarkerWidget::FindMeters()
{
	return FString::FromInt(GetMeterDistanceToMarker()) + TEXT(" m");
}
FString UMinimapDistantMarkerWidget::FindYards() { return FString(); } // 구현 생략
FString UMinimapDistantMarkerWidget::GetDigitFromInteger(int32 Integer, int32 Index) { return FString(); }
FString UMinimapDistantMarkerWidget::FindUnder10MiInt() { return FString(); }
FString UMinimapDistantMarkerWidget::FindOver10MiInt() { return FString(); }
FString UMinimapDistantMarkerWidget::FindUnder10KmInt() { return FString(); }
FString UMinimapDistantMarkerWidget::FindOver10KmInt() { return FString(); }
FString UMinimapDistantMarkerWidget::FindKilometers() { return FString(); }
FString UMinimapDistantMarkerWidget::FindMiles() { return FString(); }

void UMinimapDistantMarkerWidget::SetDistanceText(FString InString)
{
	if(Txt_DistanceValue)
	{
		Txt_DistanceValue->SetText(FText::FromString(InString));
	}
}

void UMinimapDistantMarkerWidget::InterpolateRenderOpacity(UUserWidget* Input, float Target, float InterpSpeed)
{
	if(Input)
	{
		float Current = Input->GetRenderOpacity();
		float Next = FMath::FInterpTo(Current, Target, GetWorld()->GetDeltaSeconds(), InterpSpeed);
		Input->SetRenderOpacity(Next);
	}
}

APawn* UMinimapDistantMarkerWidget::GetPlayerPawn() const
{
	return GetOwningPlayer()->GetPawn();
}

FLinearColor UMinimapDistantMarkerWidget::FindDistantMarkerColor() { return MarkerColor; }
UObject* UMinimapDistantMarkerWidget::FindDistantMarkerIcon() { return Icon; }
int32 UMinimapDistantMarkerWidget::FindMarkerTypeIndex() { return 0; }
FVector2D UMinimapDistantMarkerWidget::FindZoomScaleValue() { return FVector2D(1.f, 1.f); }
FLinearColor UMinimapDistantMarkerWidget::FindColor(FLinearColor InColor, FLinearColor StylesheetColor) { return InColor; }
FTextInfo UMinimapDistantMarkerWidget::GetDistanceTextInfo() { return FTextInfo(); }
FLinearColor UMinimapDistantMarkerWidget::FindDistanceTextColor(FLinearColor StylesheetColor) { return StylesheetColor; }


FVector UMinimapDistantMarkerWidget::GetMarkerLocation() const
{
	if(AActor* MarkerActor = Cast<AActor>(MarkerObject))
	{
		return MarkerActor->GetActorLocation();
	}
	else if(USceneComponent* SceneComp = Cast<USceneComponent>(MarkerObject))
	{
		return SceneComp->GetComponentLocation();
	}
	return FVector::ZeroVector;
}

FVector UMinimapDistantMarkerWidget::GetPlayerLocation() const
{
	if(APawn* Pawn = GetPlayerPawn())
	{
		return Pawn->GetActorLocation();
	}
	return FVector::ZeroVector;
}

float UMinimapDistantMarkerWidget::GetPlayerDistanceToMarker() const
{
	return FVector::Dist(GetPlayerLocation(), GetMarkerLocation());
}

int32 UMinimapDistantMarkerWidget::GetPlayerDistanceToMarkerInM()
{
	return GetMeterDistanceToMarker();
}