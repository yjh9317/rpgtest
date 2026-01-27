
#include "UI/UIUtilityLibrary.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/ProgressBar.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"


#pragma region SizeBox

void UUIUtilityLibrary::SetSizeBoxSize(USizeBox* _SizeBox, float NewWidth, float NewHeight)
{
	if(nullptr != _SizeBox)
	{
		_SizeBox->SetWidthOverride(NewWidth);
		_SizeBox->SetHeightOverride(NewHeight);
	}
}

void UUIUtilityLibrary::GetSizeBoxSize(USizeBox* _SizeBox, float& OutWidth, float& OutHeight)
{
	if(nullptr != _SizeBox)
	{
		OutWidth = _SizeBox->GetWidthOverride();
		OutHeight = _SizeBox->GetHeightOverride();
	}
}
#pragma endregion

#pragma region ProgressBar

void UUIUtilityLibrary::SetProgressBarFillImage(UProgressBar* _ProgressBar, UObject* NewImage)
{
	ModifyProgressBarFillImage(_ProgressBar, [NewImage](FSlateBrush& Brush) {
		Brush.SetResourceObject(NewImage);
	});
}

void UUIUtilityLibrary::SetProgressBarFillImageSize(UProgressBar* _ProgressBar, FVector2D NewSize)
{
	ModifyProgressBarFillImage(_ProgressBar, [NewSize](FSlateBrush& Brush) {
		Brush.ImageSize = NewSize;
	});
}

void UUIUtilityLibrary::SetProgressBarFillImageTiling(UProgressBar* _ProgressBar, ESlateBrushTileType::Type NewType)
{
	ModifyProgressBarFillImage(_ProgressBar, [NewType](FSlateBrush& Brush) {
		Brush.Tiling = NewType;
	});
}

void UUIUtilityLibrary::SetProgressBarFillImageMargin(UProgressBar* _ProgressBar, float NewMargin)
{
	ModifyProgressBarFillImage(_ProgressBar, [NewMargin](FSlateBrush& Brush) {
		Brush.Margin.Bottom = NewMargin;
		Brush.Margin.Top = NewMargin;
		Brush.Margin.Left = NewMargin;
		Brush.Margin.Right = NewMargin;
	});
}

void UUIUtilityLibrary::SetProgressBarFillImageDrawAS(UProgressBar* _ProgressBar, ESlateBrushDrawType::Type NewType)
{
	ModifyProgressBarFillImage(_ProgressBar, [NewType](FSlateBrush& Brush) {
		Brush.DrawAs = NewType;
	});
}

void UUIUtilityLibrary::SetProgressBarMarqueeImageSize(UProgressBar* _ProgressBar, FVector2D NewSize)
{
	ModifyProgressBarMarquee(_ProgressBar, [NewSize](FSlateBrush& Brush) {
		Brush.ImageSize = NewSize;
	});
}

void UUIUtilityLibrary::SetProgressBarMarqueeMargin(UProgressBar* _ProgressBar, FMargin NewMargin)
{
	ModifyProgressBarMarquee(_ProgressBar, [NewMargin](FSlateBrush& Brush) {
		Brush.Margin = NewMargin;
	});
}

void UUIUtilityLibrary::SetProgressBarMarqueeTint(UProgressBar* _ProgressBar, FLinearColor NewTint)
{
	ModifyProgressBarMarquee(_ProgressBar, [NewTint](FSlateBrush& Brush) {
		FSlateColor NewTintColor(NewTint);
		Brush.TintColor = NewTintColor;
	});
}

void UUIUtilityLibrary::SetProgressBarMarqueeDrawAs(UProgressBar* _ProgressBar, ESlateBrushDrawType::Type NewType)
{
	ModifyProgressBarMarquee(_ProgressBar, [NewType](FSlateBrush& Brush) {
		Brush.DrawAs = NewType;
	});
}

void UUIUtilityLibrary::SetProgressBarMarqueeTiling(UProgressBar* _ProgressBar, ESlateBrushTileType::Type NewType)
{
	ModifyProgressBarMarquee(_ProgressBar, [NewType](FSlateBrush& Brush) {
	Brush.Tiling = NewType;
});
}

