

#include "Shared/Components/ObjectPoolComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Math/Int128.h" 

// 생성자
UObjectPoolComponent::UObjectPoolComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    LastFoundPoolIndex = INDEX_NONE;
    PooledActorMinSpawnCoordinate = DEFAULT_POOL_OBJECT_MIN_POSITION;
    PooledActorMaxSpawnCoordinate = DEFAULT_POOL_OBJECT_MAX_POSITION;
}


void UObjectPoolComponent::BeginPlay()
{
    Super::BeginPlay();
    // 설정된 풀들에 대해 초기 오브젝트를 생성할 수 있습니다. (PrewarmPool과 유사)
    // for (FObjectPoolInstance& PoolInstance : PoolInstances)
    // {
    //     if (PoolInstance.TypeConfig.SpawnSettings.NumToSpawnPerFrame > 0 && PoolInstance.TypeConfig.SpawnSettings.MaxPoolSize > 0)
    //     {
    //          // 초기 생성 로직
    //     }
    // }
}


void UObjectPoolComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    DestroyAllPools();
    Super::EndPlay(EndPlayReason);
}


void UObjectPoolComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdatePools();
}

// --- 풀 설정 추가 함수 구현 ---

void UObjectPoolComponent::AddPoolForUObjectClass(TSubclassOf<UObject> ObjectClass, const FPooledObjectSpawnSettings& Settings)
{
    if (!ObjectClass)
    {
        return;
    }
    FPooledObjectTypeConfig Config;
    Config.ObjectClassToPool = ObjectClass;
    Config.SpawnSettings = Settings;
    AddOrUpdatePoolConfiguration(Config);
}

void UObjectPoolComponent::AddPoolForActorClass(TSubclassOf<AActor> ActorClass, const FPooledObjectSpawnSettings& Settings)
{
    AddPoolForUObjectClass(ActorClass, Settings);
}

void UObjectPoolComponent::AddPoolForActorComponentClass(TSubclassOf<UActorComponent> ComponentClass, const FPooledObjectSpawnSettings& Settings)
{
    AddPoolForUObjectClass(ComponentClass, Settings);
}

void UObjectPoolComponent::AddPoolForMaterialInstanceDynamic(UMaterialInterface* BaseMaterial, const FPooledObjectSpawnSettings& Settings)
{
    if (!BaseMaterial)
    {
        return;
    }
    FPooledObjectTypeConfig Config;
    Config.BaseMaterialInterface = BaseMaterial;
    Config.SpawnSettings = Settings;
    AddOrUpdatePoolConfiguration(Config);
}

void UObjectPoolComponent::AddPoolForUserWidgetClass(TSubclassOf<UUserWidget> WidgetClass, UUserWidget* OwningWidgetOrPlayerController, const FPooledObjectSpawnSettings& Settings)
{
    if (!WidgetClass)
    {
        return;
    }
    
    if (!OwningWidgetOrPlayerController) {
        // return; // 필요에 따라 여기서 반환하거나, 기본 Owner (예: PlayerController)를 사용하도록 수정
    }

    FPooledObjectTypeConfig Config;
    Config.ObjectClassToPool = WidgetClass;
    Config.WidgetOwner = OwningWidgetOrPlayerController;
    Config.SpawnSettings = Settings;
    AddOrUpdatePoolConfiguration(Config);
}


// --- 오브젝트 가져오기 함수 구현 ---

UObject* UObjectPoolComponent::GetObjectFromPool(TSubclassOf<UObject> ObjectClass)
{
    FObjectPoolInstance* PoolInstance = FindPoolInstanceByClass(ObjectClass);
    if (!PoolInstance)
    {
        // 필요하다면 여기서 자동으로 풀을 생성하고 오브젝트를 반환하는 로직 추가 가능
        // FPooledObjectSpawnSettings DefaultSettings;
        // AddPoolForUObjectClass(ObjectClass, DefaultSettings);
        // PoolInstance = FindPoolInstanceByClass(ObjectClass);
        // if (!PoolInstance) return nullptr;
        return nullptr;
    }

    if (!PoolInstance->AvailableObjects.IsEmpty())
    {
        UObject* PooledObject = PoolInstance->AvailableObjects.Pop(false); // Shrink하지 않음
        // 상태 활성화 로직 (예: Actor인 경우 SetActorHiddenInGame(false) 등)은 각 타입별 Get 함수에서 처리
        return PooledObject;
    }

    // 풀에 사용 가능한 오브젝트가 없으면 새로 생성 (풀 최대 크기 제한은 RetrieveOrCreateObjectForPool에서 처리)
    return RetrieveOrCreateObjectForPool(*PoolInstance);
}

