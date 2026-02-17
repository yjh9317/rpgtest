// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/Components/QuestGiverComponent.h"

#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Pawn.h"
#include "Player/RPGPlayerController.h"
#include "Quest/Components/QuestManagerComponent.h"
#include "Quest/UI/QuestMarkerWidget.h"

UQuestGiverComponent::UQuestGiverComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
}

void UQuestGiverComponent::OnQuestInteracted(AActor* Interactor)
{
	if (!QuestManagerComponent)
	{
		return;
	}

	ARPGPlayerController* PlayerController = nullptr;
	if (APawn* InteractorPawn = Cast<APawn>(Interactor))
	{
		PlayerController = Cast<ARPGPlayerController>(InteractorPawn->GetController());
	}
	else
	{
		PlayerController = Cast<ARPGPlayerController>(Interactor);
	}

	// Priority 1: turn in rewards for quests that are already valid.
	for (const TPair<int, FQuestGiverEntry>& Entry : QuestList)
	{
		if (!Entry.Value.bIsQuestReceiver)
		{
			continue;
		}

		if (URPGQuest* Quest = QuestManagerComponent->QueryQuest(Entry.Key))
		{
			if (Quest->QuestState == EQuestState::Valid && QuestManagerComponent->CompleteQuestByID(Entry.Key))
			{
				UpdateQuestMarker();
				return;
			}
		}
	}

	// Priority 2: accept the first quest that can be started.
	for (const TPair<int, FQuestGiverEntry>& Entry : QuestList)
	{
		if (!Entry.Value.bIsQuestGiver)
		{
			continue;
		}

		if (QuestManagerComponent->AcceptQuestByID(Entry.Key, PlayerController))
		{
			BindFunctionsToQuestDelegates();
			UpdateQuestMarker();
			return;
		}
	}
}

void UQuestGiverComponent::BeginPlay()
{
	Super::BeginPlay();
	Owner = GetOwner();
#if WITH_EDITOR
	/* Return if the play mode is Simulate */
	if (GetWorld()->WorldType == EWorldType::PIE && GEditor->IsSimulateInEditorInProgress())
		return;
#endif

	GetWorld()->OnWorldBeginPlay.AddUObject(this, &UQuestGiverComponent::LateBeginPlay);

	RemoveVisualMarker();
	
}

void UQuestGiverComponent::LateBeginPlay()
{
	/* Get the local Player */
	APawn* localPlayer = GetWorld()->GetFirstPlayerController()->GetPawn();
	if (!localPlayer)
		return;

	/* Get the quest manager */
	QuestManagerComponent = localPlayer->FindComponentByClass<UQuestManagerComponent>();

	if (QuestMarkerClass)
		CreateQuestMarkerWidget();

	BindFunctionsToQuestDelegates();
}


void UQuestGiverComponent::OnQuestStateChangedWrapper(URPGQuest* QuestUpdate, EQuestState QuestState)
{
	UpdateQuestMarker();
    
	// Blueprint Event 호출
	if (OnQuestStateChanged.IsBound())
	{
		OnQuestStateChanged.Broadcast(QuestState);
	}

	/* 완료된 퀘스트(Archive)는 델리게이트 해제 */
	if (QuestState == EQuestState::Archive)
	{
		if (QuestUpdate && QuestUpdate->QuestData)
		{
			// 데일리/위클리가 아니면 해제
			if (QuestUpdate->QuestData->QuestType != EQuestType::Daily &&
				QuestUpdate->QuestData->QuestType != EQuestType::Weekly)
			{
				QuestUpdate->OnQuestStateChanged.RemoveDynamic(this, &UQuestGiverComponent::OnQuestStateChangedWrapper);
			}
		}
	}
}

