// ItemDefinition.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "ItemDefinition.generated.h"

class UItemFragment;
class UItemInstance;

/**
 * 아이템의 정적 정의 (PrimaryDataAsset)
 * 
 * - 모든 아이템의 불변 데이터를 정의
 * - Fragment들은 Instanced로 설정되어 에디터에서 직접 편집 가능
 * - CreateInstance()를 통해 런타임 UItemInstance를 생성
 */
UCLASS(BlueprintType, Blueprintable)
class RPGSYSTEM_API UItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UItemDefinition();
	
	// ========================================
	// Display (기본 정보)
	// ========================================
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
	FText ItemName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
	TSoftObjectPtr<UTexture2D> Icon;

	// ========================================
	// World Display
	// ========================================
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "World Display")
	TSoftObjectPtr<UStaticMesh> WorldMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "World Display")
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;

	// ========================================
	// Core Properties (모든 아이템이 가지는 것)
	// ========================================
	
	/** 아이템 무게 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties", 
		meta = (ClampMin = "0.0"))
	float Weight = 1.0f;

	/** 기본 가치 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties", 
		meta = (ClampMin = "0.0"))
	float BaseValue = 1.0f;

	/** 요구 레벨 (옵션) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties", 
		meta = (ClampMin = "1"))
	int32 RequiredLevel = 1;

	// ========================================
	// Stacking (거의 모든 아이템이 영향받음)
	// ========================================
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stacking")
	bool bStackable = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stacking", 
		meta = (EditCondition = "bStackable", ClampMin = "1"))
	int32 MaxStackSize = 1;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Classification", meta = (Categories = "Item"))
	FGameplayTagContainer ItemTags;

	// ========================================
	// Fragments (Instanced = 에디터에서 바로 편집)
	// ========================================
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "Fragments")
	TArray<TObjectPtr<UItemFragment>> Fragments;

public:
	/**
	 * 런타임 아이템 인스턴스 생성
	 * 
	 * @param Outer - 인스턴스를 소유할 Object (보통 InventoryComponent)
	 * @return 생성된 UItemInstance (Fragment들이 복제되어 포함됨)
	 */
	UFUNCTION(BlueprintCallable, Category = "Item")
	UItemInstance* CreateInstance(UObject* Outer) const;
	
#if WITH_EDITOR
	// ========================================
	// 에디터 전용: 검증
	// ========================================
	
	/**
	 * PostEditChangeProperty: 프로퍼티 변경 시 자동 검증
	 */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
	/**
	 * PreSave: 저장 전 검증
	 */
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
	
	/**
	 * 수동 검증 함수 (블루프린트 노출)
	 */
	UFUNCTION(CallInEditor, Category = "Validation")
	void ValidateItemDefinition();
	
private:
	/** 실제 검증 로직 */
	bool ValidateInternal(bool bShowNotification = false);
#endif

	// ========================================
	// Asset Manager
	// ========================================
	
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("ItemDef", GetFName());
	}
public:
	// ========================================
	// Helpers
	// ========================================
	
	/**
	 * Definition의 Fragment 템플릿 검색 (const)
	 * Instance 생성 전에 기능 존재 여부 확인용
	 */
	template<typename T>
	const T* FindFragmentByClass() const
	{
		for (const UItemFragment* Fragment : Fragments)
		{
			if (const T* TypedFragment = Cast<T>(Fragment))
			{
				return TypedFragment;
			}
		}
		return nullptr;
	}

	/** 특정 태그 보유 여부 */
	UFUNCTION(BlueprintPure, Category = "Item")
	bool HasTag(const FGameplayTag& TagToCheck) const
	{
		return ItemTags.HasTag(TagToCheck);
	}

	/** 여러 태그 중 하나라도 보유 여부 */
	UFUNCTION(BlueprintPure, Category = "Item")
	bool HasAnyTag(const FGameplayTagContainer& TagsToCheck) const
	{
		return ItemTags.HasAny(TagsToCheck);
	}

	// ========================================
	// Inline Getters
	// ========================================

	FORCEINLINE float GetWeight() const { return Weight; }
	FORCEINLINE float GetValue() const { return BaseValue; }
	FORCEINLINE bool IsStackable() const { return bStackable; }
	FORCEINLINE int32 GetMaxStackSize() const { return MaxStackSize; }
};