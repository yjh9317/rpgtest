#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractorInterface.generated.h"

class UInteractableComponent;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractorInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 상호작용을 수행하는 주체(예: 플레이어 캐릭터)가 구현하는 인터페이스입니다.
 * InteractionComponent가 Owner에게 상태 변경을 알릴 때 사용됩니다.
 */
class RPGSYSTEM_API IInteractorInterface
{
	GENERATED_BODY()

public:
	/**
	 * 오브젝트와의 상호작용을 시작합니다
	 * @param InteractableComponent 상호작용할 대상 컴포넌트
	 */
	virtual void StartInteractionWithObject(UInteractableComponent* InteractableComponent) = 0;
	
	/**
	 * 오브젝트와의 상호작용을 종료합니다
	 * @param InteractableComponent 상호작용을 종료할 대상 컴포넌트
	 */
	virtual void EndInteractionWithObject(UInteractableComponent* InteractableComponent) = 0;
	
	/**
	 * 오브젝트와의 상호작용을 제거합니다 (강제 종료)
	 * @param InteractableComponent 상호작용을 제거할 대상 컴포넌트
	 */
	virtual void RemoveInteractionWithObject(UInteractableComponent* InteractableComponent) = 0;
	
	/**
	 * 오브젝트와의 상호작용을 초기화합니다 (상호작용 전 준비 단계)
	 * @param InteractableComponent 초기화할 대상 컴포넌트
	 */
	virtual void InitializeInteractionWithObject(UInteractableComponent* InteractableComponent) = 0;
	
	/**
	 * 현재 상호작용 중인 오브젝트를 반환합니다
	 * @return 현재 상호작용 중인 액터, 없으면 nullptr
	 */
	virtual AActor* GetCurrentInteractableObject() = 0;
};