AActor* UObjectPoolComponent::GetActorFromPool(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform)
{
    AActor* PooledActor = Cast<AActor>(GetObjectFromPool(ActorClass));
    if (PooledActor)
    {
        ManageActorState(PooledActor, true); // Actor 활성화
        PooledActor->SetActorTransform(SpawnTransform, false, nullptr, ETeleportType::TeleportPhysics);
    }
    return PooledActor;
}

UActorComponent* UObjectPoolComponent::GetActorComponentFromPool(TSubclassOf<UActorComponent> ComponentClass, AActor* NewOwner)
{
    if (!NewOwner)
    {
        return nullptr;
    }
    UActorComponent* PooledComponent = Cast<UActorComponent>(GetObjectFromPool(ComponentClass));
    if (PooledComponent)
    {
        ManageComponentState(PooledComponent, true, NewOwner); // Component 활성화 및 재등록
    }
    return PooledComponent;
}

UUserWidget* UObjectPoolComponent::GetWidgetFromPool(TSubclassOf<UUserWidget> WidgetClass)
{
    UUserWidget* PooledWidget = Cast<UUserWidget>(GetObjectFromPool(WidgetClass));
    if (PooledWidget)
    {
        ManageWidgetState(PooledWidget, true); // Widget 활성화 (예: AddToViewport)
    }
    return PooledWidget;
}

UMaterialInstanceDynamic* UObjectPoolComponent::GetMaterialInstanceDynamicFromPool(UMaterialInterface* BaseMaterial)
{
    FObjectPoolInstance* PoolInstance = FindPoolInstanceByMaterial(BaseMaterial);
    if (!PoolInstance)
    {
        return nullptr;
    }

    if (!PoolInstance->AvailableObjects.IsEmpty())
    {
        return Cast<UMaterialInstanceDynamic>(PoolInstance->AvailableObjects.Pop(false));
    }
    return Cast<UMaterialInstanceDynamic>(RetrieveOrCreateObjectForPool(*PoolInstance));
}


// --- 오브젝트 반환 함수 구현 ---

