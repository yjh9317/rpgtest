// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/WorldItem.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/ActorChannel.h"
#include "Interaction/InteractableComponent.h"
#include "Inventory/InventoryCoreComponent.h"
#include "Item/Data/ItemDefinition.h"
#include "Item/Data/ItemInstance.h"
#include "Net/UnrealNetwork.h"

AWorldItem::AWorldItem()
{
    // 1. [물리 담당] 메쉬를 루트로 설정
    // 역할: 바닥에 예쁘게 떨어지기, 발에 차이기
    StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
    RootComponent = StaticMeshComponent;
    
    StaticMeshComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
    StaticMeshComponent->SetSimulatePhysics(true);
    StaticMeshComponent->SetEnableGravity(true);
    StaticMeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    StaticMeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

    SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    SkeletalMeshComponent->SetupAttachment(RootComponent);
    SkeletalMeshComponent->SetCollisionProfileName(TEXT("NoCollision"));
    SkeletalMeshComponent->SetVisibility(false);
    
    InteractionCollision = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionCollision"));
    InteractionCollision->SetupAttachment(StaticMeshComponent);
    InteractionCollision->SetSphereRadius(SphereRadius);
   
    InteractionCollision->SetSimulatePhysics(false); 
    InteractionCollision->SetCollisionProfileName(TEXT("InteractableItem"));
    
    InteractionWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionWidget"));
    InteractionWidget->SetupAttachment(RootComponent);
    
    InteractableComponent = CreateDefaultSubobject<UInteractableComponent>(TEXT("InteractableComponent"));

    InteractionWidget->SetWidgetSpace(EWidgetSpace::Screen);
    InteractionWidget->SetDrawAtDesiredSize(true);
    InteractionWidget->SetVisibility(false);
    
    if (StaticMeshComponent)
    {
        StaticMeshComponent->SetCustomDepthStencilValue(StencilValue);
    }

    if (SkeletalMeshComponent)
    {
        SkeletalMeshComponent->SetCustomDepthStencilValue(StencilValue);
    }
}

void AWorldItem::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    // 게임 시작 전(에디터 상태)일 때만 Definition 기반 업데이트 수행
    // (게임 중에는 BeginPlay에서 Instance 기반으로 업데이트하므로)
    if (!IsPostLoadThreadSafe() && !GetWorld()->IsGameWorld()) 
    {
#if WITH_EDITOR
        UpdateVisualsFromDefinition();
#endif
    }
}

void AWorldItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AWorldItem, ItemInstance);
}

// UObject인 ItemInstance를 복제하기 위해 필수
bool AWorldItem::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
    bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

    if (ItemInstance)
    {
        // ItemInstance 내부의 Fragment들까지 복제 처리됨 (ItemInstance 구현 참고)
        bWroteSomething |= Channel->ReplicateSubobject(ItemInstance, *Bunch, *RepFlags);
    }

    return bWroteSomething;
}

void AWorldItem::BeginPlay()
{
    Super::BeginPlay();

    // 에디터에 배치된 액터라면 DefaultItemDef로 인스턴스 생성
    if (HasAuthority() && !ItemInstance && DefaultItemDef.IsValid())
    {
        UItemDefinition* LoadedDef = DefaultItemDef.LoadSynchronous();
        if (LoadedDef)
        {
            // 임시로 Owner를 this로 하여 생성 (나중에 인벤토리에 들어갈 때 변경됨)
            UItemInstance* NewInstance = LoadedDef->CreateInstance(this);
            // 수량 처리 로직 필요 (Instance에는 수량이 없고 Slot에 있음. 
            // 만약 WorldItem이 수량을 가져야 한다면 별도 변수 관리 필요)
            
            InitializeItem(NewInstance);
        }
    }
    
    if (InteractableComponent)
    {
        // HighlightableObjects 세트 구성 (아웃라인 효과 줄 메쉬들)
        TSet<UPrimitiveComponent*> HighlightObjs;
        if (StaticMeshComponent) HighlightObjs.Add(StaticMeshComponent);
        if (SkeletalMeshComponent) HighlightObjs.Add(SkeletalMeshComponent);
        
        InteractableComponent->SetupInteractableReferences(
            InteractionCollision,
            InteractionWidget,  
            HighlightObjs       
        );
    }
    
    if (InteractionWidget && InteractionWidgetClass)
    {
        InteractionWidget->SetWidgetClass(InteractionWidgetClass);
    }
}

void AWorldItem::Initialize()
{
    IInteractableInterface::Initialize();
    
    if (InteractionWidget)
    {
        InteractionWidget->SetVisibility(false);
    }
}

void AWorldItem::Interaction(AActor* Interactor)
{
    if (!HasAuthority() || !Interactor || !ItemInstance) return;

    UInventoryCoreComponent* InventoryComp = Interactor->FindComponentByClass<UInventoryCoreComponent>();
    
    if (InventoryComp && ItemInstance)
    {
        bool bSuccess = InventoryComp->LootItemInstance(ItemInstance);

        if (bSuccess)
        {
            ItemInstance = nullptr;
            Destroy(); 
        }
        else
        {
            // 인벤토리가 꽉 찼거나 하는 등의 실패 처리 (알림 메시지 등)
        }
    }
}

