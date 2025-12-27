// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "GlobalEventHandler.generated.h"

class URPGEventBase;
/**
* A delegate linked to a single event. Used to bind/unbind (subscribe/unsubscribe) and event to a global multicast event.
*/
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FRPGOnEventCalledSingle, UObject*, Publisher, UObject*, Payload, const TArray<FString>&, Metadata);

/**
* A multicast delegate linked to many delegates. Used to call (publish) to every linked delegated associated to this multicast.
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FRPGOnEventCalledMulticast, UObject*, Publisher, UObject*, Payload, const TArray<FString>&, Metadata);

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UGlobalEventHandler : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
	/**
	* Dictionary that holds the active global events by using a URPGEventBase class as key
	*/
	UPROPERTY()
	TMap<TSubclassOf<URPGEventBase>, FRPGOnEventCalledMulticast> ListenerEventsByClass;
	/**
	* Dictionary that holds the active global events by using a name as key
	*/
	UPROPERTY()
	TMap<FName, FRPGOnEventCalledMulticast> ListenerEventsByName;
	/**
	* Dictionary that holds the active global events by using a gameplaytag as key
	*/
	UPROPERTY()
	TMap<FGameplayTag, FRPGOnEventCalledMulticast> ListenerEventsByGameplayTag;

public:
	/**
	* Call (Publish) a global event using a TMWEventBase class as filter/signature. Any UObject bound (Subscribed) to this class will receive this message.
	* Create childrens of TMWEventBase before using this method.
	* @param EventClass - Class used as signature/filter. The class must inherit from TMWEventBase.
	* @param Payload - Instance of UObject that will be passed as message. I recommend to use an instance of the same EventClass for easy debugging.
	* @param Metadata - Simple metadata that can be passed in the message.
	*/
	UFUNCTION(BlueprintCallable, Category = "TMW|GlobalEvent", meta = (DisplayName = "RPGEvent::Call Global Event By Class", DefaultToSelf = "Publisher", 
		HidePin = "Publisher", AdvancedDisplay = "Payload,Metadata", AutoCreateRefTerm = "Metadata", Keywords = "Publish"))
	void CallGlobalEventByClass(UObject* Publisher, TSubclassOf<URPGEventBase> EventClass, UObject* Payload, const TArray<FString>& Metadata);

		/**
	* Call (Publish) a global event using a Name as filter. Any UObject bound (Subscribed) to this class will receive this message.
	* Recommended to use a data table or string table to make sure the names match. Names are not case sensitive!
	* @param EventName - Name used as signature/filter.
	* @param Payload - Instance of UObject that will be passed as message.
	* @param Metadata - Simple metadata that can be passed in the message.
	*/
	UFUNCTION(BlueprintCallable, Category = "TMW|GlobalEvent", meta = (DisplayName = "RPGEvent::Call Global Event By Name", DefaultToSelf = "Publisher", 
		HidePin = "Publisher", AdvancedDisplay = "Payload,Metadata", AutoCreateRefTerm = "Metadata", Keywords = "Publish"))
		void CallGlobalEventByName(UObject* Publisher, FName EventName, UObject* Payload, const TArray<FString>& Metadata);
	/**
	* Call (Publish) a global event using a GameplayTag as filter. Any UObject bound (Subscribed) to this class will receive this message.
	* Recommended to use a data table or string table to make sure the names match. Names are not case sensitive!
	* @param EventGameplayTag - GameplayTag used as signature/filter.
	* @param Payload - Instance of UObject that will be passed as message.
	* @param Metadata - Simple metadata that can be passed in the message.
	*/
	UFUNCTION(BlueprintCallable, Category = "TMW|GlobalEvent", meta = (DisplayName = "RPGEvent::Call Global Event By GameplayTag", DefaultToSelf = "Publisher",
		HidePin = "Publisher", AdvancedDisplay = "Payload,Metadata", AutoCreateRefTerm = "Metadata", Keywords = "Publish"))
		void CallGlobalEventByGameplayTag(UObject* Publisher, FGameplayTag EventGameplayTag, UObject* Payload, const TArray<FString>& Metadata);

	/**
	* Bind (Subscribe) a local event to a global event using a class as filter/signature. 
	* If a message is called/published using the same signature, the event linked to this method bill be triggered.
	* @param EventClass - Class used as signature/filter. The class must inherit from TMWEventBase.
	* @param Event - Local Event linked to the global event.
	*/
	UFUNCTION(BlueprintCallable, Category = "TMW|GlobalEvent", meta = (DisplayName = "RPGEvent::Bind Global Event By Class", Keywords = "Subscribe"))
		void BindGlobalEventByClass(TSubclassOf<URPGEventBase> EventClass, const FRPGOnEventCalledSingle& Event);
	/**
	* Bind (Subscribe) a local event to a global event using a Name as filter/signature.
	* If a message is called/published using the same signature, the event linked to this method bill be triggered.
	* @param EventName - Name used as signature/filter.
	* @param Event - Local Event linked to the global event.
	*/
	UFUNCTION(BlueprintCallable, Category = "TMW|GlobalEvent", meta = (DisplayName = "RPGEvent::Bind Global Event By Name", Keywords = "Subscribe"))
		void BindGlobalEventByName(FName EventName, const FRPGOnEventCalledSingle& Event);
	/**
	* Bind (Subscribe) a local event to a global event using a GameplayTag as filter/signature.
	* If a message is called/published using the same signature, the event linked to this method bill be triggered.
	* @param EventGameplayTag - GameplayTag used as signature/filter.
	* @param Event - Local Event linked to the global event.
	*/
	UFUNCTION(BlueprintCallable, Category = "TMW|GlobalEvent", meta = (DisplayName = "RPGEvent::Bind Global Event By GameplayTag", Keywords = "Subscribe"))
		void BindGlobalEventByGameplayTag(FGameplayTag EventGameplayTag, const FRPGOnEventCalledSingle& Event);

	/**
 	* Unbind (Unsubscribe) a local event to a global event using a TMWEventBase class as filter/signature.
 	* @param EventClass - Class used as signature/filter. The class must inherit from TMWEventBase.
 	* @param Event - Local Event linked to the global event.
 	*/
	UFUNCTION(BlueprintCallable, Category = "TMW|GlobalEvent", meta = (DisplayName = "RPGEvent::Unbind Global Event By Class", Keywords = "Unsubscribe"))
		void UnbindGlobalEventByClass(TSubclassOf<URPGEventBase> EventClass, const FRPGOnEventCalledSingle& Event);
	/**
	* Unbind (Unsubscribe) a local event to a global event using a Name class as filter/signature.
	* @param EventName - Name used as signature/filter.
	* @param Event - Local Event linked to the global event.
	*/
	UFUNCTION(BlueprintCallable, Category = "TMW|GlobalEvent", meta = (DisplayName = "RPGEvent::Unbind Global Event By Name", Keywords = "Unsubscribe"))
		void UnbindGlobalEventByName(FName EventName, const FRPGOnEventCalledSingle& Event);
	/**
	* Unbind (Unsubscribe) a local event to a global event using a GameplayTag class as filter/signature.
	* @param EventGameplayTag - Name used as signature/filter.
	* @param Event - Local Event linked to the global event.
	*/
	UFUNCTION(BlueprintCallable, Category = "TMW|GlobalEvent", meta = (DisplayName = "RPGEvent::Unbind Global Event By GameplayTag", Keywords = "Unsubscribe"))
		void UnbindGlobalEventByGameplayTag(FGameplayTag EventGameplayTag, const FRPGOnEventCalledSingle& Event);

	/**
	* Unbind (Unsubscribe) all the local events using a TMWEventBase class as filter/signature.
	* NOTE: Only unbinds events from the object calling this method.
	* @param EventClass - Class used as signature/filter. The class must inherit from TMWEventBase.
	*/
	UFUNCTION(BlueprintCallable, Category = "TMW|GlobalEvent", meta = (DisplayName = "RPGEvent::Unbind All Global Events By Class", DefaultToSelf = "Unsubscriber", 
		HidePin = "Unsubscriber", Keywords = "Unsubscribe All"))
		void UnbindAllGlobalEventsByClass(UObject* Unsubscriber, TSubclassOf<URPGEventBase> EventClass);
	/**
	* Unbind (Unsubscribe) all the local events using a Name as filter/signature.
	* NOTE: Only unbinds events from the object calling this method.
	* @param EventName - Name used as signature/filter.
	*/
	UFUNCTION(BlueprintCallable, Category = "TMW|GlobalEvent", meta = (DisplayName = "RPGEvent::Unbind All Global Events By Name", DefaultToSelf = "Unsubscriber",
		HidePin = "Unsubscriber", Keywords = "Unsubscribe All"))
		void UnbindAllGlobalEventsByName(UObject* Unsubscriber, FName EventName);
	/**
	* Unbind (Unsubscribe) all the local events using a GameplayTag as filter/signature.
	* NOTE: Only unbinds events from the object calling this method.
	* @param EventGameplayTag - GameplayTag used as signature/filter.
	*/
	UFUNCTION(BlueprintCallable, Category = "TMW|GlobalEvent", meta = (DisplayName = "RPGEvent::Unbind All Global Events By GameplayTag", DefaultToSelf = "Unsubscriber",
		HidePin = "Unsubscriber", Keywords = "Unsubscribe All"))
		void UnbindAllGlobalEventsByGameplayTag(UObject* Unsubscriber, FGameplayTag EventGameplayTag);

	/**
	* Get all global events that are currently active using a TMWEventBase class as filter/signature.
	* GetAllBoundObjectsToEventByClass() can be called afterward to see the subscribed objects/events associated to the global event.
	* -----------------------------------------------------------------------------------------------------------------------------------------
	* Active global events can have 0 subscribed objects associated, those can be cleaned by calling ClearGlobalEventByClass(ForceClear=false).
	* However these events that have no subscribed objects should not cause any kind of issue, unless there are thousands (they'll use some space in memory).
	*/
	UFUNCTION(BlueprintPure, Category = "TMW|GlobalEvent", meta = (DisplayName = "RPGEvent::Get All Active Global Events By Class"))
		void GetAllActiveGlobalEventsClass(TArray<TSubclassOf<URPGEventBase>>& Events);
	/**
	* Get all global events that are currently active using a Name as filter/signature.
	* GetAllBoundObjectsToEventByName() can be called afterward to see the subscribed objects/events associated to the global event.
	* -----------------------------------------------------------------------------------------------------------------------------------------
	* Active global events can have 0 subscribed objects associated, those can be cleaned by calling ClearGlobalEventByName(ForceClear=false).
	* However these events that have no subscribed objects should not cause any kind of issue, unless there are thousands (they'll use some space in memory).
	*/
	UFUNCTION(BlueprintPure, Category = "TMW|GlobalEvent", meta = (DisplayName = "RPGEvent::Get All Active Global Events By Name"))
		void GetAllActiveGlobalEventNames(TArray<FName>& Events);
	/**
	* Get all global events that are currently active using a GameplayTag as filter/signature.
	* GetAllBoundObjectsToEventByGameplayTag() can be called afterward to see the subscribed objects/events associated to the global event.
	* -----------------------------------------------------------------------------------------------------------------------------------------
	* Active global events can have 0 subscribed objects associated, those can be cleaned by calling ClearGlobalEventByGameplayTag(ForceClear=false).
	* However these events that have no subscribed objects should not cause any kind of issue, unless there are thousands (they'll use some space in memory).
	*/
	UFUNCTION(BlueprintPure, Category = "TMW|GlobalEvent", meta = (DisplayName = "RPGEvent::Get All Active Global Events By GameplayTag"))
		void GetAllActiveGlobalEventGameplayTag(TArray<FGameplayTag>& Events);

	/**
	* Get all bound/subscribed UObjects to a global event using a TMWEventBase class as filter/signature.
	* An object can subscribe to a global event by calling BindGlobalEventByClass()
	* @param EventClass - Class used as signature/filter. The class must inherit from TMWEventBase.
	*/
	UFUNCTION(BlueprintPure, Category = "TMW|GlobalEvent", meta = (DisplayName = "RPGEvent::Get All Bound Objects To Event By Class"))
		void GetAllBoundObjectsToEventByClass(TSubclassOf<URPGEventBase> EventClass, TArray<UObject*>& Objects);
	/**
	* Get all bound/subscribed UObjects to a global event using a Name as filter/signature.
	* An object can subscribe to a global event by calling BindGlobalEventByName()
	* @param EventName - Name used as signature/filter.
	*/
	UFUNCTION(BlueprintPure, Category = "TMW|GlobalEvent", meta = (DisplayName = "RPGEvent::Get All Bound Objects To Event By Name"))
		void GetAllBoundObjectsToEventByName(FName EventName, TArray<UObject*>& Objects);
	/**
	* Get all bound/subscribed UObjects to a global event using a GameplayTag as filter/signature.
	* An object can subscribe to a global event by calling BindGlobalEventByGameplayTag()
	* @param EventGameplayTag - GameplayTag used as signature/filter.
	*/
	UFUNCTION(BlueprintPure, Category = "TMW|GlobalEvent", meta = (DisplayName = "RPGEvent::Get All Bound Objects To Event By GameplayTag"))
		void GetAllBoundObjectsToEventByGameplayTag(FGameplayTag EventGameplayTag, TArray<UObject*>& Objects);

	/**
	* Clear a global event using a TMWEventBase class as filter/signature.
	* By default, the event will be cleared from the stack only if the event has 0 subscribed objects.
	* However, if bForceClear is set to true. The global event and its subscriptions will be cleared.
	* -----------------------------------------------------------------------------------------------
	* This method is mostly useful for debugging or for clearing events that have 0 subscribed objects (in case the little memory they use are needed).
	* @param EventClass - Class used as signature/filter. The class must inherit from TMWEventBase.
	* @param bForceClear - If true, will clear the global event even if it has subscribed objects.
	*/
	UFUNCTION(BlueprintCallable, Category = "TMW|GlobalEvent", meta = (DisplayName = "RPGEvent::Clear Global Event By Class", AdvancedDisplay = "bForceClear"))
		void ClearGlobalEventByClass(TSubclassOf<URPGEventBase> EventClass, const bool bForceClear = false);
	/**
	* Clear a global event using a Name as filter/signature.
	* By default, the event will be cleared from the stack only if the event has 0 subscribed objects.
	* However, if bForceClear is set to true. The global event and its subscriptions will be cleared.
	* -----------------------------------------------------------------------------------------------
	* This method is mostly useful for debugging or for clearing events that have 0 subscribed objects (in case the little memory they use are needed).
	* @param EventName - Name used as signature/filter.
	* @param bForceClear - If true, will clear the global event even if it has subscribed objects.
	*/
	UFUNCTION(BlueprintCallable, Category = "TMW|GlobalEvent", meta = (DisplayName = "RPGEvent::Clear Global Event By Name", AdvancedDisplay = "bForceClear"))
		void ClearGlobalEventByName(FName EventName, const bool bForceClear = false);
	/**
	* Clear a global event using a GameplayTag as filter/signature.
	* By default, the event will be cleared from the stack only if the event has 0 subscribed objects.
	* However, if bForceClear is set to true. The global event and its subscriptions will be cleared.
	* -----------------------------------------------------------------------------------------------
	* This method is mostly useful for debugging or for clearing events that have 0 subscribed objects (in case the little memory they use are needed).
	* @param EventGameplayTag - GameplayTag used as signature/filter.
	* @param bForceClear - If true, will clear the global event even if it has subscribed objects.
	*/
	UFUNCTION(BlueprintCallable, Category = "TMW|GlobalEvent", meta = (DisplayName = "RPGEvent::Clear Global Event By GameplayTag", AdvancedDisplay = "bForceClear"))
		void ClearGlobalEventByGameplayTag(FGameplayTag EventGameplayTag, const bool bForceClear = false);

	/**
	* Clear all the global events by class in the subsystem.
	* @see ClearGlobalEventByClass() for more information.
	* -----------------------------------------------------------------------------------------------
	* This method is mostly useful for debugging or for clearing events that have 0 subscribed objects (in case the little memory they use is needed).
	* @param bForceClear - If true, will clear the global events even if they have subscribed objects.
	*/
	UFUNCTION(BlueprintCallable, Category = "TMW|GlobalEvent", meta = (DisplayName = "RPGEvent::Clear All Global Events By Class", AdvancedDisplay = "bForceClear"))
		void ClearAllGlobalEventsClass(const bool bForceClear = false);
	/**
	* Clear all the global events by name in the subsystem.
	* @see ClearGlobalEventByName() for more information.
	* -----------------------------------------------------------------------------------------------
	* This method is mostly useful for debugging or for clearing events that have 0 subscribed objects (in case the little memory they use is needed).
	* @param bForceClear - If true, will clear the global events even if they have subscribed objects.
	*/
	UFUNCTION(BlueprintCallable, Category = "TMW|GlobalEvent", meta = (DisplayName = "RPGEvent::Clear All Global Event By Name", AdvancedDisplay = "bForceClear"))
		void ClearAllGlobalEventNames(const bool bForceClear = false);
	/**
	* Clear all the global events by GameplayTag in the subsystem.
	* @see ClearGlobalEventByGameplayTag() for more information.
	* -----------------------------------------------------------------------------------------------
	* This method is mostly useful for debugging or for clearing events that have 0 subscribed objects (in case the little memory they use is needed).
	* @param bForceClear - If true, will clear the global events even if they have subscribed objects.
	*/
	UFUNCTION(BlueprintCallable, Category = "TMW|GlobalEvent", meta = (DisplayName = "RPGEvent::Clear All Global Events By GameplayTag", AdvancedDisplay = "bForceClear"))
		void ClearAllGlobalEventGameplayTags(const bool bForceClear = false);

	static UGlobalEventHandler* Get(const UObject* WorldContextObject);
