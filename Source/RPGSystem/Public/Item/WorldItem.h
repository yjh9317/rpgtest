#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/Interface/InteractableInterface.h"
#include "SaveSystem/Interface/RPGActorSaveInterface.h"
#include "WorldItem.generated.h"

class UInteractableComponent;
class UWidgetComponent;
class UItemInstance;
class UItemDefinition;
class USphereComponent;

UCLASS()
class RPGSYSTEM_API AWorldItem : public AActor, public IInteractableInterface,
                                                public IRPGActorSaveInterface
{
    GENERATED_BODY()
    
public:    
    AWorldItem();
    virtual void OnConstruction(const FTransform& Transform) override;
    //~Begin AActor Interface
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
    virtual void BeginPlay() override;
    //~End AActor Interface

    //~Begin IInteractableInterface
    virtual void Initialize() override;
    virtual void Interaction(AActor* Interactor) override; // [Server] 실제 아이템 획득
    virtual void ClientBeginInteraction(AActor* Interactor) override; // [Client] UI 표시 (LookAt)
    virtual void ClientEndInteraction(AActor* Interactor) override;   // [Client] UI 숨김 (LookAway)
    //~End IInteractableInterface

    // 초기화 함수 (드랍 시 호출)
    void InitializeItem(UItemInstance* InInstance);

#if WITH_EDITOR
    // 에디터에서 프로퍼티 변경 시 호출되는 함수
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
protected:
    /** * 실제 아이템 데이터 (런타임 상태 포함).
     * ReplicatedSubobject로 설정하여 클라이언트에도 복제되도록 함.
     */
    UPROPERTY(ReplicatedUsing = OnRep_ItemInstance, VisibleAnywhere, BlueprintReadOnly, Category = "Item")
    TObjectPtr<UItemInstance> ItemInstance;

    /** 에디터 배치 테스트용 (BeginPlay에서 Instance 생성) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    TSoftObjectPtr<UItemDefinition> DefaultItemDef;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    int32 DefaultQuantity = 1;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
    int32 StencilValue = 1;

    // --- Components ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USphereComponent> InteractionCollision;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UWidgetComponent> InteractionWidget;
    
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> InteractionWidgetClass;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UInteractableComponent> InteractableComponent;
    
    // 필요하다면 SkeletalMeshComponent도 추가 (Definition 설정에 따라 스위칭)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

    UPROPERTY(EditDefaultsOnly,BlueprintReadWrite, Category = "Default")
    float SphereRadius = 80.f;
    
    UFUNCTION()
    void OnRep_ItemInstance();

    /** 시각적 외형(메쉬) 업데이트 */
    void UpdateVisuals();
    
private:
    // 에디터용 비주얼 업데이트 함수 (Instance가 없을 때 Definition만 보고 처리)
    void UpdateVisualsFromDefinition();
    
public:
    FORCEINLINE UItemInstance* GetItemInstance() const { return ItemInstance; }
};