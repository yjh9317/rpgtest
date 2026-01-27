// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Navigation/Minimap/MinimapWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Border.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/RetainerBox.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "UI/UIUtilityLibrary.h"
#include "UI/Navigation/Minimap/MinimapDistantMarkerWidget.h"
#include "UI/Navigation/Minimap/MinimapMarkerWidget.h"

UMinimapWidget::UMinimapWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	IsActive = true;
}

void UMinimapWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	if(Img_Map)
	{
		if (IsDesignTime() && bShowImageInDesignTime)
		{
			Img_Map->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			Img_Map->SetVisibility(ESlateVisibility::Visible);
		}
	}

	if (IsDesignTime())
	{
		// 에디터에서는 작업해야 하니 투명도 1.0 (완전 불투명)
		SetRenderOpacity(1.f);
		IsActive = true; 
	}
	else
	{
		// 실제 게임 시작 시에는 꺼진 상태(투명)로 시작
		IsActive = false;
		SetRenderOpacity(0.f);
	}
	
	if(RotateMinimapWithPlayer)
	{
		if(Img_Frame) Img_Frame->SetRenderTransformAngle(-90.f);
		if(OV_PointerAnim) OV_PointerAnim->SetRenderTransformAngle(-90.f);
		if(OV_CanvasPanelMap) OV_CanvasPanelMap->SetRenderTransformAngle(-90.f);
	}
}

void UMinimapWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UMinimapWidget::InitMinimapData(const FMinimapInitData& Data)
{
	float Size = FMath::Clamp(Data.Size, 64.f, 512.f);
	UUIUtilityLibrary::SetSizeBoxSize(SB_Content, Size, Size);
	UUIUtilityLibrary::SetImageBrushAndColor(Img_Pointer, Data.PointerIcon, Data.PointerColor);
	SetPointerSize(Data.PointerSize);
	SetMinimapTint(Data.MinimapTint);
	
	MinMaxScale = Data.PointerMinMaxScale;
	ZoomInterpSpeed = Data.ZoomInterpSpeed;
	ScalePointerWithZoom = Data.ScalePointerWithZoom;
	PointerMinMaxScale = Data.PointerMinMaxScale;
	RotateMinimapWithPlayer = Data.RotateMinimapWithPlayer;
	IsFirstPerson = Data.IsFirstPerson;
	
	if(Data.MinimapFrame && Img_Frame)
	{
		Img_Frame->SetRenderOpacity(1.f);
		UUIUtilityLibrary::SetImageBrushAndColor(Img_Frame, Data.MinimapFrame, Data.MinimapFrameTint);
		Img_Frame->SetRenderScale(Data.MinimapFrameScale);
	}
}

void UMinimapWidget::ShowMinimap(FMiniMapInfo Info)
{
	if(IsActive) return;
	IsActive = true;
	SetupMinimapInfo(Info);
	SetRenderOpacity(1.f);
}

void UMinimapWidget::HideMinimap()
{
	if(!IsActive) return;
	IsActive = false;
	SetRenderOpacity(0.f);
}

void UMinimapWidget::UpdateMiniMap(float NewZoomValue)
{
	InterpolateCanvasMapPosition(ZoomInterpSpeed, false);
	InterpolateCanvasMapZoom(NewZoomValue, ZoomInterpSpeed, false);
}

void UMinimapWidget::AddOrUpdateMarker(UObject* Marker, bool ScaleWithZoom, FMiniMapInfo Info)
{
	HandleCreateOrFindMarker(Marker);
	
	if(UMinimapMarkerWidget* FoundMarker = FindMarker(Marker))
	{
		// 마커 정보 업데이트 로직
		FoundMarker->MiniMapInfo = Info;
		// 필요시 추가적인 업데이트 로직 구현
	}
}

void UMinimapWidget::RemoveMarker(UObject* Marker)
{
	// 마커 제거 로직 (구현 필요 시 추가)
}

void UMinimapWidget::RemoveAllMinimapMarker()
{
	// 모든 마커 제거 로직 (구현 필요 시 추가)
}

void UMinimapWidget::SetupMinimapInfo(FMiniMapInfo& Info)
{
	MiniMapInfo = Info;
	SetupMap();
	SetMinimapInfoToMarker();
}