void UUIUtilityLibrary::SetProgressBarMarqueeMirroring(UProgressBar* _ProgressBar, ESlateBrushMirrorType::Type NewType)
{
	ModifyProgressBarMarquee(_ProgressBar, [NewType](FSlateBrush& Brush) {
		Brush.Mirroring = NewType;
	});
}

void UUIUtilityLibrary::SetProgressBarBackgroundTint(UProgressBar* _ProgressBar, FLinearColor NewTint)
{
	ModifyProgressBarBackgroundImage(_ProgressBar, [NewTint](FSlateBrush& Brush) {
		FSlateColor NewColor(NewTint);
		Brush.TintColor = NewColor;
});
}


void UUIUtilityLibrary::SetProgressBarMarqueeImage(UProgressBar* _ProgressBar, UObject* NewImage)
{
	ModifyProgressBarMarquee(_ProgressBar, [NewImage](FSlateBrush& Brush)
	{
		Brush.SetResourceObject(NewImage);
	});
}

#pragma endregion

#pragma region Border

void UUIUtilityLibrary::SetBorderBrush(UBorder* _Border, UObject* NewBrush)
{
	FSlateBrush BackgroundBrush = _Border->Background;
	BackgroundBrush.SetResourceObject(NewBrush);
	_Border->SetBrush(BackgroundBrush);
}

void UUIUtilityLibrary::SetBorderBrushSize(UBorder* _Border, FVector2D NewSize)
{
	FSlateBrush BackgroundBrush = _Border->Background;
	BackgroundBrush.ImageSize = NewSize;
	_Border->SetBrush(BackgroundBrush);
}

void UUIUtilityLibrary::SetBorderBrushTiling(UBorder* _Border, ESlateBrushTileType::Type NewType)
{
	FSlateBrush BackgroundBrush = _Border->Background;
	BackgroundBrush.Tiling = NewType;
	_Border->SetBrush(BackgroundBrush);
}

#pragma endregion

#pragma region Utility

UOverlaySlot* UUIUtilityLibrary::AddChildToOverlay(UOverlay* _Overlay, UUserWidget* ChildWidget,
	EVerticalAlignment NewVerticalAlignment, EHorizontalAlignment NewHorizontalAlignment)
{
	UOverlaySlot* Slot = _Overlay->AddChildToOverlay(ChildWidget);
	if(Slot)
	{
		Slot->SetVerticalAlignment(NewVerticalAlignment);
		Slot->SetHorizontalAlignment(NewHorizontalAlignment);
		return Slot;
	}
	return nullptr;
}

FVector UUIUtilityLibrary::GetMiniMapActorLocation(UObject* WorldContextObject, TSubclassOf<AActor> ActorClass)
{
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(WorldContextObject,ActorClass,Actors);

	if(!Actors.IsEmpty())
	{
		return Actors[0]->GetActorLocation();
	}
	return FVector{};
}

UVerticalBoxSlot* UUIUtilityLibrary::AddChildToVerticalBox(UVerticalBox* _VerticalBox, UUserWidget* ChildWidget,
	EVerticalAlignment NewVerticalAlignment, EHorizontalAlignment NewHorizontalAlignment)
{
	UVerticalBoxSlot* Slot = _VerticalBox->AddChildToVerticalBox(ChildWidget);
	if(Slot)
	{
		Slot->SetVerticalAlignment(NewVerticalAlignment);
		Slot->SetHorizontalAlignment(NewHorizontalAlignment);
		return Slot;
	}
	return nullptr;
}

UHorizontalBoxSlot* UUIUtilityLibrary::AddChildToHorizontalBox(UHorizontalBox* _HorizontalBox, UUserWidget* ChildWidget,
	EVerticalAlignment NewVerticalAlignment, EHorizontalAlignment NewHorizontalAlignment)
{
	UHorizontalBoxSlot* Slot = _HorizontalBox->AddChildToHorizontalBox(ChildWidget);
	if(Slot)
	{
		Slot->SetVerticalAlignment(NewVerticalAlignment);
		Slot->SetHorizontalAlignment(NewHorizontalAlignment);
		return Slot;
	}
	return nullptr;
}