void UObjectPoolComponent::ReturnObjectToPool(UObject* ObjectToReturn)
{
    if (!IsValid(ObjectToReturn))
    {
        return;
    }

    FObjectPoolInstance* PoolInstance = nullptr;
    if (UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(ObjectToReturn))
    {
        // MID의 Parent를 통해 원래 BaseMaterial을 찾아야 함. (복잡할 수 있음. 생성 시 저장해두거나, 다른 식별자 필요)
        // 여기서는 간단히 클래스로 찾지만, MID는 클래스가 동일하므로 다른 방법 필요.
        // 임시로, MID는 별도의 로직으로 처리하거나, 생성 시 PoolInstance와 매핑해둬야 함.
        // 여기서는 ObjectClass 기반으로만 찾도록 단순화.
        // 실제로는 UMaterialInstanceDynamic::Parent를 통해 원본 Material을 찾아야 함.
        // 또는 풀링 시점에 어떤 Material Pool에 속했는지 정보를 태깅/저장해야 함.
        // 지금은 MID 반환은 해당 Material의 PoolInstance를 찾는 로직이 추가되어야 완전해짐.
        // 간단히 하기 위해 클래스 기반으로만 찾음.
        PoolInstance = FindPoolInstanceByClass(ObjectToReturn->GetClass()); // MID의 경우 이 방식은 부정확함.
        // PoolInstance = FindPoolInstanceByMaterial(MID->Parent); // 이런식으로 찾아야 함.
    }
    else
    {
        PoolInstance = FindPoolInstanceByClass(ObjectToReturn->GetClass());
    }


    if (!PoolInstance)
    {
        // 풀이 없는 오브젝트는 그냥 소멸 처리 (혹은 다른 정책)
        if (AActor* ActorToDestroy = Cast<AActor>(ObjectToReturn)) ActorToDestroy->Destroy();
        else if (UActorComponent* CompToDestroy = Cast<UActorComponent>(ObjectToReturn)) CompToDestroy->DestroyComponent();
        // else ObjectToReturn->ConditionalBeginDestroy(); // 일반 UObject
        return;
    }

    // 이미 풀에 있거나, 최대 크기를 초과하면 반환하지 않고 파괴 (선택적)
    // if (PoolInstance->AvailableObjects.Contains(ObjectToReturn) || PoolInstance->AvailableObjects.Num() >= PoolInstance->TypeConfig.SpawnSettings.MaxPoolSize)
    // {
    //     // ... 파괴 로직 ...
    //     return;
    // }

    // 상태 비활성화 로직
    if (AActor* ActorToReturn = Cast<AActor>(ObjectToReturn))
    {
        ManageActorState(ActorToReturn, false);
    }
    else if (UActorComponent* ComponentToReturn = Cast<UActorComponent>(ObjectToReturn))
    {
        ManageComponentState(ComponentToReturn, false);
    }
    else if (UUserWidget* WidgetToReturn = Cast<UUserWidget>(ObjectToReturn))
    {
        ManageWidgetState(WidgetToReturn, false);
    }
    // Material Instance Dynamic은 특별한 비활성화 로직이 없을 수 있음.

    PoolInstance->AvailableObjects.Add(ObjectToReturn);
}

void UObjectPoolComponent::ReturnObjectsToPool(TArray<UObject*>& ObjectsToReturn)
{
    for (UObject* Obj : ObjectsToReturn)
    {
        ReturnObjectToPool(Obj);
    }
    ObjectsToReturn.Empty(); // 반환 후 배열 비우기 (선택적)
}

// --- 풀 관리 및 유틸리티 함수 구현 ---
void UObjectPoolComponent::PrewarmPool(TSubclassOf<UObject> ObjectClass, int32 CountToSpawn)
{
    FObjectPoolInstance* PoolInstance = FindPoolInstanceByClass(ObjectClass);
    if (!PoolInstance)
    {
        return;
    }

    int32 ActuallySpawnedCount = 0;
    for (int32 i = 0; i < CountToSpawn; ++i)
    {
        if (PoolInstance->AvailableObjects.Num() + PoolInstance->TotalSpawnedCount - PoolInstance->AvailableObjects.Num() /* 현재 활성 객체 수 고려 */ >= PoolInstance->TypeConfig.SpawnSettings.MaxPoolSize)
        {
            break; // 최대 풀 크기 도달
        }
        UObject* NewObj = RetrieveOrCreateObjectForPool(*PoolInstance);
        if (NewObj)
        {
            ReturnObjectToPool(NewObj); // 생성 후 바로 반환하여 사용 가능 상태로 만듦
            ActuallySpawnedCount++;
        }
        else
        {
            break; 
        }
    }
}

void UObjectPoolComponent::DestroyObjectsInPoolByClass(TSubclassOf<UObject> ObjectClass)
{
    FObjectPoolInstance* PoolInstance = FindPoolInstanceByClass(ObjectClass);
    if (!PoolInstance)
    {
        return;
    }

    for (UObject* Obj : PoolInstance->AvailableObjects)
    {
        if (IsValid(Obj))
        {
            if (AActor* ActorToDestroy = Cast<AActor>(Obj)) ActorToDestroy->Destroy();
            else if (UActorComponent* CompToDestroy = Cast<UActorComponent>(Obj)) CompToDestroy->DestroyComponent();
            // UserWidget이나 MaterialInstanceDynamic은 명시적 Destroy 호출이 다를 수 있음.
            // else Obj->ConditionalBeginDestroy(); // 일반 UObject
        }
    }
    PoolInstance->AvailableObjects.Empty();
    // 여기서 PoolInstances 배열에서 해당 PoolInstance 자체를 제거할 수도 있음.
    // PoolInstances.RemoveAt(FoundIndex); // 단, 인덱스 기반으로 찾아야 함.
}

