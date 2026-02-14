
#pragma once

#include "CoreMinimal.h"
#include "MinimapData.h"
#include "Blueprint/UserWidget.h"
#include "MinimapMarkerWidget.generated.h"

class UImage;
class UMinimapWidget;
/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UMinimapMarkerWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	UFUNCTION()
	void MinimapMarkerInit();
	
	UFUNCTION()
	void RemoveMarker();
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exposed")
	UObject* MarkerObject;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exposed")
	UUserWidget* MinimapReference;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exposed")
	float TextureMapSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exposed")
	FVector2D MinMaxScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exposed")
	float ZoomDivider;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exposed")
	FMiniMapInfo MiniMapInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exposed")
	bool bIsFirstPerson;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exposed")
	bool bRotateWithPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exposed")
	float MinimapSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exposed")
	UUserWidget* MinimapDistantMarker;

protected:
	// UPROPERTY(Transient, meta = (BindWidgetAnim))
	// class UWidgetAnimation* Fade;
	//
	// UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	// class UWidget* MarkerIcon;
	//
	// UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	// class UWidget* SB_Marker;
	//
	// UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	// class UWidget* Shine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FVector2D BottomLeftMapPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FVector2D BottomTopRightPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float CanvasMapSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float ZoomValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	bool bScaleWithZoom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float MapAreaTolerance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	bool bIsVisible;
};
