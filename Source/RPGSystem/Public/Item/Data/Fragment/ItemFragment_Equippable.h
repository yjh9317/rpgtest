// ItemFragment_Equippable.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ItemFragment_Usable.h"
#include "Item/Data/Fragment/ItemFragment.h"
#include "ItemFragment_Equippable.generated.h"


UENUM(BlueprintType)
enum class EEquipmentMeshType : uint8
{
    SkeletalMesh    UMETA(DisplayName = "Skeletal Mesh"), // 갑옷, 천옷 등
    StaticMesh      UMETA(DisplayName = "Static Mesh"),   // 방패, 투구, 무기 등
    // ChildActor      UMETA(DisplayName = "Child Actor") // 랜턴(라이트 포함) 등
};

/**
 * 장비 가능 Fragment
 * "사용(Use)" 시 "장착" 행위를 수행
 */
UCLASS(meta = (DisplayName = "Equippable"))
class RPGSYSTEM_API UItemFragment_Equippable : public UItemFragment_Usable // UItemFragment 대신 상속
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment", meta = (Categories = "Equipment.Slot"))
    FGameplayTag EquipmentSlotTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
    FName AttachSocketName = NAME_None;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
    EEquipmentMeshType MeshType = EEquipmentMeshType::SkeletalMesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment", meta = (EditCondition = "MeshType == EEquipmentMeshType::SkeletalMesh"))
    TSoftObjectPtr<USkeletalMesh> SkeletalEquipmentMesh;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment", meta = (EditCondition = "MeshType == EEquipmentMeshType::StaticMesh"))
    TSoftObjectPtr<UStaticMesh> StaticEquipmentMesh;

public:
    //========================================
    // UItemFragment_Usable 인터페이스 오버라이드
    //========================================
    
    /** 장착 가능 여부 확인 (예: 스탯, 레벨 제한 체크) */
    virtual bool CanUse_Implementation(AActor* User) const override;
    /** "사용" = "장착" 로직 실행 */
    virtual bool UseItem_Implementation(AActor* User) override;
    /** 장착은 아이템을 소모하지 않으므로, OnItemUsed는 비워둡니다. (쿨다운 갱신은 부모 호출) */
    virtual void OnItemUsed_Implementation(AActor* User) override;
};