FVector2D UUIUtilityLibrary::GetTexture2DSize(UTexture2D* Texture2D)
{
	FVector2D NewVector = UKismetMathLibrary::MakeVector2D(Texture2D->GetSizeX(),Texture2D->GetSizeY());
	return NewVector;
}

FVector2D UUIUtilityLibrary::GetTexture2DSizeClamp(UTexture2D* Texture2D, FVector2D Clamp)
{
	float NewX = FMath::Clamp(Texture2D->GetSizeX(),0.f,Clamp.X);
	float NewY = FMath::Clamp(Texture2D->GetSizeY(),0.f,Clamp.Y);
	FVector2D NewVector = UKismetMathLibrary::MakeVector2D(NewX,NewY);
	return NewVector;
}

float UUIUtilityLibrary::RandomizePositiveNegativeFloat(float NewValue)
{
	if(UKismetMathLibrary::RandomBool())
	{
		return -NewValue;
	}
	return NewValue;
}

float UUIUtilityLibrary::FInterpolate(UObject* WorldContextObject, float Current, float Target, float InterpSpeed,
	bool InterpConstant)
{
	float DeltaTime = UGameplayStatics::GetWorldDeltaSeconds(WorldContextObject);

	float Result = 0.f;
	if(InterpConstant)
	{
		Result = UKismetMathLibrary::FInterpTo_Constant(Current,Target,DeltaTime,InterpSpeed);	
	}
	else
	{
		Result = UKismetMathLibrary::FInterpTo(Current,Target,DeltaTime,InterpSpeed);
	}
	return Result;
}

FVector2D UUIUtilityLibrary::Vector2DInterpolate(UObject* WorldContextObject, FVector2D Current, FVector2D Target,
	float InterpSpeed, bool InterpConstant)
{
	float DeltaTime = UGameplayStatics::GetWorldDeltaSeconds(WorldContextObject);

	FVector2D Result {};
	if(InterpConstant)
	{
		Result = UKismetMathLibrary::Vector2DInterpTo_Constant(Current,Target,DeltaTime,InterpSpeed);	
	}
	else
	{
		Result = UKismetMathLibrary::Vector2DInterpTo(Current,Target,DeltaTime,InterpSpeed);
	}
	return Result;
}

FVector2D UUIUtilityLibrary::MapRangeClampedVector2D(FVector2D Value, FVector2D InRangeA, FVector2D InRangeB,
													 FVector2D OutRangeA, FVector2D OutRangeB)
{
	double X = UKismetMathLibrary::MapRangeClamped(Value.X, InRangeA.X, InRangeB.X, OutRangeA.X, OutRangeA.X);
	double Y = UKismetMathLibrary::MapRangeClamped(Value.Y, InRangeA.Y, InRangeB.Y, OutRangeA.Y, OutRangeA.Y);
	return FVector2D{X, Y};
}

FVector2D UUIUtilityLibrary::ClampVector2D(FVector2D Value, float Min, float Max)
{
	float NewX = FMath::Clamp(Value.X,Min,Max);
	float NewY = FMath::Clamp(Value.Y,Min,Max);
	return FVector2D{NewX,NewY};
}

void UUIUtilityLibrary::InterpRenderOpacity(UObject* WorldContextObject,UUserWidget* Target, float TargetValue, float InterpSpeed,
	bool InterpConstant)
{
	float Opacity = Target->GetRenderOpacity();
	Target->SetRenderOpacity(FInterpolate(WorldContextObject,Opacity,TargetValue,InterpSpeed,InterpConstant));
}

