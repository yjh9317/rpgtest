// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialInterface.h"
#include "Blueprint/UserWidget.h"
#include "ObjectPoolComponent.generated.h"


// 기본 풀 위치 값 (상수는 헤더나 전용 Constants.h 파일에 정의 가능)
constexpr float DEFAULT_POOL_OBJECT_MIN_POSITION = 0.0f;
constexpr float DEFAULT_POOL_OBJECT_MAX_POSITION = 100000.0f;

/**
 * @enum EPooledObjectType
 * @brief 풀링될 오브젝트의 주요 타입을 정의합니다.
 */
UENUM(BlueprintType)
enum class EPooledObjectType : uint8
{
    Undefined UMETA(DisplayName = "Undefined"), // 정의되지 않음
    Actor     UMETA(DisplayName = "Actor"),     // AActor 파생 클래스
    Component UMETA(DisplayName = "ActorComponent"), // UActorComponent 파생 클래스
    GenericUObject UMETA(DisplayName = "UObject"), // 일반 UObject 파생 클래스
    MaterialInstanceDynamic UMETA(DisplayName = "MaterialInstanceDynamic"), // UMaterialInstanceDynamic
    UserWidget UMETA(DisplayName = "UserWidget")  // UUserWidget 파생 클래스
};

/**
 * @struct FPooledObjectSpawnSettings
 * @brief 각 오브젝트 풀의 생성 및 관리 설정을 정의합니다.
 */
USTRUCT(BlueprintType)
struct FPooledObjectSpawnSettings
{
    GENERATED_BODY()

    /** 생성될 오브젝트의 기본 이름 (디버깅용, 고유할 필요는 없지만 권장) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool|SpawnSettings")
    FString ObjectBaseName;

    /** 풀에 유지할 최대 오브젝트 수 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool|SpawnSettings", meta = (ClampMin = "0"))
    int32 MaxPoolSize = 10;

    /** 프레임당 생성할 오브젝트 수 (0이면 초기 생성 없음) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool|SpawnSettings", meta = (ClampMin = "0"))
    int32 NumToSpawnPerFrame = 1;

    /** 풀이 최대치에 도달했을 때, 추가 생성을 시도하기 전 대기할 프레임 수 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool|SpawnSettings", meta = (ClampMin = "0"))
    int32 SpawnDelayInFramesAfterReachingMax = 0;

    FPooledObjectSpawnSettings() {}
};

/**
 * @struct FPooledObjectTypeConfig
 * @brief 특정 타입의 오브젝트 풀을 설정하기 위한 상세 정보입니다.
 */
USTRUCT(BlueprintType)
struct FPooledObjectTypeConfig
{
    GENERATED_BODY()

    /** 풀링할 UObject 클래스 (Actor, Component, Widget 등) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool|TypeConfig")
    TSubclassOf<UObject> ObjectClassToPool = nullptr;

    /** 머티리얼 인스턴스 다이나믹을 풀링할 경우, 원본 머티리얼 인터페이스 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool|TypeConfig")
    TObjectPtr<UMaterialInterface> BaseMaterialInterface = nullptr;

    /** 위젯을 풀링할 경우, 위젯의 소유자 (일반적으로 PlayerController 또는 다른 위젯) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool|TypeConfig")
    TObjectPtr<UUserWidget> WidgetOwner = nullptr; // 실제로는 PlayerController 등이 더 적합할 수 있음

    /** 이 타입의 오브젝트 풀에 적용될 스폰 설정 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool|TypeConfig")
    FPooledObjectSpawnSettings SpawnSettings;

    FPooledObjectTypeConfig() {}
};

/**
 * @struct FObjectPoolInstance
 * @brief 특정 타입의 오브젝트 풀 인스턴스 정보를 관리합니다.
 */
USTRUCT(BlueprintType)
struct FObjectPoolInstance
{
    GENERATED_BODY()

    /** 현재 풀에 사용 가능한 (비활성) 오브젝트 목록 */
    UPROPERTY(VisibleAnywhere, Category = "ObjectPool|InstanceData")
    TArray<TObjectPtr<UObject>> AvailableObjects;

    /** 이 풀에 대한 설정 정보 */
    UPROPERTY(EditAnywhere, Category = "ObjectPool|InstanceData")
    FPooledObjectTypeConfig TypeConfig;

    /** 다음 스폰까지 남은 지연 프레임 수 */
    UPROPERTY(VisibleAnywhere, Category = "ObjectPool|InstanceData")
    int32 CurrentSpawnDelayFrames = 0;

    /** 이 풀에서 총 생성된 오브젝트 수 (디버깅 및 이름 생성용) */
    UPROPERTY(VisibleAnywhere, Category = "ObjectPool|InstanceData")
    int32 TotalSpawnedCount = 0;

    /** 이 풀이 관리하는 오브젝트의 타입 */
    UPROPERTY(VisibleAnywhere, Category = "ObjectPool|InstanceData")
    EPooledObjectType PoolType = EPooledObjectType::Undefined;

    FObjectPoolInstance() {}

