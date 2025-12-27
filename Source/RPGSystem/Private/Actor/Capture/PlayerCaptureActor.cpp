// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Capture/PlayerCaptureActor.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Components/SpotLightComponent.h"

// Sets default values
APlayerCaptureActor::APlayerCaptureActor()
{
	PrimaryActorTick.bCanEverTick = false;

	DefaultScene = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultScene"));
	SetRootComponent(DefaultScene);

	RootOfCharacter = CreateDefaultSubobject<USceneComponent>(TEXT("RootOfCharacter"));
	RootOfCharacter->SetupAttachment(GetRootComponent());

	RootSkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RootSkeletalMesh"));
	RootSkeletalMeshComponent->SetupAttachment(RootOfCharacter);

	BootsMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Boots"));
	BootsMeshComponent->SetupAttachment(RootSkeletalMeshComponent);
	LegsMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Legs"));
	LegsMeshComponent->SetupAttachment(RootSkeletalMeshComponent);
	GlovesMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Gloves"));
	GlovesMeshComponent->SetupAttachment(RootSkeletalMeshComponent);
	ChestMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Chest"));
	ChestMeshComponent->SetupAttachment(RootSkeletalMeshComponent);
	HeadMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Head"));
	HeadMeshComponent->SetupAttachment(RootSkeletalMeshComponent);

	SceneCaptureComponent2D = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureComponent2D"));
	SceneCaptureComponent2D->SetupAttachment(GetRootComponent());
	SceneCaptureComponent2D->ShowOnlyActorComponents(this, true);
	
	SpotLightComponent = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotLightComponent"));
	SpotLightComponent->SetupAttachment(GetRootComponent());
}
//
// void APlayerCaptureActor::AttachItem(const FItemData& InItemData)
// {
// 	if (!UInventoryUtilityLibrary::IsItemClassValid(InItemData) || !InItemData.ItemClass) return;
// 	
// 	UItemBase* ItemBase = InItemData.ItemClass.GetDefaultObject();
// 	if(!ItemBase)
// 	{
// 		return;
// 	}
// 	EItemSlot LocalItemSlot{InItemData.EquipmentSlot};
// 	TSoftClassPtr<ABaseEquippable> EquippableItem{ItemBase->EquippableClass};
// 	TSoftObjectPtr<USkeletalMesh> LocalSkeletalMesh{ItemBase->SkeletalMesh};
//
// 	USkeletalMesh* LoadedSkeletalMesh = LocalSkeletalMesh.LoadSynchronous();
// 	switch (LocalItemSlot)
// 	{
// 	// Armor(Skeletal Mesh)
// 	case EItemSlot::HEAD:
// 		if (nullptr == LoadedSkeletalMesh) return;
// 		HeadMeshComponent->SetSkinnedAssetAndUpdate(LoadedSkeletalMesh, true);
// 		HeadMeshComponent->SetLeaderPoseComponent(RootSkeletalMeshComponent, true, false);
// 		PrestreamTextures(0.f, true, 0.f);
// 		break;
// 	case EItemSlot::GLOVES:
// 		if (nullptr == LoadedSkeletalMesh) return;
// 		GlovesMeshComponent->SetSkinnedAssetAndUpdate(LoadedSkeletalMesh, true);
// 		GlovesMeshComponent->SetLeaderPoseComponent(RootSkeletalMeshComponent, true, false);
// 		PrestreamTextures(0.f, true, 0.f);
// 		break;
// 	case EItemSlot::CHESTPLATE:
// 		if (nullptr == LoadedSkeletalMesh) return;
// 		ChestMeshComponent->SetSkinnedAssetAndUpdate(LoadedSkeletalMesh, true);
// 		ChestMeshComponent->SetLeaderPoseComponent(RootSkeletalMeshComponent, true, false);
// 		PrestreamTextures(0.f, true, 0.f);
// 		break;
// 	case EItemSlot::LEGS:
// 		if (nullptr == LoadedSkeletalMesh) return;
// 		LegsMeshComponent->SetSkinnedAssetAndUpdate(LoadedSkeletalMesh, true);
// 		LegsMeshComponent->SetLeaderPoseComponent(RootSkeletalMeshComponent, true, false);
// 		PrestreamTextures(0.f, true, 0.f);
// 		break;
// 	case EItemSlot::BOOTS:
// 		if (nullptr == LoadedSkeletalMesh) return;
// 		BootsMeshComponent->SetSkinnedAssetAndUpdate(LoadedSkeletalMesh, true);
// 		BootsMeshComponent->SetLeaderPoseComponent(RootSkeletalMeshComponent, true, false);
// 		PrestreamTextures(0.f, true, 0.f);
// 		break;
// 	// Euippable(Actor)
// 	case EItemSlot::WEAPON:
// 	case EItemSlot::SHIELD:
// 		UClass* LoadedEquippable = EquippableItem.LoadSynchronous();
// 		if (EquippableRef.Contains(LocalItemSlot))
// 		{
// 			ABaseEquippable* CurrentEquippable = EquippableRef[LocalItemSlot];
// 			if (IsValid(CurrentEquippable))
// 			{
// 				CurrentEquippable->Destroy();
// 			}
// 			EquippableRef.Remove(LocalItemSlot);
// 		}
// 		
// 		FTransform SpawnLocationAndRotation = GetActorTransform();
// 		FActorSpawnParameters SpawnParams;
// 		SpawnParams.Owner = this;
// 		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
// 	
// 		ABaseEquippable* NewEquippable = GetWorld()->SpawnActor<ABaseEquippable>(
// 			LoadedEquippable,
// 			SpawnLocationAndRotation,
// 			SpawnParams
// 		);
// 		if(NewEquippable)
// 		{
// 			NewEquippable->ParentMesh = RootSkeletalMeshComponent;
// 			NewEquippable->CurrentSlot = LocalItemSlot;
// 		}
// 		EquippableRef.Add({LocalItemSlot,NewEquippable});
// 		SceneCaptureComponent2D->ShowOnlyActorComponents(NewEquippable,true);
// 		NewEquippable->ToggleCombatMode(true);
// 		break;
// 	}
// }
//
//
// void APlayerCaptureActor::DetachItem(const FItemData& InItemData)
// {
// 	EItemSlot LocalItemSlot = InItemData.EquipmentSlot;
//
// 	switch (LocalItemSlot)
// 	{
// 	case EItemSlot::HEAD:
// 		HeadMeshComponent->SetSkinnedAssetAndUpdate(nullptr,true);
// 	case EItemSlot::LEGS:
// 		LegsMeshComponent->SetSkinnedAssetAndUpdate(nullptr,true);
// 	case EItemSlot::GLOVES:
// 		GlovesMeshComponent->SetSkinnedAssetAndUpdate(nullptr,true);
// 	case EItemSlot::CHESTPLATE:
// 		ChestMeshComponent->SetSkinnedAssetAndUpdate(nullptr,true);
// 	case EItemSlot::BOOTS:
// 		BootsMeshComponent->SetSkinnedAssetAndUpdate(nullptr,true);
// 		break;
// 	case EItemSlot::WEAPON:
// 	case EItemSlot::SHIELD:
// 		if(EquippableRef.Find(LocalItemSlot))
// 		{
// 			EquippableRef[LocalItemSlot]->Destroy();
// 		}
// 		break;
// 	}
// }
//
// void APlayerCaptureActor::StartCapture()
// {
// 	SceneCaptureComponent2D->bCaptureEveryFrame = true;
//
// 	SpawnWindComponent(1.f, 1.f);
// 	RootOfCharacter->SetRelativeRotation(FRotator::ZeroRotator);
// }
//
// void APlayerCaptureActor::EndCapture()
// {
// 	SceneCaptureComponent2D->bCaptureEveryFrame = false;
//
// 	RootOfCharacter->SetRelativeRotation(FRotator::ZeroRotator);
// 	if (WindActor)
// 	{
// 		WindActor->Destroy();
// 	}
// }
//
// void APlayerCaptureActor::AddCaptureRotation(FRotator Rotator)
// {
// 	FRotator LocalRotator(0.f, Rotator.Yaw * -1, 0.f);
// 	RootOfCharacter->AddLocalRotation(LocalRotator);
// }
//
// void APlayerCaptureActor::InitializePlayerCapture(UEquipmentComponent* EquipComp)
// {
// 	if (EquipComp)
// 	{
// 		EquipComp->OnItemAttach.AddUObject(this, &ThisClass::AttachItem);
// 		EquipComp->OnItemDetach.AddUObject(this, &ThisClass::DetachItem);
// 	}
// }
//
// void APlayerCaptureActor::SpawnWindComponent(float WindStrength, float WindSpeed)
// {
// 	if (WindActor) return;
//
// 	FActorSpawnParameters SpawnParams;
// 	SpawnParams.Owner = this;
// 	SpawnParams.Instigator = GetInstigator();
//
// 	FVector SpawnLocation = GetActorLocation();
// 	FRotator SpawnRotation = GetActorRotation();
//
// 	WindActor = GetWorld()->SpawnActor<AWindDirectionalSource>(
// 		AWindDirectionalSource::StaticClass(), // 스폰할 액터의 UClass
// 		SpawnLocation,
// 		SpawnRotation,
// 		SpawnParams
// 	);
//
// 	if (WindActor)
// 	{
// 		WindActor->GetComponent()->SetStrength(WindStrength);
// 		WindActor->GetComponent()->SetSpeed(WindSpeed);
// 	}
// }
