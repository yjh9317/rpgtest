// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/QuestEventMediator.h"
#include "Event/GlobalEventHandler.h"
#include "Quest/QuestEventListener.h"
#include "Quest/RPGQuest.h"
#include "Quest/Data/RPGQuestData.h"
#include "Quest/Data/Objectives/QuestObjectiveBase.h"

// void UQuestEventMediator::Initialize(UGlobalEventHandler* InGlobalEventHandler)
// {
// 	// [Safety Check 1] Null 체크
// 	if (!ensureMsgf(InGlobalEventHandler, TEXT("QuestEventMediator::Initialize - GlobalEventHandler is null!")))
// 	{
// 		return;
// 	}
//
// 	// [Safety Check 2] 중복 초기화 방지
// 	if (GlobalEventHandler.IsValid())
// 	{
// 		UE_LOG(LogTemp, Warning, TEXT("QuestEventMediator::Initialize - Already initialized, ignoring"));
// 		return;
// 	}
//
// 	GlobalEventHandler = InGlobalEventHandler;
// 	UE_LOG(LogTemp, Log, TEXT("QuestEventMediator initialized successfully"));
// }
//
// void UQuestEventMediator::RegisterListener(IQuestEventListener* Listener)
// {
// 	// [Safety Check 1] Null 체크
// 	if (!ensure(Listener))
// 	{
// 		UE_LOG(LogTemp, Warning, TEXT("QuestEventMediator::RegisterListener - Listener is null"));
// 		return;
// 	}
//
// 	// [Safety Check 2] GlobalEventHandler 유효성 체크
// 	if (!ensureMsgf(GlobalEventHandler.IsValid(), 
// 		TEXT("QuestEventMediator::RegisterListener - Not initialized! Call Initialize() first")))
// 	{
// 		return;
// 	}
//
// 	// 이 리스너가 관심있는 태그들을 가져옴
// 	TArray<FGameplayTag> ListenedTags = Listener->GetListenedEventTags();
//
// 	if (ListenedTags.Num() == 0)
// 	{
// 		UE_LOG(LogTemp, Warning, TEXT("QuestEventMediator::RegisterListener - Listener has no listened tags"));
// 		return;
// 	}
//
// 	for (const FGameplayTag& Tag : ListenedTags)
// 	{
// 		// [Safety Check 3] 태그 유효성 체크
// 		if (!ensureMsgf(Tag.IsValid(), 
// 			TEXT("QuestEventMediator::RegisterListener - Invalid tag in listener")))
// 		{
// 			continue;
// 		}
//
// 		// 태그별 리스너 맵에 추가
// 		TArray<IQuestEventListener*>& Listeners = ListenersByTag.FindOrAdd(Tag);
// 		
// 		// [Safety Check 4] 중복 등록 방지
// 		if (!Listeners.Contains(Listener))
// 		{
// 			Listeners.Add(Listener);
// 		}
//
// 		// 아직 GlobalEventHandler에 바인딩되지 않은 태그라면 바인딩
// 		BindTagIfNeeded(Tag);
//
// 		UE_LOG(LogTemp, Verbose, TEXT("QuestEventMediator: Registered listener for tag '%s' (Total: %d)"), 
// 			*Tag.ToString(), Listeners.Num());
// 	}
// }
//
// void UQuestEventMediator::UnregisterListener(IQuestEventListener* Listener)
// {
// 	if (!Listener) return;
//
// 	int32 TotalRemoved = 0;
//
// 	// 모든 태그 맵을 순회하며 해당 리스너 제거
// 	for (auto& Pair : ListenersByTag)
// 	{
// 		TotalRemoved += Pair.Value.Remove(Listener);
// 	}
//
// 	UE_LOG(LogTemp, Verbose, TEXT("QuestEventMediator: Unregistered listener (Removed from %d tags)"), TotalRemoved);
// }
//
// void UQuestEventMediator::UnregisterAllListenersForQuest(URPGQuest* Quest)
// {
// 	// [Safety Check] Quest 유효성
// 	if (!IsValid(Quest))
// 	{
// 		UE_LOG(LogTemp, Warning, TEXT("QuestEventMediator::UnregisterAllListenersForQuest - Invalid Quest"));
// 		return;
// 	}
//
// 	int32 TotalUnregistered = 0;
//
// 	// Quest에 속한 모든 Objective 제거
// 	for (UQuestObjectiveBase* Objective : Quest->RuntimeObjectives)
// 	{
// 		if (IQuestEventListener* Listener = Cast<IQuestEventListener>(Objective))
// 		{
// 			UnregisterListener(Listener);
// 			TotalUnregistered++;
// 		}
// 	}
//
// 	UE_LOG(LogTemp, Log, TEXT("QuestEventMediator: Unregistered %d listeners for Quest %d"), 
// 		TotalUnregistered, Quest->QuestData ? Quest->QuestData->QuestID : -1);
// }
//
// void UQuestEventMediator::HandleGlobalEvent(UObject* Publisher, UObject* Payload, const TArray<FString>& Metadata)
// {
// 	// [Performance Optimization] Early exit if no listeners
// 	if (ListenersByTag.Num() == 0)
// 	{
// 		return;
// 	}
//
// 	// [중요] 이벤트 태그 추출
// 	// 방법 1: Metadata[0]에서 추출 (현재 구현)
// 	// 방법 2: Publisher를 URPGEventBase로 캐스팅하여 EventTag 필드 읽기 (권장)
// 	
// 	FGameplayTag EventTag;
// 	
// 	if (URPGEventBase* EventBase = Cast<URPGEventBase>(Publisher))
// 	{
// 		// 방법 2: Type-safe approach
// 		EventTag = EventBase->EventTag;
// 	}
// 	else if (Metadata.Num() > 0)
// 	{
// 		// 방법 1: Fallback
// 		EventTag = FGameplayTag::RequestGameplayTag(FName(*Metadata[0]));
// 	}
//
// 	// [Safety Check] 태그 유효성
// 	if (!EventTag.IsValid())
// 	{
// 		UE_LOG(LogTemp, Warning, TEXT("QuestEventMediator::HandleGlobalEvent - Invalid event tag"));
// 		return;
// 	}
//
// 	// 해당 태그를 듣고 있는 리스너들 찾기
// 	TArray<IQuestEventListener*>* ListenersPtr = ListenersByTag.Find(EventTag);
// 	if (!ListenersPtr || ListenersPtr->Num() == 0)
// 	{
// 		// 아무도 안 듣고 있음 (정상 케이스)
// 		return;
// 	}
//
// 	// Payload 구성
// 	FQuestEventPayload QuestPayload;
// 	QuestPayload.Target = Payload;
// 	QuestPayload.EventTag = EventTag;
// 	QuestPayload.Metadata = Metadata;
// 	QuestPayload.Amount = 1; // 기본값 (필요시 Metadata에서 파싱)
//
// 	// [Performance Optimization] Copy array to avoid iterator invalidation
// 	// (리스너가 OnQuestEvent 내부에서 다른 리스너를 등록/해제할 경우)
// 	TArray<IQuestEventListener*> ListenersCopy = *ListenersPtr;
//
// 	// 모든 리스너에게 이벤트 전달
// 	int32 HandledCount = 0;
// 	for (IQuestEventListener* Listener : ListenersCopy)
// 	{
// 		// [Safety Check] Listener 유효성
// 		// C++ Interface는 nullptr 체크만 필요
// 		if (Listener)
// 		{
// 			if (Listener->OnQuestEvent(QuestPayload))
// 			{
// 				HandledCount++;
// 			}
// 		}
// 	}
//
// 	UE_LOG(LogTemp, VeryVerbose, TEXT("QuestEventMediator: Event '%s' handled by %d/%d listeners"),
// 		*EventTag.ToString(), HandledCount, ListenersCopy.Num());
// }
//
// void UQuestEventMediator::BindTagIfNeeded(const FGameplayTag& Tag)
// {
// 	// [Optimization] 이미 바인딩된 태그면 스킵 (O(1) 체크)
// 	if (BoundTags.Contains(Tag))
// 	{
// 		return;
// 	}
//
// 	// [Safety Check] GlobalEventHandler 유효성
// 	if (!ensureMsgf(GlobalEventHandler.IsValid(), 
// 		TEXT("QuestEventMediator::BindTagIfNeeded - GlobalEventHandler is invalid!")))
// 	{
// 		return;
// 	}
//
// 	// GlobalEventHandler에 델리게이트 바인딩
// 	FRPGOnEventCalledSingle Delegate;
// 	Delegate.BindUFunction(this, FName("HandleGlobalEvent"));
// 	GlobalEventHandler->BindGlobalEventByGameplayTag(Tag, Delegate);
//
// 	BoundTags.Add(Tag);
//
// 	UE_LOG(LogTemp, Log, TEXT("QuestEventMediator: Bound to GlobalEventHandler for tag '%s'"), *Tag.ToString());
// }
//
// int32 UQuestEventMediator::GetTotalListenerCount() const
// {
// 	int32 Total = 0;
// 	for (const auto& Pair : ListenersByTag)
// 	{
// 		Total += Pair.Value.Num();
// 	}
// 	return Total;
// }
//
// int32 UQuestEventMediator::GetListenerCountForTag(FGameplayTag Tag) const
// {
// 	if (const TArray<IQuestEventListener*>* Listeners = ListenersByTag.Find(Tag))
// 	{
// 		return Listeners->Num();
// 	}
// 	return 0;
// }