void UMinimapWidget::SetupMap()
{
	BottomLeft = MiniMapInfo.BottomLeftCoordinates;
	BLCache = BottomLeft;
	TopRight = MiniMapInfo.TopRightCoordinates;
	TRCache = TopRight;

	UUIUtilityLibrary::SetImageBrush(Img_Map, MiniMapInfo.MinimapTexture);
}

void UMinimapWidget::SetMinimapInfoToMarker()
{
	if(!OV_MapMarkers) return;

	TArray<UWidget*> Widgets = OV_MapMarkers->GetAllChildren();

	for(auto& Widget : Widgets)
	{
		if(UMinimapMarkerWidget* MarkerWidget = Cast<UMinimapMarkerWidget>(Widget))
		{
			MarkerWidget->MiniMapInfo = MiniMapInfo;
		}
	}
}

void UMinimapWidget::HandleFadeInMinimap(float FadeInSpeedPointer, float FadeInSpeedMap)
{

}

void UMinimapWidget::HandleFadeOutMinimap(float FadeInSpeedPointer, float FadeInSpeedMap)
{
}

APawn* UMinimapWidget::GetPlayerPawn() const
{
	return GetOwningPlayer()->GetPawn();
}

FVector2D UMinimapWidget::GetPlayerLocation2D() const
{
	if (APawn* Pawn = GetPlayerPawn())
	{
		return FVector2D(Pawn->GetActorLocation());
	}
	return FVector2D::ZeroVector;
}

FVector2D UMinimapWidget::GetBottomLeftLocation2D() const
{
	if (IsValid(MiniMapInfo.BottomLeftActor))
	{
		TArray<AActor*> Actors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), MiniMapInfo.BottomLeftActor, Actors);
       
		if (Actors.Num() > 0 && Actors[0])
		{
			return FVector2D(Actors[0]->GetActorLocation());
		}
	}
    
	return FVector2D::ZeroVector;
}

FVector2D UMinimapWidget::GetTopRightLocation2D() const
{
	if (IsValid(MiniMapInfo.TopRightActor))
	{
		TArray<AActor*> Actors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), MiniMapInfo.TopRightActor, Actors);
       
		if (Actors.Num() > 0 && Actors[0])
		{
			return FVector2D(Actors[0]->GetActorLocation());
		}
	}
    
	return FVector2D::ZeroVector;
}

FVector2D UMinimapWidget::FindPlayer2DLocationOnMap(FVector2D InValue) const
{
	return UUIUtilityLibrary::MapRangeClampedVector2D(
			InValue,
			GetBottomLeftLocation2D(),
			GetTopRightLocation2D(),
			BottomLeft,
			TopRight);
}

FVector2D UMinimapWidget::GetCanvasMapPosition() const
{
	if(Img_Map)
	{
		return UWidgetLayoutLibrary::SlotAsCanvasSlot(Img_Map)->GetPosition();
	}
	return FVector2D::ZeroVector;
}

float UMinimapWidget::GetCanvasMapSize() const
{
	if(Img_Map)
	{
		return UWidgetLayoutLibrary::SlotAsCanvasSlot(Img_Map)->GetSize().X;
	}
	return 0.f;
}

void UMinimapWidget::SetCanvasMapPosition(FVector2D InPosition) const
{
	if(Img_Map)
	{
		UWidgetLayoutLibrary::SlotAsCanvasSlot(Img_Map)->SetPosition(InPosition);
	}
	if(B_MapMarkers)
	{
		UWidgetLayoutLibrary::SlotAsCanvasSlot(B_MapMarkers)->SetPosition(InPosition);
	}
}

void UMinimapWidget::SetCanvasMapSize(float InSize) const
{
	FVector2D SizeVector = {InSize, InSize};
	if(Img_Map)
	{
		UWidgetLayoutLibrary::SlotAsCanvasSlot(Img_Map)->SetSize(SizeVector);
	}
	if(B_MapMarkers)
	{
		UWidgetLayoutLibrary::SlotAsCanvasSlot(B_MapMarkers)->SetSize(SizeVector);
	}
}

float UMinimapWidget::GetMapTextureSize() const
{
	if(Img_Map)
	{
		FSlateBrush MapBrush = Img_Map->GetBrush();
		if(UObject* LocalMapImage = MapBrush.GetResourceObject())
		{
			if(UTexture2D* Texture2D = Cast<UTexture2D>(LocalMapImage))
			{
				return Texture2D->GetSizeX();
			}
		}
	}
	return 0.f;
}

