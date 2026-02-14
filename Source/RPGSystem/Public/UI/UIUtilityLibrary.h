// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SlateWrapperTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UI/UIEnums.h"
#include "UIUtilityLibrary.generated.h"

class UWidget;
class UHorizontalBox;
class UHorizontalBoxSlot;
class UVerticalBox;
class UVerticalBoxSlot;
class UOverlaySlot;
class UOverlay;
class UBorder;
class USizeBox;
class UProgressBar;
class UImage;
/**
 * 
 */

UENUM()
enum ERenderTransform
{
	Translation,
	Scale,
	Shear
};


UCLASS()
class RPGSYSTEM_API UUIUtilityLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	// SizeBox
	UFUNCTION(BlueprintCallable , Category= "SizeBox")
	static void SetSizeBoxSize(USizeBox* _SizeBox,float NewWidth, float NewHeight);
	UFUNCTION(BlueprintCallable , Category= "SizeBox")
	static void GetSizeBoxSize(USizeBox* _SizeBox,float& OutWidth, float& OutHeight);
	
	// ProgressBar FillImage
	UFUNCTION(BlueprintCallable , Category= "ProgressBar")
	static void SetProgressBarFillImage(UProgressBar* _ProgressBar,UObject* NewImage);
	UFUNCTION(BlueprintCallable , Category= "ProgressBar")
	static void SetProgressBarFillImageSize(UProgressBar* _ProgressBar,FVector2D NewSize);
	UFUNCTION(BlueprintCallable , Category= "ProgressBar")
	static void SetProgressBarFillImageTiling(UProgressBar* _ProgressBar,ESlateBrushTileType::Type NewType);
	UFUNCTION(BlueprintCallable , Category= "ProgressBar")
	static void SetProgressBarFillImageMargin(UProgressBar* _ProgressBar, float NewMargin);
	UFUNCTION(BlueprintCallable , Category= "ProgressBar")
	static void SetProgressBarFillImageDrawAS(UProgressBar* _ProgressBar,ESlateBrushDrawType::Type NewType);
	
	// ProgressBar Marquee
	UFUNCTION(BlueprintCallable , Category= "ProgressBar")
	static void SetProgressBarMarqueeImage(UProgressBar* _ProgressBar, UObject* NewImage);
	UFUNCTION(BlueprintCallable , Category= "ProgressBar")
	static void SetProgressBarMarqueeImageSize(UProgressBar* _ProgressBar,FVector2D NewSize);
	UFUNCTION(BlueprintCallable , Category= "ProgressBar")
	static void SetProgressBarMarqueeMargin(UProgressBar* _ProgressBar, FMargin NewMargin);
	UFUNCTION(BlueprintCallable , Category= "ProgressBar")
	static void SetProgressBarMarqueeTint(UProgressBar* _ProgressBar, FLinearColor NewTint);
	UFUNCTION(BlueprintCallable , Category= "ProgressBar")
	static void SetProgressBarMarqueeDrawAs(UProgressBar* _ProgressBar, ESlateBrushDrawType::Type NewType);
	UFUNCTION(BlueprintCallable , Category= "ProgressBar")
	static void SetProgressBarMarqueeTiling(UProgressBar* _ProgressBar, ESlateBrushTileType::Type NewType);
	UFUNCTION(BlueprintCallable , Category= "ProgressBar")
	static void SetProgressBarMarqueeMirroring(UProgressBar* _ProgressBar, ESlateBrushMirrorType::Type NewType);
	
	// Background
	UFUNCTION(BlueprintCallable , Category= "Background")
	static void SetProgressBarBackgroundTint(UProgressBar* ProgressBar, FLinearColor NewTint);
	
	// Border
	UFUNCTION(BlueprintCallable , Category= "Border")
	static void SetBorderBrush(UBorder* _Border,UObject* NewBrush);
	UFUNCTION(BlueprintCallable , Category= "Border")
	static void SetBorderBrushSize(UBorder* _Border,FVector2D NewSize);
	UFUNCTION(BlueprintCallable , Category= "Border")
	static void SetBorderBrushTiling(UBorder* _Border,ESlateBrushTileType::Type NewType);
	
	// Overlay
	UFUNCTION(BlueprintCallable , Category= "Overlay")
	static UOverlaySlot* AddChildToOverlay(UOverlay* _Overlay,UUserWidget* ChildWidget,
		EVerticalAlignment NewVerticalAlignment,EHorizontalAlignment NewHorizontalAlignment);
	
	// Minimap
	UFUNCTION(BlueprintCallable, Category="MiniMap", meta=(WorldContext="WorldContextObject"))
	static FVector GetMiniMapActorLocation(UObject* WorldContextObject, TSubclassOf<AActor> ActorClass);
	
	// Vertical Box
	UFUNCTION(BlueprintCallable , Category= "Vertical Box")
	static UVerticalBoxSlot* AddChildToVerticalBox(UVerticalBox* _VerticalBox,UUserWidget* ChildWidget,
	EVerticalAlignment NewVerticalAlignment,EHorizontalAlignment NewHorizontalAlignment);
	
	// Horizontal Box
	UFUNCTION(BlueprintCallable , Category= "Horizontal Box")
	static UHorizontalBoxSlot* AddChildToHorizontalBox(UHorizontalBox* _HorizontalBox,UUserWidget* ChildWidget,
	EVerticalAlignment NewVerticalAlignment,EHorizontalAlignment NewHorizontalAlignment);
	
	// Utility
	UFUNCTION(BlueprintCallable, Category="Utility")
	static FVector2D GetTexture2DSize(UTexture2D* Texture2D);
	
	UFUNCTION(BlueprintCallable, Category="Utility")
	static FVector2D GetTexture2DSizeClamp(UTexture2D* Texture2D,FVector2D Clamp);
	
	UFUNCTION(BlueprintCallable, Category="Utility")
	static float RandomizePositiveNegativeFloat(float NewValue);
	
	UFUNCTION(BlueprintCallable, Category="Utility", meta=(WorldContext="WorldContextObject"))
	static float FInterpolate(UObject* WorldContextObject,float Current, float Target, float InterpSpeed, bool InterpConstant);
	
	UFUNCTION(BlueprintCallable, Category="Utility", meta=(WorldContext="WorldContextObject"))
	static FVector2D Vector2DInterpolate(UObject* WorldContextObject,FVector2D Current, FVector2D Target, float InterpSpeed, bool InterpConstant);
	
	UFUNCTION(BlueprintCallable, Category="Utility")
	static FVector2D MapRangeClampedVector2D(FVector2D Value,FVector2D InRangeA,FVector2D InRangeB,FVector2D OutRangeA,FVector2D OutRangeB);
	
	UFUNCTION(BlueprintCallable, Category="Utility")
	static FVector2D ClampVector2D(FVector2D Value, float Min, float Max);
	UFUNCTION(BlueprintCallable, Category="Utility", meta=(WorldContext="WorldContextObject"))
	static void InterpRenderOpacity(UObject* WorldContextObject,UWidget* Target, float TargetValue, float InterpSpeed, bool InterpConstant);
	UFUNCTION(BlueprintCallable, Category="Utility", meta=(WorldContext="WorldContextObject"))
	static void InterpRenderTransform(UObject* WorldContextObject,UWidget* Target, ERenderTransform TransformType, FVector2D TargetValue, float InterpSpeed, bool InterpConstant);
	UFUNCTION(BlueprintCallable, Category="Utility", meta=(WorldContext="WorldContextObject"))
	static void InterpRenderAngle(UObject* WorldContextObject,UWidget* Target, float TargetValue, float InterpSpeed, bool InterpConstant);
	UFUNCTION(BlueprintCallable, Category="Utility")
	static bool HasColorChanged(FLinearColor InColor);
	UFUNCTION(BlueprintCallable, Category="Utility")
	static FSlateFontInfo FindMarkerTextFont(FTextInfo TextInfo, FSlateFontInfo Stylesheet_ont,int32 TextSizeScaleRange);
	UFUNCTION(BlueprintCallable, Category="Utility")
	static FSlateFontInfo FindTextScaleRange(FTextInfo TextInfo, FSlateFontInfo FontInfo,int32 SizeScaleRange);
	UFUNCTION(BlueprintCallable, Category="Utility")
	static bool HasTransformChanged(FWidgetTransform InTransform);
	UFUNCTION(BlueprintCallable, Category="Utility")
	static FVector2D LerpVector2D(FVector2D A, FVector2D B, float Alpha);
	
	// Color
	UFUNCTION(BlueprintCallable, Category="Color")
	static FLinearColor SubtractFromColor(FLinearColor InColor, float Value);
	UFUNCTION(BlueprintCallable, Category="Color")
	static FLinearColor SubtractBrightnessValue(FLinearColor InColor, float Value);
	
	// Image
	UFUNCTION(BlueprintCallable, Category="Image")
	static void SetImageBrush(UImage* Target, UObject* Brush);
	UFUNCTION(BlueprintCallable, Category="Image")
	static void SetImageBrushAndColor(UImage* Target, UObject* Brush,FLinearColor Color);
	UFUNCTION(BlueprintCallable, Category="Image")
	static void SetImageBrushTiling(UImage* Target, ESlateBrushTileType::Type NewType);
	UFUNCTION(BlueprintCallable, Category="Image")
	static void SetImageBrushSize(UImage* Target, FVector2D NewSize);

	// HUD
	UFUNCTION(BlueprintCallable , Category= "HUD")
	static int32 GetCardinalDirection(UObject* WorldContextObject,TSubclassOf<AActor> ActorClass);

	UFUNCTION(BlueprintCallable, Category = "Actor Location")
	static FVector GetActorLocationByClassAndIndex(UObject* WorldContextObject,TSubclassOf<AActor> ActorClass,int32 index = 0);

	
	UFUNCTION(BlueprintCallable, Category = "RPGSystem|UI|Material")
	static void SetImageScalarParameter(UImage* InImage, FName ParamName, float Value);

	UFUNCTION(BlueprintCallable, Category = "RPGSystem|UI|Material")
	static void SetImageTextureParameter(UImage* InImage, FName ParamName, UTexture* Value);

	UFUNCTION(BlueprintCallable, Category = "RPGSystem|UI|Material")
	static void SetImageVectorParameter(UImage* InImage, FName ParamName, FLinearColor Value);

	UFUNCTION(BlueprintCallable, Category = "RPGSystem|UI|Transform")
	static void SetWidgetRenderAngle(UWidget* InWidget, float Angle);

	UFUNCTION(BlueprintCallable, Category = "RPGSystem|UI|Transform")
	static void SetWidgetRenderTranslation(UWidget* InWidget, FVector2D Translation);

	UFUNCTION(BlueprintCallable, Category = "RPGSystem|UI|Transform")
	static void SetWidgetRenderScale(UWidget* InWidget, FVector2D Scale);
	
	UFUNCTION(BlueprintCallable, Category = "RPGSystem|UI|Helper", meta = (DeterminesOutputType = "ParentClass", DynamicOutputParam = "FoundWidget"))
	static void FindParentWidgetByClass(UWidget* InWidget, TSubclassOf<UUserWidget> ParentClass, UUserWidget*& FoundWidget);
	
	UFUNCTION(BlueprintCallable, Category = "RPGSystem|UI|Helper")
	static void SetWidgetVisibilityByBool(UWidget* InWidget, bool bVisible, ESlateVisibility TrueVisibility = ESlateVisibility::SelfHitTestInvisible, ESlateVisibility FalseVisibility = ESlateVisibility::Collapsed);
	
	UFUNCTION(BlueprintCallable, Category = "RPGSystem|UI|World", meta = (WorldContext = "WorldContextObject"))
	static bool GetScreenPositionForWorldLocation(const UObject* WorldContextObject, FVector WorldLocation, FVector2D& OutScreenPosition, FVector2D ScreenOffset = FVector2D::ZeroVector, bool bPlayerViewportRelative = true);
	
	UFUNCTION(BlueprintCallable, Category = "RPGSystem|UI|Texture")
	static UTexture2D* LoadTextureFromSoftPtr(TSoftObjectPtr<UTexture2D> SoftTexture);
};


void ModifyProgressBarFillImage(UProgressBar* InProgressBar, TFunction<void(FSlateBrush&)> ModifyFunc);
void ModifyProgressBarMarquee(UProgressBar* InProgressBar, TFunction<void(FSlateBrush&)> ModifyFunc);
void ModifyProgressBarBackgroundImage(UProgressBar* InProgressBar, TFunction<void(FSlateBrush&)> ModifyFunc);

// UFUNCTION(BlueprintCallable, Category = "Global")
// static FTimerHandle DelayedExecute(
// UObject* WorldContextObject,
// TFunction<void()> Callback,
// float Delay = 1.0f
// );

// template<typename... Args>
// static FTimerHandle DelayedExecuteWithParams(
// 	UObject* WorldContextObject,
// 	TFunction<void(Args...)> Callback,
// 	float Delay,
// 	Args... Params
// )
// {
// 	if (!WorldContextObject) return FTimerHandle();
//         
// 	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
// 	if (!World) return FTimerHandle();
//         
// 	FTimerHandle Handle;
// 	World->GetTimerManager().SetTimer(Handle, [Callback, Params...]()
// 	{
// 		Callback(Params...);
// 	}, Delay, false);
//         
// 	return Handle;
// }
