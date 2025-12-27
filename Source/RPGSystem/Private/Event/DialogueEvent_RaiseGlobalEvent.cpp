// Fill out your copyright notice in the Description page of Project Settings.


#include "Event/DialogueEvent_RaiseGlobalEvent.h"

#include "Event/GlobalEventHandler.h"

void UDialogueEvent_RaiseGlobalEvent::RecieveEventTriggered_Implementation(APlayerController* ConsideringPlayer, AActor* NPCActor)
{
    // 1. PlayerController를 통해 GameInstance에 접근합니다.
    UGameInstance* GameInstance = nullptr;
    if (ConsideringPlayer)
    {
        GameInstance = ConsideringPlayer->GetGameInstance();
    }
    else if (NPCActor)
    {
        GameInstance = NPCActor->GetGameInstance();
    }

    if (!GameInstance)
    {
        return;
    }

    // 2. GameInstanceSubsystem인 GlobalEventHandler를 가져옵니다.
    UGlobalEventHandler* GlobalEventHandler = GameInstance->GetSubsystem<UGlobalEventHandler>();
    if (!GlobalEventHandler)
    {
        return;
    }

    // 3. 설정된 방식에 따라 이벤트를 호출합니다.
    if (bUseGameplayTag)
    {
        if (EventTag.IsValid())
        {
            // 태그 기반 이벤트 호출
            // Payload나 Metadata가 필요하면 추가 인자를 구성해서 넘겨줄 수 있습니다.
            GlobalEventHandler->CallGlobalEventByGameplayTag(ConsideringPlayer, EventTag, NPCActor, TArray<FString>());
        }
    }
    else
    {
        if (!EventName.IsNone())
        {
            // 이름 기반 이벤트 호출
            GlobalEventHandler->CallGlobalEventByName(ConsideringPlayer, EventName, NPCActor, TArray<FString>());
        }
    }
}