void UObjectPoolComponent::DestroyMaterialInstancesInPool(UMaterialInterface* BaseMaterial)
{
    FObjectPoolInstance* PoolInstance = FindPoolInstanceByMaterial(BaseMaterial);
     if (!PoolInstance)
    {
        return;
    }
    // MaterialInstanceDynamic은 특별한 Destroy 호출이 필요 없을 수 있으나, 참조를 제거하는 것이 중요.
    PoolInstance->AvailableObjects.Empty();
}

void UObjectPoolComponent::DestroyAllPools()
{
    for (FObjectPoolInstance& PoolInstance : PoolInstances)
    {
        for (UObject* Obj : PoolInstance.AvailableObjects)
        {
            if (IsValid(Obj))
            {
                if (AActor* ActorToDestroy = Cast<AActor>(Obj)) ActorToDestroy->Destroy();
                else if (UActorComponent* CompToDestroy = Cast<UActorComponent>(Obj)) CompToDestroy->DestroyComponent();
            }
        }
        PoolInstance.AvailableObjects.Empty();
    }
    PoolInstances.Empty();
    LastFoundPoolIndex = INDEX_NONE;
}

bool UObjectPoolComponent::IsObjectClassRegistered(TSubclassOf<UObject> ObjectClass) const
{
    return FindPoolInstanceByClass(ObjectClass) != nullptr;
}

bool UObjectPoolComponent::IsMaterialInterfaceRegistered(UMaterialInterface* BaseMaterial) const
{
    return FindPoolInstanceByMaterial(BaseMaterial) != nullptr;
}

int32 UObjectPoolComponent::GetTotalObjectCountInAllPools() const
{
    int32 TotalCount = 0;
    for (const FObjectPoolInstance& PoolInstance : PoolInstances)
    {
        TotalCount += PoolInstance.AvailableObjects.Num();
        // 활성 객체 수도 포함하려면 추가 로직 필요
    }
    return TotalCount;
}

int32 UObjectPoolComponent::GetAvailableObjectCountForClass(TSubclassOf<UObject> ObjectClass) const
{
    const FObjectPoolInstance* PoolInstance = FindPoolInstanceByClass(ObjectClass);
    return PoolInstance ? PoolInstance->AvailableObjects.Num() : 0;
}

int32 UObjectPoolComponent::GetAvailableMaterialInstanceCount(UMaterialInterface* BaseMaterial) const
{
    const FObjectPoolInstance* PoolInstance = FindPoolInstanceByMaterial(BaseMaterial);
    return PoolInstance ? PoolInstance->AvailableObjects.Num() : 0;
}

FVector UObjectPoolComponent::GetTemporarySpawnLocationForActors() const
{
    return FVector(
        FMath::RandRange(PooledActorMinSpawnCoordinate, PooledActorMaxSpawnCoordinate),
        FMath::RandRange(PooledActorMinSpawnCoordinate, PooledActorMaxSpawnCoordinate),
        FMath::RandRange(PooledActorMinSpawnCoordinate, PooledActorMaxSpawnCoordinate)
    );
}

bool UObjectPoolComponent::IsLocationWithinTemporarySpawnArea(const FVector& Location, float ErrorMargin) const
{
    return (Location.X >= (PooledActorMinSpawnCoordinate - ErrorMargin) && Location.X <= (PooledActorMaxSpawnCoordinate + ErrorMargin) &&
            Location.Y >= (PooledActorMinSpawnCoordinate - ErrorMargin) && Location.Y <= (PooledActorMaxSpawnCoordinate + ErrorMargin) &&
            Location.Z >= (PooledActorMinSpawnCoordinate - ErrorMargin) && Location.Z <= (PooledActorMaxSpawnCoordinate + ErrorMargin));
}