void UUIUtilityLibrary::InterpRenderTransform(UObject* WorldContextObject,UUserWidget* Target, ERenderTransform TransformType,
	FVector2D TargetValue, float InterpSpeed, bool InterpConstant)
{
	const FWidgetTransform& OldTransform = Target->GetRenderTransform();
	FVector2D InterpValue;
	
	switch (TransformType)
	{
	case Translation:
		{
			InterpValue = OldTransform.Translation;
			Vector2DInterpolate(WorldContextObject,InterpValue,TargetValue,InterpSpeed,InterpConstant);
			Target->SetRenderTranslation(InterpValue);
		}
		break;
	case Scale:
		{
			InterpValue = OldTransform.Scale;
			Vector2DInterpolate(WorldContextObject,InterpValue,TargetValue,InterpSpeed,InterpConstant);
			Target->SetRenderScale(InterpValue);
		}
		break;
	case Shear:
		{
			InterpValue = OldTransform.Shear;
			Vector2DInterpolate(WorldContextObject,InterpValue,TargetValue,InterpSpeed,InterpConstant);
			Target->SetRenderShear(InterpValue);
		}
		break;
	}
	
}

void UUIUtilityLibrary::InterpRenderAngle(UObject* WorldContextObject,UUserWidget* Target, float TargetValue, float InterpSpeed,
	bool InterpConstant)
{
	const FWidgetTransform& OldTransform = Target->GetRenderTransform();
	float InterpValue = OldTransform.Angle;
	FInterpolate(WorldContextObject,InterpValue,TargetValue,InterpSpeed,InterpConstant);
	Target->SetRenderTransformAngle(InterpValue);
}

bool UUIUtilityLibrary::HasColorChanged(FLinearColor InColor)
{
	return (InColor.R != 0 || InColor.G != 0 || InColor.B != 0 || InColor.A != 0); 
}

FSlateFontInfo UUIUtilityLibrary::FindMarkerTextFont(FTextInfo TextInfo, FSlateFontInfo StylesheetFont,
	int32 TextSizeScaleRange)
{
	FSlateFontInfo NewFontInfo = StylesheetFont;
	if(TextInfo.UseManualFontInfo)
	{
		NewFontInfo = TextInfo.FontInfo;
	}

	return FindTextScaleRange(TextInfo,NewFontInfo,TextSizeScaleRange);
}

FSlateFontInfo UUIUtilityLibrary::FindTextScaleRange(FTextInfo TextInfo, FSlateFontInfo FontInfo,
	int32 SizeScaleRange)
{
	FSlateFontInfo NewInfo = FontInfo;
	
	if(TextInfo.Scale != 1.f)
	{
		float ClampValue = FMath::Clamp(TextInfo.Scale,0.f,2.f);
		float MapRangeClampValue = UKismetMathLibrary::MapRangeClamped(ClampValue,0.f,2.f,FontInfo.Size - SizeScaleRange,FontInfo.Size + SizeScaleRange);
		int32 TruncValue = UKismetMathLibrary::FTrunc(MapRangeClampValue);
		NewInfo.Size = TruncValue;
	}
	else
	{
		NewInfo.Size = SizeScaleRange;
	}
	
	return NewInfo;
}

bool UUIUtilityLibrary::HasTransformChanged(FWidgetTransform InTransform)
{
	return
		InTransform.Translation.X != 0.f || InTransform.Translation.Y != 0.f ||
		InTransform.Scale.X != 1.f || InTransform.Scale.Y != 1.f ||
		InTransform.Shear.X != 0.f || InTransform.Shear.Y != 0.f ||
		InTransform.Angle != 0.f;

}

FVector2D UUIUtilityLibrary::LerpVector2D(FVector2D A, FVector2D B, float Alpha)
{
	return FVector2D{UKismetMathLibrary::Lerp(A.X,B.X,Alpha),	UKismetMathLibrary::Lerp(A.Y,B.Y,Alpha)};
}

FLinearColor UUIUtilityLibrary::SubtractFromColor(FLinearColor InColor, float Value)
{
	return FLinearColor{
		FMath::Clamp(InColor.R - Value, 0.f, 1.f),
		FMath::Clamp(InColor.G - Value, 0.f, 1.f),
		FMath::Clamp(InColor.B - Value, 0.f, 1.f),
		InColor.A
	};
	
}