void UQuestGiverComponent::BindFunctionsToQuestDelegates()
{
	if (!QuestManagerComponent) return;

	for (auto& Elem : QuestList)
	{
		int32 QuestID = Elem.Key;
       
		// 현재 활성화된(Active) 퀘스트 인스턴스가 있는지 확인
		URPGQuest* ActiveQuest = QuestManagerComponent->QueryQuest(QuestID);

		if (ActiveQuest)
		{
			// 델리게이트 이름 변경: QuestStateChangedDelegate -> OnQuestStateChanged
			if (!ActiveQuest->OnQuestStateChanged.IsAlreadyBound(this, &UQuestGiverComponent::OnQuestStateChangedWrapper))
			{
				ActiveQuest->OnQuestStateChanged.AddDynamic(this, &UQuestGiverComponent::OnQuestStateChangedWrapper);
			}
		}
       
		// [삭제] bIsRequirementMet 체크 로직 삭제
		// [삭제] QuestRequirementMetDelegate 바인딩 삭제
		// 이유는 이제 QuestManager가 수락 가능 여부를 판단하기 때문입니다.
	}
    
	UpdateQuestMarker();
}

void UQuestGiverComponent::SetQuestMarker(bool isMarkerVisible, bool isQuestValid, EQuestType QuestState)
{
	/* If no quest to show, set the component to be silent */
	if (isMarkerVisible)
		bIsComponentSilent = false;
	else
		bIsComponentSilent = true;

	OnSleepStateChanged.Broadcast(bIsComponentSilent);

	if (!QuestMarkerClass)
		return;

	UQuestMarkerWidget* widget = Cast<UQuestMarkerWidget>(QuestMarkerWidget->GetWidget());

	/* Update the quest marker */
	if (widget)
	{
		QuestMarkerWidget->SetVisibility(isMarkerVisible);
		widget->SetImageQuest(isQuestValid, QuestState);
	}
}

void UQuestGiverComponent::UpdateQuestMarker()
{
    if (!QuestMarkerClass || !QuestManagerComponent) return;

    if (QuestList.Num() == 0)
    {
       SetQuestMarker(false, false, EQuestType::MainQuest); // Enum 기본값 추가
       return;
    }

    bool bIsAnyQuestValid = false;
    bool bIsAnyQuestPending = false;
    
    // 우선순위 계산을 위해 임시 저장
    EQuestType ValidType = EQuestType::MainQuest; 
    EQuestType PendingType = EQuestType::MainQuest;

    for (auto& Elem : QuestList)
    {
       int32 QuestID = Elem.Key;
       FQuestGiverEntry& Config = Elem.Value;

       // 1. 이미 받은 퀘스트인지 확인 (인스턴스 조회)
       URPGQuest* ActiveQuest = QuestManagerComponent->QueryQuest(QuestID);

       // [Case 1: 완료 보고 가능?] (Valid)
       if (ActiveQuest && Config.bIsQuestReceiver)
       {
           if (ActiveQuest->QuestState == EQuestState::Valid)
           {
               bIsAnyQuestValid = true;
               // 타입 우선순위 체크 (메인 > 서브)
               if (ActiveQuest->QuestData) ValidType = ActiveQuest->QuestData->QuestType;
               continue; // 완료 가능한 게 있으면 이게 최우선
           }
       }

       // [Case 2: 퀘스트 수락 가능?] (Pending)
       // ActiveQuest가 없거나(아직 안 받음) Pending 상태여야 함
       if (Config.bIsQuestGiver && !bIsAnyQuestValid)
       {
           // 퀘스트 데이터를 가져옴 (QuestManager에 데이터 조회 함수가 필요함 via ID)
           // 만약 없다면 QuestList의 Key를 ID가 아니라 URPGQuestData*로 바꾸는 게 훨씬 편합니다.
           const URPGQuestData* Data = QuestManagerComponent->GetQuestDataByID(QuestID); 
       	
           if (Data)
           {
               // [핵심 변경] bIsRequirementMet 대신 CanAcceptQuest 사용
               ARPGPlayerController* LocalPC = Cast<ARPGPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
               if (QuestManagerComponent->CanAcceptQuest(Data, LocalPC))
               {
                   bIsAnyQuestPending = true;
                   PendingType = Data->QuestType;
               }
           }
       }
    }

    /* 마커 설정 로직 */
    if (bIsAnyQuestValid)
    {
       SetQuestMarker(true, true, ValidType);
    }
    else if (bIsAnyQuestPending)
    {
       SetQuestMarker(true, false, PendingType);
    }
    else
    {
       SetQuestMarker(false, false, EQuestType::MainQuest);
    }
}