void UObjectPoolComponent::SetTemporaryActorSpawnAreaLimits(float MinCoordinate, float MaxCoordinate)
{
    PooledActorMinSpawnCoordinate = MinCoordinate;
    PooledActorMaxSpawnCoordinate = MaxCoordinate;
}

FString UObjectPoolComponent::ConvertIntToUniqueString(int32 Value)
{
    if (Value < 0)
    {
        return FString();
    }

    // Unreal Engine의 FName이나 FGuid 생성 방식을 참고하거나, 간단한 Base36 인코딩 사용 가능
    static const TCHAR Alphabet[] = TEXT("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    const int32 Base = UE_ARRAY_COUNT(Alphabet) - 1; // 36

    FString Result;
    FUInt128 ValueCopy = Value; // 더 큰 값을 다루기 위해 FUInt128 사용 가능 (FInt128.h)

    if (ValueCopy == 0)
    {
        return TEXT("0");
    }

    while (ValueCopy > 0)
    {
        uint32 Remainder;
        ValueCopy = ValueCopy.Divide(Base, Remainder); // FUInt128::Divide 필요
        Result += Alphabet[Remainder];
    }

    return Result.Reverse(); // 결과를 뒤집어야 올바른 순서가 됨
}


// --- Private Helper Functions ---

void UObjectPoolComponent::UpdatePools()
{
    for (FObjectPoolInstance& PoolInstance : PoolInstances)
    {
        UpdateSinglePoolInstance(PoolInstance);
    }
}

void UObjectPoolComponent::UpdateSinglePoolInstance(FObjectPoolInstance& PoolInstance)
{
    // 현재 풀에 있는 오브젝트 수 (사용 가능한 것 + 활성화된 것). 정확한 활성 객체 수 파악이 어려우므로, MaxPoolSize 기준으로만 판단.
    // 실제로는 활성 객체 목록을 따로 관리하거나, GetWorld()->FindAllActors/ObjectsOfClass 등으로 현재 월드에 존재하는 객체 수를 파악해야 함.
    // 여기서는 단순하게 AvailableObjects.Num()과 MaxPoolSize를 비교.
    if (PoolInstance.AvailableObjects.Num() >= PoolInstance.TypeConfig.SpawnSettings.MaxPoolSize)
    {
        PoolInstance.CurrentSpawnDelayFrames = PoolInstance.TypeConfig.SpawnSettings.SpawnDelayInFramesAfterReachingMax;
        return;
    }

    if (PoolInstance.CurrentSpawnDelayFrames > 0)
    {
        PoolInstance.CurrentSpawnDelayFrames--;
        return;
    }

    for (int32 i = 0; i < PoolInstance.TypeConfig.SpawnSettings.NumToSpawnPerFrame; ++i)
    {
        if (PoolInstance.AvailableObjects.Num() >= PoolInstance.TypeConfig.SpawnSettings.MaxPoolSize) // 다시 한번 최대 크기 체크
        {
            PoolInstance.CurrentSpawnDelayFrames = PoolInstance.TypeConfig.SpawnSettings.SpawnDelayInFramesAfterReachingMax;
            return;
        }

        UObject* NewObj = RetrieveOrCreateObjectForPool(PoolInstance);
        if (NewObj)
        {
            ReturnObjectToPool(NewObj); // 생성 후 바로 풀에 반환 (비활성 상태로)
        }
        else
        {
            // 오브젝트 생성 실패 시 더 이상 시도하지 않음 (혹은 로그만 남기고 계속)
            return;
        }
    }
}

void UObjectPoolComponent::AddOrUpdatePoolConfiguration(const FPooledObjectTypeConfig& NewConfig)
{
    FObjectPoolInstance* ExistingPool = nullptr;
    if (IsValid(NewConfig.ObjectClassToPool))
    {
        ExistingPool = FindPoolInstanceByClass(NewConfig.ObjectClassToPool);
    }
    else if (IsValid(NewConfig.BaseMaterialInterface))
    {
        ExistingPool = FindPoolInstanceByMaterial(NewConfig.BaseMaterialInterface);
    }

    if (ExistingPool)
    {
        // 기존 풀 설정 업데이트 (예: MaxPoolSize 변경 시 처리 등)
        ExistingPool->TypeConfig = NewConfig;
        ExistingPool->AvailableObjects.Reserve(NewConfig.SpawnSettings.MaxPoolSize);
    }
    else
    {
        PoolInstances.Emplace(NewConfig); // 새 풀 인스턴스 생성 및 추가
    }
}


UObject* UObjectPoolComponent::RetrieveOrCreateObjectForPool(FObjectPoolInstance& PoolInstance)
{
    PoolInstance.TotalSpawnedCount++;
    FString ObjectNameSuffix = TEXT("_Pooled_") + ConvertIntToUniqueString(PoolInstance.TotalSpawnedCount);
    FName FinalObjectName = NAME_None;

    if (!PoolInstance.TypeConfig.SpawnSettings.ObjectBaseName.IsEmpty())
    {
        FinalObjectName = FName(*(PoolInstance.TypeConfig.SpawnSettings.ObjectBaseName + ObjectNameSuffix));
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        PoolInstance.TotalSpawnedCount--; // 생성 실패 시 카운트 복원
        return nullptr;
    }

    UObject* NewPooledObject = nullptr;
    UClass* ClassToSpawn = PoolInstance.TypeConfig.ObjectClassToPool; // TSubclassOf<UObject>는 UClass*로 사용 가능

    switch (PoolInstance.PoolType)
    {
    case EPooledObjectType::Actor:
        if (ClassToSpawn && ClassToSpawn->IsChildOf(AActor::StaticClass()))
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = GetOwner();
            SpawnParams.Instigator = GetOwner()->GetInstigator();
            if (FinalObjectName != NAME_None) SpawnParams.Name = FinalObjectName;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            NewPooledObject = World->SpawnActor<AActor>(ClassToSpawn, GetTemporarySpawnLocationForActors(), FRotator::ZeroRotator, SpawnParams);
            if (NewPooledObject)
            {
                ManageActorState(Cast<AActor>(NewPooledObject), false);
            }
        }
        else
        {
        }
        break;

    case EPooledObjectType::Component:
        if (ClassToSpawn && ClassToSpawn->IsChildOf(UActorComponent::StaticClass()))
        {
            // 컴포넌트는 일반적으로 액터에 의해 소유됩니다. 풀에서는 임시 Outer를 가질 수 있습니다.
            // 여기서는 이 ObjectPoolComponent 자신을 Outer로 사용합니다.
            NewPooledObject = NewObject<UActorComponent>(this, ClassToSpawn, FinalObjectName != NAME_None ? FinalObjectName : NAME_None);
            // UActorComponent* NewComp = Cast<UActorComponent>(NewPooledObject);
            // if (NewComp)
            // {
            //     // 생성 시점에 특별한 활성화 작업은 ManageComponentState에서 처리합니다.
            // }
        }
        else
        {
        }
        break;

    case EPooledObjectType::GenericUObject:
        if (ClassToSpawn) // 일반 UObject는 UObject의 자식이므로 IsChildOf(UObject::StaticClass())는 항상 참
        {
            NewPooledObject = NewObject<UObject>(GetTransientPackage(), ClassToSpawn, FinalObjectName != NAME_None ? FinalObjectName : NAME_None);
        }
        else
        {
        }
        break;

    case EPooledObjectType::MaterialInstanceDynamic:
        if (PoolInstance.TypeConfig.BaseMaterialInterface)
        {
            NewPooledObject = UMaterialInstanceDynamic::Create(PoolInstance.TypeConfig.BaseMaterialInterface, this);
        }
        else
        {
        }
        break;

    case EPooledObjectType::UserWidget:
        if (ClassToSpawn && ClassToSpawn->IsChildOf(UUserWidget::StaticClass()))
        {
            TSubclassOf<UUserWidget> WidgetClassToSpawn = ClassToSpawn; // UClass*에서 TSubclassOf<UUserWidget>로의 변환

            UObject* ResolvedWidgetOwner = nullptr;
            if (PoolInstance.TypeConfig.WidgetOwner.Get())
            {
                ResolvedWidgetOwner = PoolInstance.TypeConfig.WidgetOwner.Get();
            }

            if (!ResolvedWidgetOwner) // 명시된 오너가 없으면, 이 컴포넌트의 오너 액터의 컨트롤러를 시도
            {
                AActor* ComponentOwnerActor = GetOwner();
                if (ComponentOwnerActor)
                {
                    APawn* OwnerPawn = Cast<APawn>(ComponentOwnerActor);
                    if (OwnerPawn)
                    {
                        ResolvedWidgetOwner = Cast<APlayerController>(OwnerPawn->GetController());
                    }

                    // 만약 위에서 못 찾았고, 컴포넌트 오너 액터 자체가 PlayerController라면 (매우 드문 경우)
                    if (!ResolvedWidgetOwner && Cast<APlayerController>(ComponentOwnerActor)) {
                        ResolvedWidgetOwner = ComponentOwnerActor;
                    }
                }
            }

            // 오너 타입에 따라 CreateWidget 호출
            if (APlayerController* PC = Cast<APlayerController>(ResolvedWidgetOwner))
            {
                NewPooledObject = CreateWidget(PC, WidgetClassToSpawn, FinalObjectName != NAME_None ? FinalObjectName : NAME_None);
            }
            else if (UWidget* ParentWidget = Cast<UWidget>(ResolvedWidgetOwner))
            {
                NewPooledObject = CreateWidget(ParentWidget, WidgetClassToSpawn, FinalObjectName != NAME_None ? FinalObjectName : NAME_None);
            }
            else // 유효한 오너를 찾지 못하면 World를 사용 (최후의 수단)
            {
                NewPooledObject = CreateWidget(World, WidgetClassToSpawn, FinalObjectName != NAME_None ? FinalObjectName : NAME_None);
            }

            if (NewPooledObject)
            {
                ManageWidgetState(Cast<UUserWidget>(NewPooledObject), false);
            }
        }
        else
        {
        }
        break;

    default:
        break;
    }

    if (!NewPooledObject)
    {
        PoolInstance.TotalSpawnedCount--; 
    }
    return NewPooledObject;
}