void UMinimapWidget::FindZoomAspectRatio(float Value, float& AspectRatio, float& ZoomSize) const
{
	float OutputValue = UKismetMathLibrary::MapRangeClamped(Value, 0.f, 2.f, MapSize / ZoomDivider, MapSize);
	AspectRatio = MapSize / OutputValue;
	ZoomSize = OutputValue;
}

void UMinimapWidget::SetMinimapSize(float InSize)
{
	float NewSize = FMath::Clamp(InSize, 64.f, 512.f);
	UUIUtilityLibrary::SetSizeBoxSize(SB_Content, NewSize, NewSize);
}

void UMinimapWidget::SetPointerSize(float InSize)
{
	UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Img_Pointer);
	if(CanvasSlot)
	{
		FVector2D PointerSize = FVector2D{InSize, InSize};
		FVector2D PointerPosition = UKismetMathLibrary::Divide_Vector2DFloat(PointerSize, 2.f) * -1.f;
	
		CanvasSlot->SetSize(PointerSize);
		UUIUtilityLibrary::SetImageBrushSize(Img_Pointer, PointerSize);
		CanvasSlot->SetPosition(PointerPosition);
	}
}

void UMinimapWidget::SetPointerIconAndColor(UObject* InIcon, FLinearColor InColor)
{
	UUIUtilityLibrary::SetImageBrush(Img_Pointer, InIcon);
	Img_Pointer->SetColorAndOpacity(InColor);
}

void UMinimapWidget::SetBlendMask(UTexture2D* InTexture)
{
	if(UMaterialInstanceDynamic* MID = RB_Minimap->GetEffectMaterial())
	{
		MID->SetTextureParameterValue("Mask", InTexture);
	}
}

void UMinimapWidget::SetMinimapTint(FLinearColor InColor)
{
	if(Img_Map)
	{
		Img_Map->SetColorAndOpacity(InColor);
	}
}

float UMinimapWidget::FindAngleByDirection(int32 InAngle)
{
	float Result = 0.f;
	if(InAngle >= -180.f && InAngle <= -90.f)
	{
		Result = UKismetMathLibrary::MapRangeClamped(InAngle, -90.f, -180.f, 0.f, 90.f);	
	}
	else if(InAngle >= -90.f && InAngle <= 0.f)
	{
		Result = UKismetMathLibrary::MapRangeClamped(InAngle, -90.f, 0.f, 0.f, -90.f);
	}
	else if(InAngle >= 0.f && InAngle <= 90.f)
	{
		Result = UKismetMathLibrary::MapRangeClamped(InAngle, 0.f, 90.f, -90.f, -180.f);
	}
	else if(InAngle >= 90.f && InAngle <= 180.f)
	{
		Result = UKismetMathLibrary::MapRangeClamped(InAngle, 90.f, 180.f, 180.f, 90.f);
	}
	return Result;
}
float UMinimapWidget::GetPlayerPointerAngle()
{
	FRotator CameraRot = GetOwningPlayerCameraManager()->GetCameraRotation();
	FRotator InverseRot = UKismetMathLibrary::MakeRotator(0.f, 0.f, CameraRot.Yaw).GetInverse();

	FRotator PawnRot = GetOwningPlayerPawn()->GetActorRotation();
	
	return UKismetMathLibrary::ComposeRotators(InverseRot, PawnRot).Yaw;
}

bool UMinimapWidget::MarkerObjectInUse(UObject* InMarkerObject)
{
	if(!OV_MapMarkers) return false;
	
	TArray<UWidget*> Markers = OV_MapMarkers->GetAllChildren();

	for(auto Marker : Markers)
	{
		// MarkerWidget 내부에 MarkerObject를 참조하는 변수가 있다고 가정 (헤더 확인 필요)
		// 현재 구현상 TMap으로 관리중이므로 MarkerMap을 조회하는 것이 더 정확함
		if(MarkerMap.Contains(InMarkerObject))
		{
			return true;
		}
	}
	return false;
}

UObject* UMinimapWidget::ValidateMarkerObject(UObject* InObject)
{
	if (InObject)
	{
		if (AActor* Actor = Cast<AActor>(InObject))
		{
			return Cast<USceneComponent>(Actor) ? InObject : nullptr;
		}
	}
	return nullptr;
}

bool UMinimapWidget::IsMarkerObjectInUse(UObject* InObject)
{
	if(InObject)
	{
		return MarkerObjectInUse(InObject); 
	}
	return false;
}

