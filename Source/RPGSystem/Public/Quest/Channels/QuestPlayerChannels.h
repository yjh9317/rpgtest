#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuestPlayerChannels.generated.h"

class UQuestChannel;
class UQuestManagerComponent;
class URPGQuest;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPGSYSTEM_API UQuestPlayerChannels : public UActorComponent
{
	GENERATED_BODY()

public:
	UQuestPlayerChannels();

protected:
	virtual void BeginPlay() override;

public:
	// [유일한 채널] UI와 사운드 연출 담당
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
	TObjectPtr<UQuestChannel> QuestChannel;
	
	// [설정] 퀘스트 위젯 클래스 (BP에서 설정)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TSubclassOf<class UUserWidget> QuestWidgetClass;
	
	// [설정] 사운드 데이터 (BP에서 설정)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TObjectPtr<class UQuestSounds> QuestSounds;

private:
	UPROPERTY()
	TObjectPtr<UQuestManagerComponent> QuestManager;

	// 퀘스트가 새로 시작될 때 호출됨 (QuestManager 등의 델리게이트와 연결)
	UFUNCTION()
	void HandleQuestStarted(URPGQuest* NewQuest);
};