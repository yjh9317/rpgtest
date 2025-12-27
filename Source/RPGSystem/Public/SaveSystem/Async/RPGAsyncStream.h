
#pragma once

#include "SaveSystem/Data/RPGSaveTypes.h"
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "SaveSystem/Data/RPGSaveData.h"
#include "RPGAsyncStream.generated.h"

class URPGSaveSubsystem;
class FRPGSaveLevelLoader;

UCLASS()
class RPGSYSTEM_API URPGAsyncStream : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

protected:
	URPGAsyncStream();

public:

	UPROPERTY()
	TObjectPtr<ULevel> StreamingLevel = nullptr;

	ESaveGameMode Mode;

private:

	TSharedPtr<FRPGSaveLevelLoader> Loader;

	UPROPERTY()
	TObjectPtr<URPGSaveSubsystem> RPGSaveSubsystem;

	UPROPERTY(Transient)
	TMap<FName, const TWeakObjectPtr<AActor>> StreamActorsMap;

	UPROPERTY(Transient)
	FMultiLevelStreamingData PrunedData;

	uint8 bIsActive : 1;

	int32 TotalActors;

public:

	UFUNCTION()
	static bool InitStreamingLoadTask(URPGSaveSubsystem* _RPGSaveSubsystem, ULevel* InLevel);

	virtual void Activate() override;
	inline bool IsActive() const { return bIsActive; }

	void ForceDestroy();

private:

	void StartLoad();
	bool SetupLevelActors();

	void FinishLoadingStreamLevel();

	void EndTask(const bool bBroadcastFinish);
	void Deactivate();
};