FObjectPoolInstance* UObjectPoolComponent::FindPoolInstanceByClass(UClass* ObjectClass)
{
    if (!ObjectClass) return nullptr;

    // 캐시된 인덱스 우선 확인
    if (PoolInstances.IsValidIndex(LastFoundPoolIndex) &&
        PoolInstances[LastFoundPoolIndex].TypeConfig.ObjectClassToPool == ObjectClass)
    {
        return &PoolInstances[LastFoundPoolIndex];
    }

    for (int32 i = 0; i < PoolInstances.Num(); ++i)
    {
        if (PoolInstances[i].TypeConfig.ObjectClassToPool == ObjectClass)
        {
            LastFoundPoolIndex = i;
            return &PoolInstances[i];
        }
    }
    return nullptr;
}
const FObjectPoolInstance* UObjectPoolComponent::FindPoolInstanceByClass(UClass* ObjectClass) const
{
    // const 버전은 mutable LastFoundPoolIndex를 업데이트하지 않음
     if (!ObjectClass) return nullptr;
    for (int32 i = 0; i < PoolInstances.Num(); ++i)
    {
        if (PoolInstances[i].TypeConfig.ObjectClassToPool == ObjectClass)
        {
            return &PoolInstances[i];
        }
    }
    return nullptr;
}


