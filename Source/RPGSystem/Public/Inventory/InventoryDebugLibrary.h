// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InventoryDebugLibrary.generated.h"

class UInventoryCoreComponent;
struct FGuid;
/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UInventoryDebugLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/** 모든 인벤토리의 상세 상태를 Output Log에 출력합니다. */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Debug", meta = (WorldContext = "WorldContextObject"))
	static void LogAllInventories(const UObject* WorldContextObject, UInventoryCoreComponent* InventoryComp);

	/** 특정 GUID를 가진 인벤토리의 상세 정보를 출력합니다. */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Debug")
	static void LogInventoryByGuid(UInventoryCoreComponent* InventoryComp, FGuid InventoryGuid);

	/** 특정 이름(FName)을 가진 인벤토리의 상세 정보를 출력합니다. */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Debug")
	static void LogInventoryByName(UInventoryCoreComponent* InventoryComp, FName InventoryName);

	/** 화면(Viewport)에 현재 인벤토리들의 요약 정보를 텍스트로 표시합니다. (PrintString 대체) */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Debug")
	static void DrawDebugInventoryOnScreen(UInventoryCoreComponent* InventoryComp, float Duration = 5.0f);
};
