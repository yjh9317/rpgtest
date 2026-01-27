// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Capture/MinimapCaptureActor.h"
#include "Components/BoxComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "DrawDebugHelpers.h"

#if WITH_EDITOR
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Factories/TextureFactory.h"
#include "PackageTools.h"
#endif

AMinimapCaptureActor::AMinimapCaptureActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// 기본 경로 및 이름 설정
	SaveFolderPath = TEXT("/Game/Textures/Minimap/");
	FileName = TEXT("T_NewMinimap");
	bDrawDebugBounds = true;
	DebugLineThickness = 5.0f;
	DebugColor = FColor::Green;

	// 캡처 컴포넌트 가져오기 (ASceneCapture2D에 이미 존재함)
	GetCaptureComponent2D()->ProjectionType = ECameraProjectionMode::Orthographic;
	GetCaptureComponent2D()->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	
	// 카메라는 아래를 바라보도록 회전 (Top-Down)
	GetCaptureComponent2D()->SetRelativeRotation(FRotator(-90.0f, 0.0f, -90.0f));

	// 영역 박스 컴포넌트 설정
	CaptureBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("CaptureBounds"));
	CaptureBounds->SetupAttachment(RootComponent);
	CaptureBounds->SetBoxExtent(FVector(1000.0f, 1000.0f, 100.0f));
	CaptureBounds->SetCollisionProfileName(TEXT("NoCollision"));
}

void AMinimapCaptureActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	UpdateCaptureComponentSettings();
}

void AMinimapCaptureActor::BeginPlay()
{
	Super::BeginPlay();
	// 게임 시작 시에는 굳이 틱을 돌릴 필요가 없다면 꺼두셔도 됩니다.
	// SetActorTickEnabled(false); 
}

void AMinimapCaptureActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

#if WITH_EDITOR
	// 에디터 뷰포트에서 범위 확인용 디버그 박스 그리기
	if (bDrawDebugBounds && CaptureBounds)
	{
		DrawDebugBox(
			GetWorld(),
			CaptureBounds->GetComponentLocation(),
			CaptureBounds->GetScaledBoxExtent(),
			CaptureBounds->GetComponentQuat(),
			DebugColor,
			false,
			-1.0f,
			0,
			DebugLineThickness
		);
	}
#endif
}

void AMinimapCaptureActor::UpdateCaptureComponentSettings()
{
	if (USceneCaptureComponent2D* CaptureComp = GetCaptureComponent2D())
	{
		// 박스의 X축 크기(Extent * 2)를 OrthoWidth로 설정하여 영역 동기화
		// Orthographic 카메라는 Width만 설정하면 비율에 따라 Height가 결정됨
		float OrthoWidth = CaptureBounds->GetScaledBoxExtent().X * 2.0f;
		CaptureComp->OrthoWidth = OrthoWidth;
	}
}

void AMinimapCaptureActor::CaptureAndSaveMinimap()
{
#if WITH_EDITOR
	USceneCaptureComponent2D* CaptureComp = GetCaptureComponent2D();
	if (!CaptureComp || !CaptureComp->TextureTarget)
	{
		UE_LOG(LogTemp, Error, TEXT("CaptureComponent가 없거나 RenderTarget이 설정되지 않았습니다."));
		return;
	}

	// 1. 씬 캡처 수행
	CaptureComp->CaptureScene();

	// 2. 패키지 경로 생성
	FString PackageName = SaveFolderPath;
	if (!PackageName.EndsWith(TEXT("/")))
	{
		PackageName += TEXT("/");
	}
	PackageName += FileName;

	// 패키지 생성
	UPackage* Package = CreatePackage(*PackageName);
	if (!Package) return;
	Package->FullyLoad();

	// 3. 텍스처 생성
	UTexture2D* NewTexture = CaptureComp->TextureTarget->ConstructTexture2D(
		Package,
		FileName,
		CaptureComp->TextureTarget->GetMaskedFlags(),
		CTF_Default,
		nullptr
	);
	
	if (NewTexture)
	{
		// [중요 수정] UI용 텍스처 설정
		NewTexture->CompressionSettings = TextureCompressionSettings::TC_EditorIcon; // UI 전용 압축 (투명도 및 색감 유지)
		NewTexture->LODGroup = TextureGroup::TEXTUREGROUP_UI; // 밉맵 자동 생성 방지 및 UI 그룹 설정
		NewTexture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps; 
		NewTexture->SRGB = true; // UI는 sRGB 사용
		
		// 변경사항 적용
		NewTexture->UpdateResource();

		// 4. 에셋 저장 및 알림
		FAssetRegistryModule::AssetCreated(NewTexture);
		Package->MarkPackageDirty();
		
		// [선택 사항] 바로 파일(.uasset)로 저장하고 싶다면 아래 주석 해제 (에디터 멈춤 현상 발생 가능)
		/*
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		UPackage::SavePackage(Package, NewTexture, *FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension()), SaveArgs);
		*/

		UE_LOG(LogTemp, Log, TEXT("미니맵 텍스처 저장 성공 (UI 설정 적용됨): %s"), *PackageName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("텍스처 생성 실패!"));
	}
#else
	UE_LOG(LogTemp, Warning, TEXT("이 기능은 에디터에서만 사용할 수 있습니다."));
#endif
}