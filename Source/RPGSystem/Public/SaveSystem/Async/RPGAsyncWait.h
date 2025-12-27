#pragma once

#include "SaveSystem/Data/RPGSaveTypes.h"
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "RPGAsyncWait.generated.h"

class URPGSaveSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAsyncWaitOutputPin);

UCLASS()
class RPGSYSTEM_API URPGAsyncWait : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

protected:
	URPGAsyncWait();

public:

	UPROPERTY(BlueprintAssignable)
	FAsyncWaitOutputPin OnCompleted;
	
private:

	UPROPERTY()
	TObjectPtr<URPGSaveSubsystem> RPGSaveSubsystem;

	EAsyncCheckType Type;
	bool bSaveOrLoadTaskWasActive;
	
	uint32 CheckCounter;

	FTimerHandle WaitTimerHandle;

public:

	/**
	* Wait until SaveGameActors or LoadGameActors have been completed.
	* For example, this is useful if you want access loaded variables in a BeginPlay event instead of ActorLoaded.
	* 
	* @param CheckType - The type of Async operation to wait for.
	*/
	UFUNCTION(BlueprintCallable, Category = "RPG Save | Actors", meta = (DisplayName = "Wait For Save or Load Completed", BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"))
	static URPGAsyncWait* AsyncWaitForOperation(UObject* WorldContextObject, EAsyncCheckType CheckType = EAsyncCheckType::CT_Load);

	virtual void Activate() override;

private:

	void StartWaitTask();
	void CompleteWaitTask();
	void ForceProceed();

};