    FObjectPoolInstance(const FPooledObjectTypeConfig& InConfig)
        : TypeConfig(InConfig)
    {
        // TypeConfig에서 ObjectClassToPool 또는 BaseMaterialInterface 유효성에 따라 PoolType 결정
        if (IsValid(TypeConfig.ObjectClassToPool))
        {
            if (TypeConfig.ObjectClassToPool->IsChildOf(AActor::StaticClass()))
                PoolType = EPooledObjectType::Actor;
            else if (TypeConfig.ObjectClassToPool->IsChildOf(UActorComponent::StaticClass())) // UPrimitiveComponent도 UActorComponent임
                PoolType = EPooledObjectType::Component;
            else if (TypeConfig.ObjectClassToPool->IsChildOf(UUserWidget::StaticClass()))
                PoolType = EPooledObjectType::UserWidget;
            else
                PoolType = EPooledObjectType::GenericUObject;
        }
        else if (IsValid(TypeConfig.BaseMaterialInterface))
        {
            PoolType = EPooledObjectType::MaterialInstanceDynamic;
        }
        else
        {
            PoolType = EPooledObjectType::Undefined;
        }
        AvailableObjects.Reserve(TypeConfig.SpawnSettings.MaxPoolSize);
    }
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RPGSYSTEM_API UObjectPoolComponent : public UActorComponent
{
	GENERATED_BODY()

public:
    UObjectPoolComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override; // BeginDestroy 보다 EndPlay가 더 적합할 수 있음

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- 풀 설정 추가 함수 ---
    UFUNCTION(BlueprintCallable, Category = "ObjectPool|Setup", meta = (DisplayName = "Add Object Pool By Class"))
    void AddPoolForUObjectClass(TSubclassOf<UObject> ObjectClass, const FPooledObjectSpawnSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "ObjectPool|Setup", meta = (DisplayName = "Add Actor Pool"))
    void AddPoolForActorClass(TSubclassOf<AActor> ActorClass, const FPooledObjectSpawnSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "ObjectPool|Setup", meta = (DisplayName = "Add Component Pool"))
    void AddPoolForActorComponentClass(TSubclassOf<UActorComponent> ComponentClass, const FPooledObjectSpawnSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "ObjectPool|Setup", meta = (DisplayName = "Add Material Instance Dynamic Pool"))
    void AddPoolForMaterialInstanceDynamic(UMaterialInterface* BaseMaterial, const FPooledObjectSpawnSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "ObjectPool|Setup", meta = (DisplayName = "Add User Widget Pool"))
    void AddPoolForUserWidgetClass(TSubclassOf<UUserWidget> WidgetClass, UUserWidget* OwningWidgetOrPlayerController, const FPooledObjectSpawnSettings& Settings);

    // --- 오브젝트 가져오기 함수 ---
    UFUNCTION(BlueprintCallable, Category = "ObjectPool|Usage", meta = (DisplayName = "Get Pooled Object"))
    UObject* GetObjectFromPool(TSubclassOf<UObject> ObjectClass);

    UFUNCTION(BlueprintCallable, Category = "ObjectPool|Usage", meta = (DisplayName = "Get Pooled Actor"))
    AActor* GetActorFromPool(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform);

    UFUNCTION(BlueprintCallable, Category = "ObjectPool|Usage", meta = (DisplayName = "Get Pooled ActorComponent"))
    UActorComponent* GetActorComponentFromPool(TSubclassOf<UActorComponent> ComponentClass, AActor* NewOwner);

    UFUNCTION(BlueprintCallable, Category = "ObjectPool|Usage", meta = (DisplayName = "Get Pooled UserWidget"))
    UUserWidget* GetWidgetFromPool(TSubclassOf<UUserWidget> WidgetClass);

    UFUNCTION(BlueprintCallable, Category = "ObjectPool|Usage", meta = (DisplayName = "Get Pooled MaterialInstanceDynamic"))
    UMaterialInstanceDynamic* GetMaterialInstanceDynamicFromPool(UMaterialInterface* BaseMaterial);

    // --- 오브젝트 반환 함수 ---
    UFUNCTION(BlueprintCallable, Category = "ObjectPool|Usage", meta = (DisplayName = "Return Object To Pool"))
    void ReturnObjectToPool(UObject* ObjectToReturn);

    UFUNCTION(BlueprintCallable, Category = "ObjectPool|Usage", meta = (DisplayName = "Return Multiple Objects To Pool"))
    void ReturnObjectsToPool(UPARAM(ref) TArray<UObject*>& ObjectsToReturn);

    // --- 풀 관리 및 유틸리티 함수 ---
    UFUNCTION(BlueprintCallable, Category = "ObjectPool|Management", meta = (DisplayName = "Prewarm Pool For Class"))
    void PrewarmPool(TSubclassOf<UObject> ObjectClass, int32 CountToSpawn);

    UFUNCTION(BlueprintCallable, Category = "ObjectPool|Management", meta = (DisplayName = "Destroy All Objects In Pool For Class"))
    void DestroyObjectsInPoolByClass(TSubclassOf<UObject> ObjectClass);

    UFUNCTION(BlueprintCallable, Category = "ObjectPool|Management", meta = (DisplayName = "Destroy All Material Instances For Base Material"))
    void DestroyMaterialInstancesInPool(UMaterialInterface* BaseMaterial);

    UFUNCTION(BlueprintCallable, Category = "ObjectPool|Management", meta = (DisplayName = "Destroy All Pooled Objects"))
    void DestroyAllPools();

    UFUNCTION(BlueprintPure, Category = "ObjectPool|Query")
    bool IsObjectClassRegistered(TSubclassOf<UObject> ObjectClass) const;
    
    UFUNCTION(BlueprintPure, Category = "ObjectPool|Query")
    bool IsMaterialInterfaceRegistered(UMaterialInterface* BaseMaterial) const;

    UFUNCTION(BlueprintPure, Category = "ObjectPool|Query")
    int32 GetTotalObjectCountInAllPools() const;

    UFUNCTION(BlueprintPure, Category = "ObjectPool|Query")
    int32 GetAvailableObjectCountForClass(TSubclassOf<UObject> ObjectClass) const;

    UFUNCTION(BlueprintPure, Category = "ObjectPool|Query")
    int32 GetAvailableMaterialInstanceCount(UMaterialInterface* BaseMaterial) const;

    UFUNCTION(BlueprintPure, Category = "ObjectPool|Utility")
    FVector GetTemporarySpawnLocationForActors() const;

    UFUNCTION(BlueprintPure, Category = "ObjectPool|Utility")
    bool IsLocationWithinTemporarySpawnArea(const FVector& Location, float ErrorMargin = 100000.0f) const;

    UFUNCTION(BlueprintCallable, Category = "ObjectPool|Utility")
    void SetTemporaryActorSpawnAreaLimits(float MinCoordinate, float MaxCoordinate);

    UFUNCTION(BlueprintPure, Category = "ObjectPool|Utility")
    static FString ConvertIntToUniqueString(int32 Value);

private:
    /** 주기적으로 풀을 업데이트하여 필요시 오브젝트를 추가 생성합니다. */
    void UpdatePools();
    void UpdateSinglePoolInstance(FObjectPoolInstance& PoolInstance);

    /** 새로운 타입의 풀 설정을 추가하거나 기존 설정을 업데이트합니다. */
    void AddOrUpdatePoolConfiguration(const FPooledObjectTypeConfig& NewConfig);

    /** 특정 풀 인스턴스에서 사용 가능한 오브젝트를 가져오거나 새로 생성합니다. */
    UObject* RetrieveOrCreateObjectForPool(FObjectPoolInstance& PoolInstance);

    /** 내부적으로 UClass를 키로 사용하여 풀 인스턴스를 찾습니다. */
    FObjectPoolInstance* FindPoolInstanceByClass(UClass* ObjectClass);
    const FObjectPoolInstance* FindPoolInstanceByClass(UClass* ObjectClass) const;

    /** 내부적으로 UMaterialInterface를 키로 사용하여 풀 인스턴스를 찾습니다. */
    FObjectPoolInstance* FindPoolInstanceByMaterial(UMaterialInterface* BaseMaterial);
    const FObjectPoolInstance* FindPoolInstanceByMaterial(UMaterialInterface* BaseMaterial) const;

    /** Actor 타입 오브젝트의 활성/비활성 상태를 처리합니다. */
    void ManageActorState(AActor* Actor, bool bActivate);

    /** Component 타입 오브젝트의 활성/비활성 상태를 처리합니다. */
    void ManageComponentState(UActorComponent* Component, bool bActivate, AActor* NewOwner = nullptr);

    /** Widget 타입 오브젝트의 활성/비활성 상태를 처리합니다. */
    void ManageWidgetState(UUserWidget* Widget, bool bActivate);

private:
    /** 풀링할 오브젝트 타입별 설정을 저장하는 배열입니다. */
    UPROPERTY(VisibleInstanceOnly, Category = "ObjectPool|Internal")
    TArray<FObjectPoolInstance> PoolInstances;

    /** MID -> BaseMaterial 매핑 (반환 시 정확한 풀 식별용) */
    UPROPERTY(Transient)
    TMap<TObjectPtr<UMaterialInstanceDynamic>, TObjectPtr<UMaterialInterface>> MaterialToBaseMap;

    /** 마지막으로 찾은 풀 인스턴스의 인덱스 (캐싱용) */
    mutable int32 LastFoundPoolIndex = INDEX_NONE;

    /** 비활성 액터를 임시로 위치시킬 공간의 최소 좌표 */
    UPROPERTY(EditAnywhere, Category = "ObjectPool|Internal")
    float PooledActorMinSpawnCoordinate = DEFAULT_POOL_OBJECT_MIN_POSITION;

    /** 비활성 액터를 임시로 위치시킬 공간의 최대 좌표 */
    UPROPERTY(EditAnywhere, Category = "ObjectPool|Internal")
    float PooledActorMaxSpawnCoordinate = DEFAULT_POOL_OBJECT_MAX_POSITION;
		
};