FObjectPoolInstance* UObjectPoolComponent::FindPoolInstanceByMaterial(UMaterialInterface* BaseMaterial)
{
    if (!BaseMaterial) return nullptr;

    if (PoolInstances.IsValidIndex(LastFoundPoolIndex) &&
        PoolInstances[LastFoundPoolIndex].TypeConfig.BaseMaterialInterface == BaseMaterial)
    {
        return &PoolInstances[LastFoundPoolIndex];
    }

    for (int32 i = 0; i < PoolInstances.Num(); ++i)
    {
        if (PoolInstances[i].TypeConfig.BaseMaterialInterface == BaseMaterial)
        {
            LastFoundPoolIndex = i;
            return &PoolInstances[i];
        }
    }
    return nullptr;
}

const FObjectPoolInstance* UObjectPoolComponent::FindPoolInstanceByMaterial(UMaterialInterface* BaseMaterial) const
{
     if (!BaseMaterial) return nullptr;
    for (int32 i = 0; i < PoolInstances.Num(); ++i)
    {
        if (PoolInstances[i].TypeConfig.BaseMaterialInterface == BaseMaterial)
        {
            return &PoolInstances[i];
        }
    }
    return nullptr;
}


void UObjectPoolComponent::ManageActorState(AActor* Actor, bool bActivate)
{
    if (!IsValid(Actor)) return;

    if (bActivate)
    {
        Actor->SetActorHiddenInGame(false);
        Actor->SetActorTickEnabled(true); // 필요에 따라 Tick 활성화
        Actor->SetActorEnableCollision(true); // 필요한 경우 Collision 활성화
    }
    else
    {
        Actor->SetActorHiddenInGame(true);
        Actor->SetActorTickEnabled(false);
        Actor->SetActorEnableCollision(false); // Collision 비활성화
        Actor->SetActorLocation(GetTemporarySpawnLocationForActors()); // 화면 밖으로 이동
        Actor->SetActorRotation(FRotator::ZeroRotator); // 회전 초기화
        // 물리 상태 리셋
        if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
        {
            if (RootPrim->IsSimulatingPhysics())
            {
                RootPrim->SetSimulatePhysics(false);
                RootPrim->SetSimulatePhysics(true); // 잠시 껐다 켜서 물리 상태 리셋
                RootPrim->SetSimulatePhysics(false); // 다시 비활성화
            }
            RootPrim->SetPhysicsLinearVelocity(FVector::ZeroVector);
            RootPrim->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
        }
    }
}

