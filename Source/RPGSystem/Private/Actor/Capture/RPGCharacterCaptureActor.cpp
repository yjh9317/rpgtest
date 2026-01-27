// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Capture/RPGCharacterCaptureActor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/WindDirectionalSourceComponent.h"

ARPGCharacterCaptureActor::ARPGCharacterCaptureActor()
{
	PrimaryActorTick.bCanEverTick = false; // 틱 불필요 (성능 최적화)

	// 1. 루트 컴포넌트 설정
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// 2. 더미 메쉬 설정
	PreviewMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PreviewMesh"));
	PreviewMesh->SetupAttachment(RootComponent);
	PreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewMesh->SetCastShadow(true);

	// 3. 캡처 카메라 설정
	CaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("CaptureComponent"));
	CaptureComponent->SetupAttachment(RootComponent);
	// 위치는 블루프린트에서 미세조정하겠지만, 대략 정면 배치
	CaptureComponent->SetRelativeLocation(FVector(200.0f, 0.0f, 100.0f)); 
	CaptureComponent->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));

	// [중요] 캡처 최적화 설정
	// "이 액터(Self)만 찍겠다"는 옵션을 켜야 월드의 다른 물체가 안 찍힘
	CaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	CaptureComponent->CaptureSource = ESceneCaptureSource::SCS_SceneColorSceneDepth;
	CaptureComponent->bCaptureEveryFrame = false;
	
	WindDirectionalSourceComponent = CreateDefaultSubobject<UWindDirectionalSourceComponent>(TEXT("WindDirectionalSourceComponent"));
	WindDirectionalSourceComponent->bAutoActivate = false; 
	WindDirectionalSourceComponent->Strength = 1.0f;
	WindDirectionalSourceComponent->Speed = 1.0f;
	
	WindTextRenderComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("WindTextRenderComponent"));
	// 4. 조명 설정
	// KeyLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("KeyLight"));
	// KeyLight->SetupAttachment(RootComponent);
	// KeyLight->SetIntensity(5000.0f);
	// KeyLight->SetRelativeLocation(FVector(150.0f, 100.0f, 200.0f));
	// KeyLight->SetRelativeRotation(FRotator(-45.0f, 135.0f, 0.0f));
	//
	// FillLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("FillLight"));
	// FillLight->SetupAttachment(RootComponent);
	// FillLight->SetIntensity(2000.0f);
	// FillLight->SetRelativeLocation(FVector(150.0f, -100.0f, 100.0f));
}

void ARPGCharacterCaptureActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ARPGCharacterCaptureActor::StartWindComponent()
{
	WindDirectionalSourceComponent->bAutoActivate = true;
	WindTextRenderComponent->SetRelativeTransform(WindTextRenderComponent->GetRelativeTransform());
	WindDirectionalSourceComponent->Strength = WindStrength;
	WindDirectionalSourceComponent->Speed = WindSpeed;
}

void ARPGCharacterCaptureActor::EndWindComponent()
{
	WindDirectionalSourceComponent->bAutoActivate = false;
}

void ARPGCharacterCaptureActor::StartCapture()
{
	CaptureComponent->bCaptureEveryFrame = true;
	PreviewMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	
}

void ARPGCharacterCaptureActor::EndCapture()
{
	CaptureComponent->bCaptureEveryFrame = false;
	PreviewMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
}

void ARPGCharacterCaptureActor::AddCaptureRotation(FRotator InRotator)
{
	FRotator NewRot(0.f,0.f,InRotator.Yaw * -1);
	PreviewMesh->AddLocalRotation(NewRot);
}

void ARPGCharacterCaptureActor::UpdateAppearance(USkeletalMesh* NewMesh, UMaterialInterface* NewMaterial)
{
	if (PreviewMesh && NewMesh)
	{
		PreviewMesh->SetSkeletalMesh(NewMesh);
		
		if (NewMaterial)
		{
			PreviewMesh->SetMaterial(0, NewMaterial);
		}

		// 캡처 컴포넌트가 "나 자신(더미)"만 찍도록 리스트에 추가
		CaptureComponent->ShowOnlyActors.Empty();
		CaptureComponent->ShowOnlyActors.Add(this);
	}
}