void AWorldItem::ClientBeginInteraction(AActor* Interactor)
{
    if (InteractionWidget)
    {
        InteractionWidget->SetVisibility(true);
    }

    if (StaticMeshComponent)
    {
        StaticMeshComponent->SetRenderCustomDepth(true);
    }
}

void AWorldItem::ClientEndInteraction(AActor* Interactor)
{
    if (InteractionWidget)
    {
        InteractionWidget->SetVisibility(false);
    }

    if (StaticMeshComponent)
    {
        StaticMeshComponent->SetRenderCustomDepth(false);
    }
}

void AWorldItem::InitializeItem(UItemInstance* InInstance)
{
    if (!InInstance) return;

    ItemInstance = InInstance;
    
    // 인스턴스의 Outer를 이 액터로 변경하여 GC 방지 및 복제 계층 설정
    // (이미 생성 시 설정했다면 생략 가능)
    if (HasAuthority())
    {
        ItemInstance->Rename(nullptr, this);
    }

    // 서버에서도 시각적 업데이트 수행
    UpdateVisuals();
    
    // 클라이언트 복제 트리거
    OnRep_ItemInstance(); 
    
    // 메쉬가 변경된 후 물리 설정을 다시 확실하게 적용
    if (StaticMeshComponent)
    {
        // 1. 물리 시뮬레이션 활성화
        StaticMeshComponent->SetSimulatePhysics(true);
        
        // 2. 충돌 채널 설정 (Old 코드 참고)
        // Pawn과는 충돌하되 밀리지 않게 하거나, 
        // 작은 아이템이라면 Pawn을 무시(Ignore)하고 밟고 지나가게 설정하는 것이 좋습니다.
        StaticMeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore); 
        StaticMeshComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
        StaticMeshComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
    }
}

void AWorldItem::OnRep_ItemInstance()
{
    UpdateVisuals();
}

void AWorldItem::UpdateVisuals()
{
    if (!ItemInstance || !ItemInstance->GetItemDef())
    {
        return;
    }
    
    if (!StaticMeshComponent || !SkeletalMeshComponent)
        {
            return;
        }

    const UItemDefinition* Def = ItemInstance->GetItemDef();

    // 1. Static Mesh 확인 (New ItemDefinition에 추가된 필드 활용)
    if (!Def->WorldMesh.IsNull())
    {
        // 비동기 로딩 권장하지만 예시를 위해 로드
        UStaticMesh* LoadedMesh = Def->WorldMesh.LoadSynchronous(); 
        if (LoadedMesh)
        {
            StaticMeshComponent->SetStaticMesh(LoadedMesh);
            StaticMeshComponent->SetVisibility(true);
            SkeletalMeshComponent->SetVisibility(false);
            return;
        }
    }

    // 2. Skeletal Mesh 확인
    if (!Def->SkeletalMesh.IsNull())
    {
        USkeletalMesh* LoadedMesh = Def->SkeletalMesh.LoadSynchronous();
        if (LoadedMesh)
        {
            SkeletalMeshComponent->SetSkeletalMesh(LoadedMesh);
            SkeletalMeshComponent->SetVisibility(true);
            StaticMeshComponent->SetVisibility(false);
            return;
        }
    }
}


#if WITH_EDITOR

void AWorldItem::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    // 변경된 프로퍼티 이름 가져오기
    FName PropertyName = (PropertyChangedEvent.Property != nullptr) 
        ? PropertyChangedEvent.Property->GetFName() 
        : NAME_None;

    // DefaultItemDef가 변경되었는지 확인
    if (PropertyName == GET_MEMBER_NAME_CHECKED(AWorldItem, DefaultItemDef))
    {
        UpdateVisualsFromDefinition();
    }
}

void AWorldItem::UpdateVisualsFromDefinition()
{
    // DefaultItemDef가 비어있으면 메쉬 초기화 후 리턴
    if (DefaultItemDef.IsNull())
    {
        if (StaticMeshComponent) StaticMeshComponent->SetStaticMesh(nullptr);
        if (SkeletalMeshComponent) SkeletalMeshComponent->SetSkeletalMesh(nullptr);
        return;
    }

    // Definition 에셋 로드 (에디터이므로 동기 로딩)
    UItemDefinition* Def = DefaultItemDef.LoadSynchronous();
    if (!Def) return;

    // 1. Static Mesh 확인 및 적용
    if (!Def->WorldMesh.IsNull())
    {
        UStaticMesh* LoadedMesh = Def->WorldMesh.LoadSynchronous();
        if (LoadedMesh && StaticMeshComponent)
        {
            StaticMeshComponent->SetStaticMesh(LoadedMesh);
            StaticMeshComponent->SetVisibility(true);
            
            if (SkeletalMeshComponent) SkeletalMeshComponent->SetVisibility(false);
        }
    }
    // 2. Skeletal Mesh 확인 및 적용
    else if (!Def->SkeletalMesh.IsNull())
    {
        USkeletalMesh* LoadedMesh = Def->SkeletalMesh.LoadSynchronous();
        if (LoadedMesh && SkeletalMeshComponent)
        {
            SkeletalMeshComponent->SetSkeletalMesh(LoadedMesh);
            SkeletalMeshComponent->SetVisibility(true);
            
            if (StaticMeshComponent) StaticMeshComponent->SetVisibility(false);
        }
    }
}

#endif