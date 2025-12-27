// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Navigation/Minimap/MinimapDistantMarkerWidget.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Animation/WidgetAnimation.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"
#include "UI/UIUtilityLibrary.h"

UMinimapDistantMarkerWidget::UMinimapDistantMarkerWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	IsActive = true;
	IsVisible = false;
}

void UMinimapDistantMarkerWidget::UpdateDistantMarker(bool _IsMarkerInMapRadius, float _ZoomValue, float _ZoomDivider)
{
	if(IsActive)
	{
		IsMarkerInMapRadius = _IsMarkerInMapRadius;
		ZoomValue = _ZoomValue;
		ZoomDivider = _ZoomDivider;

		if(IsMarkerInMapRadius)
		{
			HideMarker();
		}
		else
		{
			ShowMarker();
		}
		
		// 추가적인 거리 업데이트 및 표시 로직이 여기에 들어갈 수 있습니다.
		// 예: SetDistanceText(FindMeters());
	}
}

void UMinimapDistantMarkerWidget::ShowMarker()
{
	if(!IsVisible)
	{
		IsVisible = true;
		AnimFade(true, 12.f);
	}
}

void UMinimapDistantMarkerWidget::HideMarker()
{
	if(IsVisible)
	{
		IsVisible = false;
		AnimFade(false, 12.f);
	}
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
	if(DistanceValue)
	{
		FString ResultStr, DummyStr;
		UKismetStringLibrary::Split(DistanceValue->GetText().ToString(), TEXT(" "), ResultStr, DummyStr);
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
	if(DistanceValue)
	{
		DistanceValue->SetText(FText::FromString(InString));
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

void UMinimapDistantMarkerWidget::HandleRenderOpacity(float InterpSpeedIcon, float InterpSpeedText)
{
	// 투명도 보간 로직
}

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