void UObjectPoolComponent::ManageComponentState(UActorComponent* Component, bool bActivate, AActor* NewOwner)
{
    if (!IsValid(Component)) return;

    if (bActivate)
    {
        if (!Component->IsRegistered()) // 아직 등록되지 않았다면
        {
            if (NewOwner)
            {
                // 기존 Owner와 다르면 Rename. 동일하면 그대로 사용.
                // UObject::Rename의 첫 번째 인자는 NewName, 두 번째 인자는 NewOuter.
                // 이름을 변경하지 않으려면 첫 번째 인자에 NAME_None 또는 현재 이름을 전달.
                Component->Rename(nullptr, NewOwner); 
            }
            Component->RegisterComponent();
        }
        Component->Activate(true); // 컴포넌트 활성화
        Component->SetComponentTickEnabled(true); // 필요에 따라 Tick 활성화
    }
    else
    {
        Component->Deactivate(); // 컴포넌트 비활성화
        Component->SetComponentTickEnabled(false);
        if (Component->IsRegistered())
        {
            Component->UnregisterComponent(); // 풀로 돌아갈 때 등록 해제
        }
    }
}

void UObjectPoolComponent::ManageWidgetState(UUserWidget* Widget, bool bActivate)
{
    if (!IsValid(Widget)) return;

    if (bActivate)
    {
        if (!Widget->IsInViewport())
        {
            // PlayerScreen에 추가할지 Viewport에 추가할지 결정 필요
            // 여기서는 Viewport에 추가하는 것으로 가정
            Widget->AddToViewport(); 
        }
        Widget->SetVisibility(ESlateVisibility::Visible); // 또는 HitTestInvisible 등 필요에 따라
    }
    else
    {
        Widget->SetVisibility(ESlateVisibility::Collapsed); // 또는 Hidden
        if (Widget->IsInViewport())
        {
            Widget->RemoveFromParent();
        }
    }
}