void UQuestGiverComponent::CheckQuestTypes(TArray<URPGQuest*>& ValidQuests, EQuestType& currentType)
{
	for (auto quests : ValidQuests)
	{
		switch (quests->QuestData->QuestType)
		{
		case EQuestType::MainQuest:
			currentType = EQuestType::MainQuest;
			return;

		case EQuestType::SubQuest:
			currentType = EQuestType::SubQuest;
			break;

		case EQuestType::Daily:
		case EQuestType::Weekly:
			{
				if (currentType != EQuestType::SubQuest)
					currentType = quests->QuestData->QuestType;
				break;
			}
		}
	}
}

void UQuestGiverComponent::ShowVisualMarker()
{
	if(VisualWidget) return;
	
	TArray<AActor*> TempArray = { Owner };
	FVector OutCenter{}, OutBoxExtent{};
	UGameplayStatics::GetActorArrayBounds(TempArray,true,OutCenter,OutBoxExtent);
	float ZExtend = OutBoxExtent.Z;

}

void UQuestGiverComponent::RemoveVisualMarker()
{
	if(!VisualWidget)
	{
		VisualWidget = nullptr;
		RerunScript();
	}
}

void UQuestGiverComponent::CreateQuestMarkerWidget()
{
	if(!Owner) return;
	
	FVector origin, extent;
	Owner->GetActorBounds(true, origin, extent);


	QuestMarkerWidget = Cast<UWidgetComponent>(GetOwner()->AddComponentByClass(UWidgetComponent::StaticClass(), false, FTransform(), false));

	// Check if the WidgetComponent was successfully added
	if (QuestMarkerWidget)
	{
		QuestMarkerWidget->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
		QuestMarkerWidget->SetWidgetClass(QuestMarkerClass);
		QuestMarkerWidget->SetMaterial(0, QuestMarkerMaterial);
		QuestMarkerWidget->SetWidgetSpace(EWidgetSpace::World);
		QuestMarkerWidget->RegisterComponent();
		QuestMarkerWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		float zCoord = extent.Z * 2 + zOffset;
		QuestMarkerWidget->SetRelativeLocation(FVector(0, 0, zCoord));
		QuestMarkerWidget->SetDrawSize(FVector2D(512.f));
		QuestMarkerWidget->SetWorldScale3D(FVector(.2f));
	}

	SetQuestMarker(false, false);
}

void UQuestGiverComponent::MarkerFloatingMovement(float DeltaTime)
{
	if (!CameraManager)
		CameraManager = GetWorld()->GetFirstLocalPlayerFromController()->PlayerController->PlayerCameraManager;

	if (CameraManager)
	{
		CurrentDelta += DeltaTime;

		float floatingEffect = sin(CurrentDelta * FloatingSpeed) * FloatingAmplitude;

		QuestMarkerWidget->SetWorldRotation(UKismetMathLibrary::FindLookAtRotation(QuestMarkerWidget->GetComponentLocation(), CameraManager->GetCameraLocation()));
		QuestMarkerWidget->AddWorldOffset(FVector(0, 0, floatingEffect));
	}
}


void UQuestGiverComponent::RerunScript()
{
#if WITH_EDITOR
	AActor* Actor = GetOwner();
	Actor->RerunConstructionScripts();
#endif
}