UMinimapMarkerWidget* UMinimapWidget::FindMarker(UObject* InObject)
{
	if(UObject* ValidateObj = ValidateMarkerObject(InObject))
	{
		if(MarkerMap.Contains(ValidateObj))
		{
			return MarkerMap[ValidateObj];
		}
	}
	return nullptr;
}

UMinimapDistantMarkerWidget* UMinimapWidget::FindDistantMarker(UMinimapMarkerWidget* InMarkerWidget)
{
	if(InMarkerWidget && DistantMarkerMap.Contains(InMarkerWidget))
	{
		return DistantMarkerMap[InMarkerWidget];
	}
	return nullptr;
}

void UMinimapWidget::InterpolateCanvasMapPosition(float InInterpSpeed, bool InInterpConstant)
{
	FVector2D NewCanvasMapPosition = UUIUtilityLibrary::Vector2DInterpolate(
		GetWorld(),
		GetCanvasMapPosition(),
		FindPlayer2DLocationOnMap(GetPlayerLocation2D()),
		InInterpSpeed,
		InInterpConstant);

	SetCanvasMapPosition(NewCanvasMapPosition);
}

void UMinimapWidget::InterpolateCanvasMapZoom(float InZoomValue, float InInterpSpeed, bool InInterpConstant)
{
	CurrentZoomValue = UUIUtilityLibrary::FInterpolate(
		GetWorld(),
		CurrentZoomValue,
		InZoomValue,
		InInterpSpeed/2,
		InInterpConstant);
	
	float ClampedZoomValue = FMath::Clamp(CurrentZoomValue, 0.f, 2.f);
	float OutAspectRatio, OutZoomSize;
	
	FindZoomAspectRatio(ClampedZoomValue, OutAspectRatio, OutZoomSize);

	BottomLeft = UUIUtilityLibrary::Vector2DInterpolate(
		GetWorld(),
		BottomLeft,
		BLCache/OutAspectRatio,
		InInterpSpeed,
		InInterpConstant);

	TopRight = UUIUtilityLibrary::Vector2DInterpolate(
		GetWorld(),
		TopRight,
		TRCache/OutAspectRatio,
		InInterpSpeed,
		InInterpConstant);
	
	float NewCanvasMapSize = UUIUtilityLibrary::FInterpolate(
		GetWorld(),
		GetCanvasMapSize(),
		OutZoomSize,
		InInterpSpeed,
		InInterpConstant);
	
	SetCanvasMapSize(NewCanvasMapSize);
}

void UMinimapWidget::HandleCreateOrFindMarker(UObject* InObject)
{
	UObject* ValidObject = ValidateMarkerObject(InObject);
	if(!ValidObject) return;

	bool IsUse = IsMarkerObjectInUse(ValidObject);

	if(IsUse)
	{
		CurrentMarker = FindMarker(ValidObject);
		CurrentDistantMarker = FindDistantMarker(CurrentMarker);
	}
	else
	{
		if(MinimapDistantMarkerClass && MinimapMarkerClass)
		{
			UMinimapDistantMarkerWidget* DistantMarkerWidget =
				CreateWidget<UMinimapDistantMarkerWidget>(this, MinimapDistantMarkerClass);

			if(DistantMarkerWidget)
			{
				CurrentDistantMarker = DistantMarkerWidget;
			}
			UUIUtilityLibrary::AddChildToOverlay(
				OV_DistantMarker,
				CurrentDistantMarker,
				VAlign_Center,
				HAlign_Center);

			UMinimapMarkerWidget* MinimapMarkerWidget
				= CreateWidget<UMinimapMarkerWidget>(this, MinimapMarkerClass);

			CurrentMarker = MinimapMarkerWidget;
			UUIUtilityLibrary::AddChildToOverlay(
				OV_MapMarkers,
				MinimapMarkerWidget,
				VAlign_Center,
				HAlign_Center);

			MarkerMap.Add(ValidObject, CurrentMarker);
			// DistantMarkerMap은 UserWidget이 아니라 UMinimapDistantMarkerWidget을 저장하도록 캐스팅 필요
			if(UMinimapDistantMarkerWidget* CastedDistant = Cast<UMinimapDistantMarkerWidget>(CurrentDistantMarker))
			{
				DistantMarkerMap.Add(CurrentMarker, CastedDistant);
			}
		}
	}
}