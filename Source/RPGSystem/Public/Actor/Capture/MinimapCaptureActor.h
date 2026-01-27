// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/SceneCapture2D.h"
#include "MinimapCaptureActor.generated.h"

class UBoxComponent;
class UTextureRenderTarget2D;

UCLASS()
class RPGSYSTEM_API AMinimapCaptureActor : public ASceneCapture2D
{
	GENERATED_BODY()
public:
	AMinimapCaptureActor();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;

public:
	// 캡처 영역을 시각적으로 설정하기 위한 박스 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Minimap | Components")
	TObjectPtr<UBoxComponent> CaptureBounds;

	// 저장할 폴더 경로 (예: /Game/Textures/Minimap/)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap | Export")
	FString SaveFolderPath;

	// 저장할 파일 이름 (예: T_Minimap_Level01)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap | Export")
	FString FileName;

	// 디버그 박스 표시 여부
	UPROPERTY(EditAnywhere, Category = "Minimap | Debug")
	bool bDrawDebugBounds;

	// 디버그 박스 선 두께
	UPROPERTY(EditAnywhere, Category = "Minimap | Debug")
	float DebugLineThickness;

	// 디버그 박스 색상
	UPROPERTY(EditAnywhere, Category = "Minimap | Debug")
	FColor DebugColor;

	/**
	 * [에디터 전용] 현재 씬을 캡처하여 정적 텍스처 에셋으로 저장합니다.
	 * 에디터의 디테일 패널 버튼으로 호출 가능합니다.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Minimap | Export")
	void CaptureAndSaveMinimap();

private:
	// 박스 컴포넌트 크기에 맞춰 카메라의 Ortho Width를 업데이트
	void UpdateCaptureComponentSettings();

};