private:
	/**
	* Call a global event by name or by class
	* @see UnrealTemplate.h for more info about this code
	*/
	template <typename TDictionary, typename TKey>
	static FORCEINLINE void CallGlobalEvent(UObject* Publisher, UObject* Payload, const TArray<FString>& Metadata, const TDictionary& Dictionary, const TKey& DictionaryKey)
	{
		if (Dictionary.Contains(DictionaryKey))
		{
			//Broadcast the multicast event found in the dictionary using the key
			Dictionary.Find(DictionaryKey)->Broadcast(Publisher, Payload, Metadata);
		}
		//#TODO If ListenerEventsByClass.Find(EventClass)->InvocationList; ever gets public, use it to filter the publish method.
		//For example. Filter ListenerEventsByClass.Find(EventClass)->GetObjects(); for tag, gameplay tag, interface, class, component
		//Then use InvocationList to manually invoke the event in the object (by index?)
		
		//#INFO If the code below is compiled, it will trigger an error. Unless this method is only called by Name.
		//This means that the Compiler is generating this method for each usage of the method only, meaning that has good performance.
		//auto x = DictionaryKey.IsNone();
	};

	/**
	* Unbind to a global event by name or by class
	*/
	template <typename TDictionary, typename TKey>
	static FORCEINLINE void UnbindGlobalEvent(const FRPGOnEventCalledSingle& Event, TDictionary& Dictionary, const TKey& DictionaryKey)
	{
		//Remove OnListenResponse from multicast linked to EventClass
		if (Dictionary.Contains(DictionaryKey))
		{
			Dictionary.Find(DictionaryKey)->Remove(Event);
		}
	};

	/**
	* Unbind to all global event by name or class
	*/
	template <typename TDictionary, typename TKey>
	static FORCEINLINE void UnbindAllGlobalEvents(UObject* Unsubscriber, TDictionary& Dictionary, const TKey& DictionaryKey)
	{
		//Remove all events linked to Unsubscriber from multicast linked to ClassToListen
		if (Dictionary.Contains(DictionaryKey))
		{
			Dictionary.Find(DictionaryKey)->RemoveAll(Unsubscriber);
		}
	};

	/**
	* Get subscribed objects to a global event by name or class
	*/
	template <typename TDictionary, typename TKey>
	static FORCEINLINE void GetAllBoundObjectsToEvent(TArray<UObject*>& Objects, TDictionary& Dictionary, const TKey& DictionaryKey)
	{
		//Get all objects linked to the multicast
		if (Dictionary.Contains(DictionaryKey))
		{
			Objects = Dictionary.Find(DictionaryKey)->GetAllObjects();
		}
	};

	/**
	* Clear a global event by name or class
	*/
	template <typename TDictionary, typename TKey>
	static FORCEINLINE void ClearGlobalEvent(const bool bForceClear, TDictionary& Dictionary, const TKey& DictionaryKey)
	{
		if (Dictionary.Contains(DictionaryKey))
		{
			if (bForceClear)
			{
				//Clear the multicast listener if forceClear is true
				Dictionary.Find(DictionaryKey)->Clear(); //Move to inline method if code grows
				Dictionary.Remove(DictionaryKey);
			}
			else
			{
				//Else, clear it only if SubscribedObjects = 0
				if (Dictionary.Find(DictionaryKey)->GetAllObjects().Num() == 0)
				{
					Dictionary.Find(DictionaryKey)->Clear(); //Move to inline method if code grows
					Dictionary.Remove(DictionaryKey);
				}
			}
		}
	};
};
