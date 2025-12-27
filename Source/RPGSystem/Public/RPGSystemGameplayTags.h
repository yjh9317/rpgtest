// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

namespace RPGGameplayTags
{
	/* ==================================================
	* Input
	* ================================================== */
	RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Move);       
    RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Look);
	RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Walk);
    RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Jump);    
    RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Interact);
    RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Crouch);  
    RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Sprint);  
    RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Attack);  
    RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Block);   
    RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Dodge);   
	RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Primary);   // LMB
	RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Secondary); // RMB
	RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Tertiary);  // MMB
	
	/* ==================================================
	* Action
	* ================================================== */
	RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Primary);  
	RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Secondary);
    /* ==================================================
     * Attribute Tags (스탯/속성)
     * ================================================== */
    // Vital (현재 상태값)
    RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Vital_Health);        
    RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Vital_Mana);          
    RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Vital_Stamina);       

    // Secondary (최대치 및 파생 스탯)
    RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Secondary_MaxHealth); 
    RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Secondary_MaxMana);   
    RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Secondary_MaxStamina);
    RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Secondary_Armor);     
    RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Secondary_Damage);    

    // Primary (기초 스탯)
    RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Primary_Strength);    
    RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Primary_Agility);     
    RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Primary_Intelligence);

    /* ==================================================
     * Item Tags (아이템 분류 및 슬롯)
     * ================================================== */
    // Item Type
    RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Type_Weapon);      
    RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Type_Armor);       
    RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Type_Consumable);  
    RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Type_Quest);       
    RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Type_Material);    

	RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot_Head);
	RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot_Neck);
	RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot_Shoulder); 
	RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot_Back);     
	RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot_Chest);       
	RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot_Wrist);    
	RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot_Hands);       
	RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot_Waist);    
	RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot_Legs);        
	RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot_Feet);        
	RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot_Ring);     
	RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot_Trinket);  
	RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot_MainHand);    
	RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot_OffHand);
    /* ==================================================
     * State Tags (상태이상, 행동 상태)
     * ================================================== */
    RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dead);            
    RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Stunned);         
    RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Invincible);      
    RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Interacting);     

    /* ==================================================
     * Event Tags (이벤트 트리거용)
     * ================================================== */
    RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_NextCombo);
    RPGSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Hit_Melee);       
}
