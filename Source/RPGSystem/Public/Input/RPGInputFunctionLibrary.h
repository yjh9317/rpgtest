// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "RPGInputFunctionLibrary.generated.h"
/**
 * 
 */
UCLASS()
class RPGSYSTEM_API URPGInputFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/**
	 * 특정 액션의 키 바인딩을 변경합니다. (가장 중요)
	 * @param Context 수정할 매핑 컨텍스트 (예: IMC_Default)
	 * @param Action 대상 입력 액션 (예: IA_Move, IA_Jump)
	 * @param OldKey 기존 키 (Move처럼 여러 키가 있는 경우, 어떤 키를 바꿀지 식별용. 단일 키면 FKey() 가능)
	 * @param NewKey 새로 할당할 키
	 * @return 변경 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "RPG|Input", meta = (WorldContext = "WorldContextObject"))
	static bool RebindKey(const UObject* WorldContextObject, UInputMappingContext* Context, const UInputAction* Action, FKey OldKey, FKey NewKey);

	/**
	 * [필수] 현재 이 액션에 할당된 '첫 번째' 키를 가져옵니다.
	 * UI에서 "현재 키: W"라고 보여줄 때 사용합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "RPG|Input")
	static FKey GetKeyForAction(const UInputMappingContext* Context, const UInputAction* Action);

	/**
	 * [필수] 이 액션에 할당된 '모든' 키를 가져옵니다.
	 * 예: IA_Move에 할당된 W, A, S, D를 전부 가져와서 UI에 목록으로 보여줄 때 필요합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "RPG|Input")
	static TArray<FKey> GetAllKeysForAction(const UInputMappingContext* Context, const UInputAction* Action);

	/**
	 * [권장] 특정 키가 이미 다른 액션에서 사용 중인지 확인합니다. (중복 방지)
	 * 예: 'I' 키를 공격으로 설정하려는데, 이미 인벤토리 키라면 경고를 띄울 수 있습니다.
	 */
	UFUNCTION(BlueprintPure, Category = "RPG|Input")
	static bool IsKeyMapped(const UInputMappingContext* Context, FKey Key);

	/**
	 * [선택] 해당 액션의 매핑을 아예 삭제합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "RPG|Input", meta = (WorldContext = "WorldContextObject"))
	static void UnbindAction(const UObject* WorldContextObject, UInputMappingContext* Context, const UInputAction* Action, FKey KeyToRemove);
};