FLinearColor UUIUtilityLibrary::SubtractBrightnessValue(FLinearColor InColor, float Value)
{
	float H,S,V,A;
	UKismetMathLibrary::RGBToHSV(InColor,H,S,V,A);
	
	return UKismetMathLibrary::HSVToRGB(H,S,FMath::Clamp(V-Value,0.f,1.f),A);
}

void UUIUtilityLibrary::SetImageBrush(UImage* Target, UObject* Brush)
{
	if (!Target || !Brush) return;
	UTexture* Texture = Cast<UTexture>(Brush);
	
	if (Texture)
	{
		FSlateBrush NewBrush = Target->GetBrush();
		NewBrush.SetResourceObject(Texture);
		Target->SetBrush(NewBrush);
	}
	else
	{
	}
}

void UUIUtilityLibrary::SetImageBrushAndColor(UImage* Target, UObject* Brush, FLinearColor Color)
{
	if (!Target || !Brush) return;
	
	SetImageBrush(Target,Brush);
	Target->SetColorAndOpacity(Color);
}

void UUIUtilityLibrary::SetImageBrushTiling(UImage* Target, ESlateBrushTileType::Type NewType)
{
	if (!Target) return;
	
	FSlateBrush NewBrush = Target->GetBrush();
	NewBrush.Tiling = NewType;
	Target->SetBrush(NewBrush);	
}

void UUIUtilityLibrary::SetImageBrushSize(UImage* Target, FVector2D NewSize)
{
	FSlateBrush NewBrush = Target->GetBrush();
	NewBrush.ImageSize = NewSize;
	Target->SetBrush(NewBrush);	
}

#pragma endregion 


#pragma region DelegateFunction

void ModifyProgressBarFillImage(UProgressBar* InProgressBar, TFunction<void(FSlateBrush&)> ModifyFunc)
{
	if (!InProgressBar) return;

	FSlateBrush Brush = InProgressBar->GetWidgetStyle().FillImage;
	
	ModifyFunc(Brush);
	
	FProgressBarStyle NewStyle = InProgressBar->GetWidgetStyle();
	NewStyle.SetFillImage(Brush);

	InProgressBar->SetWidgetStyle(NewStyle);
	InProgressBar->SynchronizeProperties();
}

void ModifyProgressBarMarquee(UProgressBar* InProgressBar, TFunction<void(FSlateBrush&)> ModifyFunc)
{
	if (!InProgressBar) return;

	FSlateBrush Brush = InProgressBar->GetWidgetStyle().MarqueeImage;
	
	ModifyFunc(Brush);
	
	FProgressBarStyle NewStyle = InProgressBar->GetWidgetStyle();
	NewStyle.SetMarqueeImage(Brush);

	InProgressBar->SetWidgetStyle(NewStyle);
	InProgressBar->SynchronizeProperties();
}

void ModifyProgressBarBackgroundImage(UProgressBar* InProgressBar, TFunction<void(FSlateBrush&)> ModifyFunc)
{
	if (!InProgressBar) return;

	FSlateBrush Brush = InProgressBar->GetWidgetStyle().MarqueeImage;
	
	ModifyFunc(Brush);
	
	FProgressBarStyle NewStyle = InProgressBar->GetWidgetStyle();
	NewStyle.SetBackgroundImage(Brush);

	InProgressBar->SetWidgetStyle(NewStyle);
	InProgressBar->SynchronizeProperties();
}



#pragma endregion

#pragma region HUD

int32 UUIUtilityLibrary::GetCardinalDirection(UObject* WorldContextObject,TSubclassOf<AActor> ActorClass)
{
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(WorldContextObject,ActorClass,Actors);

	if(Actors.IsEmpty()) return 0;
	
	for(const auto& Actor : Actors)
	{
		// Actor->GetActorLocation()
	}
	return 0;
}

