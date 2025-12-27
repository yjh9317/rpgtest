// ItemFragment.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ItemFragment.generated.h"

class UItemInstance;
class AActor;

/**
 * 아이템의 모듈화된 기능 조각
 * 
 * - Definition에서는 Instanced로 설정된 템플릿으로 존재
 * - Instance 생성 시 DuplicateObject로 복제되어 인스턴스별 런타임 상태 유지
 * - 각 Fragment는 자신이 필요한 런타임 변수를 자유롭게 선언 가능
 */

/**
 * 아이템 기능 단위 (Fragment)
 * 
 * - Blueprintable: BP에서 상속하여 커스텀 Fragment 생성 가능
 * - EditInlineNew: Definition에서 인라인 편집 가능
 * - DefaultToInstanced: 자동으로 Instanced 처리
 */

UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class RPGSYSTEM_API UItemFragment : public UObject
{
	GENERATED_BODY()
	
public:
	//~UObject interface
	virtual bool IsSupportedForNetworking() const override { return true; }
	//~End of UObject interface

	/**
	 * Fragment가 Instance에 복제된 직후 호출
	 * 런타임 변수 초기화용 (예: CurrentDurability = MaxDurability)
	 */
	virtual void OnInstanced(UItemInstance* Instance) {}

	UFUNCTION(BlueprintPure, Category = "Item Fragment")
	UItemInstance* GetOwnerInstance() const;
    
	UFUNCTION(BlueprintPure, Category = "Item Fragment")
	AActor* GetOwnerActor() const;
};