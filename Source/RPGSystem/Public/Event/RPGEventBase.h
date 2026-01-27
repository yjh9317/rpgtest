
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"
#include "RPGEventBase.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API URPGEventBase : public UObject
{
	GENERATED_BODY()

	
public:
	URPGEventBase()
	{
		// EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Combat.DamageDealt"));
	}
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Event")
	FGameplayTag EventTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Event")
	FName EventName;

	UPROPERTY(BlueprintReadOnly, Category = "Event")
	TWeakObjectPtr<UObject> Instigator;

	//UPROPERTY(BlueprintReadOnly, Category = "Event")
	//FGameplayAbilityActorInfo ActorInfo;

	UPROPERTY(BlueprintReadOnly, Category = "Event")
	TWeakObjectPtr<UObject> Target;

	UPROPERTY(BlueprintReadOnly, Category = "Event")
	float Timestamp;

	UPROPERTY(BlueprintReadWrite, Category = "Event")
	bool bHandled = false;

	UPROPERTY(BlueprintReadWrite, Category = "Event")
	bool bIsCancelled = false;

	UFUNCTION(BlueprintCallable, Category = "Event") virtual FString GetDebugString() const; 
	UFUNCTION(BlueprintCallable, Category = "Event") void CancelEvent() { bIsCancelled = true; }
};