FVector UUIUtilityLibrary::GetActorLocationByClassAndIndex(UObject* WorldContextObject,TSubclassOf<AActor> ActorClass, int32 index)
{
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(WorldContextObject,ActorClass,Actors);

	if(Actors.IsValidIndex(index)) return FVector{};
	
	return Actors[index]->GetActorLocation();
}

#pragma endregion 


void UUIUtilityLibrary::SetImageScalarParameter(UImage* InImage, FName ParamName, float Value)
{
	if (InImage)
	{
		if (UMaterialInstanceDynamic* MID = InImage->GetDynamicMaterial())
		{
			MID->SetScalarParameterValue(ParamName, Value);
		}
	}
}

void UUIUtilityLibrary::SetImageTextureParameter(UImage* InImage, FName ParamName, UTexture* Value)
{
	if (InImage)
	{
		if (UMaterialInstanceDynamic* MID = InImage->GetDynamicMaterial())
		{
			MID->SetTextureParameterValue(ParamName, Value);
		}
	}
}

void UUIUtilityLibrary::SetImageVectorParameter(UImage* InImage, FName ParamName, FLinearColor Value)
{
	if (InImage)
	{
		if (UMaterialInstanceDynamic* MID = InImage->GetDynamicMaterial())
		{
			MID->SetVectorParameterValue(ParamName, Value);
		}
	}
}

void UUIUtilityLibrary::SetWidgetRenderAngle(UWidget* InWidget, float Angle)
{
	if (InWidget)
	{
		InWidget->SetRenderTransformAngle(Angle);
	}
}

void UUIUtilityLibrary::SetWidgetRenderTranslation(UWidget* InWidget, FVector2D Translation)
{
	if (InWidget)
	{
		InWidget->SetRenderTranslation(Translation);
	}
}

void UUIUtilityLibrary::SetWidgetRenderScale(UWidget* InWidget, FVector2D Scale)
{
	if (InWidget)
	{
		InWidget->SetRenderScale(Scale);
	}
}

void UUIUtilityLibrary::FindParentWidgetByClass(UWidget* InWidget, TSubclassOf<UUserWidget> ParentClass, UUserWidget*& FoundWidget)
{
	FoundWidget = nullptr;
	if (!InWidget || !ParentClass) return;

	UWidget* CurrentParent = InWidget->GetParent();
	while (CurrentParent)
	{
		if (CurrentParent->IsA(ParentClass))
		{
			FoundWidget = Cast<UUserWidget>(CurrentParent);
			return;
		}
		CurrentParent = CurrentParent->GetParent();
	}
}

void UUIUtilityLibrary::SetWidgetVisibilityByBool(UWidget* InWidget, bool bVisible, ESlateVisibility TrueVisibility,
	ESlateVisibility FalseVisibility)
{
	if (InWidget)
	{
		InWidget->SetVisibility(bVisible ? TrueVisibility : FalseVisibility);
	}
}

bool UUIUtilityLibrary::GetScreenPositionForWorldLocation(const UObject* WorldContextObject, FVector WorldLocation,
	FVector2D& OutScreenPosition, FVector2D ScreenOffset, bool bPlayerViewportRelative)
{
	if (!WorldContextObject) return false;

	APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
	if (!PC) return false;

	FVector2D ScreenPos;
	bool bOnScreen = PC->ProjectWorldLocationToScreen(WorldLocation, ScreenPos, bPlayerViewportRelative);

	if (bOnScreen)
	{
		float ViewportScale = UWidgetLayoutLibrary::GetViewportScale(WorldContextObject);
		
		if (ViewportScale <= 0.f) ViewportScale = 1.f;

		OutScreenPosition = ScreenPos + (ScreenOffset * ViewportScale);
		return true;
	}

	return false;
}

UTexture2D* UUIUtilityLibrary::LoadTextureFromSoftPtr(TSoftObjectPtr<UTexture2D> SoftTexture)
{
	if (SoftTexture.IsNull())
	{
		return nullptr;
	}

	if (UTexture2D* LoadedTexture = SoftTexture.Get())
	{
		return LoadedTexture;
	}
	
	return SoftTexture.LoadSynchronous();
}
