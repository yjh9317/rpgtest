// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/SaveGame.h"
#include "RPGSettingsSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API URPGSettingsSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	// 일반 액션 (점프, 공격 등) 저장: [태그 : 키]
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	TMap<FGameplayTag, FKey> ActionMappings;

	// 축 매핑 (WASD 이동) 저장: [원래 키 : 바꾼 키]
	// 예: { Key_W : Key_UpArrow } -> W가 할당된 곳을 찾아 위쪽 화살표로 교체하라는 뜻
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	TMap<FKey, FKey> AxisKeyOverrides;
};
