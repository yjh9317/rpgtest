#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "QuestObjectiveBase.generated.h"

class URPGQuest;
class ARPGPlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveProgressChanged, UQuestObjectiveBase*, Objective);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveCompleted, UQuestObjectiveBase*, Objective);

UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced, DisplayName = "Quest Objective Base")
class RPGSYSTEM_API UQuestObjectiveBase : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION()
	virtual void OnGlobalEvent(UObject* Publisher, UObject* Payload, const TArray<FString>& Metadata);
	
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	FText Description; // 예: "고블린을 처치하세요 ({Current}/{Target})"

	UPROPERTY(BlueprintReadOnly, Category = "Objective | Runtime")
	bool bIsCompleted = false;

	virtual void ActivateObjective(URPGQuest* OwnerQuest);
	virtual void DeactivateObjective();
	virtual FString GetProgressString() const { return TEXT(""); }

	FOnObjectiveProgressChanged OnProgressChanged;
	FOnObjectiveCompleted OnCompleted;

protected:
	UPROPERTY(Transient)
	TObjectPtr<URPGQuest> OwningQuest;

	void FinishObjective();
	
};