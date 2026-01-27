#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "QuestEventData.generated.h"

USTRUCT(BlueprintType)
struct FQuestEventPayload
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Quest Event")
	TObjectPtr<UObject> Target = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Quest Event")
	int32 Amount = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Quest Event")
	FGameplayTag EventTag;

	UPROPERTY(BlueprintReadOnly, Category = "Quest Event")
	TArray<FString> Metadata;

	// Helper: 안전하게 Target을 특정 타입으로 캐스팅
	template<typename T>
	T* GetTargetAs() const
	{
		return Cast<T>(Target);
	}

	// Helper: Metadata에서 특정 키 찾기
	FString GetMetadata(const FString& Key) const
	{
		for (const FString& Meta : Metadata)
		{
			if (Meta.StartsWith(Key + "="))
			{
				return Meta.RightChop(Key.Len() + 1);
			}
		}
		return FString();
	}
};