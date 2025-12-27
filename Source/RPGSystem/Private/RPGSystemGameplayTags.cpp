#include "RPGSystemGameplayTags.h"

namespace RPGGameplayTags
{
    /* Input */
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Action_Move, "Input.Action.Move", "Character Movement Input");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Action_Look, "Input.Action.Look", "Camera Look Input");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Action_Walk, "Input.Action.Walk", "Walk Input");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Action_Jump, "Input.Action.Jump", "Jump Input");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Action_Interact, "Input.Action.Interact", "Interaction Input");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Action_Crouch, "Input.Action.Crouch", "Crouch Input");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Action_Sprint, "Input.Action.Sprint", "Sprint Input");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Action_Attack, "Input.Action.Attack", "Attack Input");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Action_Block, "Input.Action.Block", "Block Input");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Action_Dodge, "Input.Action.Dodge", "Dodge/Roll Input");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Action_Primary, "Input.Action.Primary","Left Click Input");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Action_Secondary, "Input.Action.Secondary","Right Click Input");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Action_Tertiary, "Input.Action.Tertiary","Middle Click Input");

    /* Action */
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Action_Primary, "Action.Slot.Primary", "Primary Action Slot (Mapped to LMB)");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Action_Secondary, "Action.Slot.Secondary", "Secondary Action Slot (Mapped to RMB)");
    /* Attributes */
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Vital_Health, "Attribute.Vital.Health", "Current Health");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Vital_Mana, "Attribute.Vital.Mana", "Current Mana");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Vital_Stamina, "Attribute.Vital.Stamina", "Current Stamina");

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Secondary_MaxHealth, "Attribute.Secondary.MaxHealth", "Maximum Health");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Secondary_MaxMana, "Attribute.Secondary.MaxMana", "Maximum Mana");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Secondary_MaxStamina, "Attribute.Secondary.MaxStamina", "Maximum Stamina");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Secondary_Armor, "Attribute.Secondary.Armor", "Armor Rating");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Secondary_Damage, "Attribute.Secondary.Damage", "Attack Damage");

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Primary_Strength, "Attribute.Primary.Strength", "Strength Stat");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Primary_Agility, "Attribute.Primary.Agility", "Agility Stat");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Primary_Intelligence, "Attribute.Primary.Intelligence", "Intelligence Stat");

    /* Items */
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Type_Weapon, "Item.Type.Weapon", "Weapon Item");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Type_Armor, "Item.Type.Armor", "Armor Item");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Type_Consumable, "Item.Type.Consumable", "Consumable Item");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Type_Quest, "Item.Type.Quest", "Quest Item");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Type_Material, "Item.Type.Material", "Crafting Material");

    /* Equipment Slots */
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Equipment_Slot_Head,     "Equipment.Slot.Head",     "Head Slot");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Equipment_Slot_Neck,     "Equipment.Slot.Neck",     "Neck Slot");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Equipment_Slot_Shoulder, "Equipment.Slot.Shoulder", "Shoulder Slot");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Equipment_Slot_Back,     "Equipment.Slot.Back",     "Back/Cloak Slot");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Equipment_Slot_Chest,    "Equipment.Slot.Chest",    "Chest Slot");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Equipment_Slot_Wrist,    "Equipment.Slot.Wrist",    "Wrist Slot");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Equipment_Slot_Hands,    "Equipment.Slot.Hands",    "Hand Slot");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Equipment_Slot_Waist,    "Equipment.Slot.Waist",    "Waist/Belt Slot");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Equipment_Slot_Legs,     "Equipment.Slot.Legs",     "Leg Slot");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Equipment_Slot_Feet,     "Equipment.Slot.Feet",     "Feet Slot");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Equipment_Slot_Ring,     "Equipment.Slot.Ring",     "Ring Slot");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Equipment_Slot_Trinket,  "Equipment.Slot.Trinket",  "Trinket Slot");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Equipment_Slot_MainHand, "Equipment.Slot.MainHand", "Main Hand Slot");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Equipment_Slot_OffHand,  "Equipment.Slot.OffHand",  "Off Hand Slot");
    
    UE_DEFINE_GAMEPLAY_TAG(Item_Weapon_Bow, "Item.Weapon.Bow");
    UE_DEFINE_GAMEPLAY_TAG(Item_Weapon_GreatSword, "Item.Weapon.GreatSword");
    UE_DEFINE_GAMEPLAY_TAG(Item_Weapon_Staff, "Item.Weapon.Staff");
    UE_DEFINE_GAMEPLAY_TAG(Item_Weapon_SwordShield, "Item.Weapon.SwordShield");

    /* State */
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Dead, "State.Dead", "Actor is Dead");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Stunned, "State.Stunned", "Actor is Stunned");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Invincible, "State.Invincible", "Actor is Invincible");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Interacting, "State.Interacting", "Actor is currently interacting");

    /* Event */
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_Montage_NextCombo, "Event.Montage.NextCombo", "Trigger for Next Combo Window");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_Hit_Melee, "Event.Hit.Melee", "Melee Hit Event");
};