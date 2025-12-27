// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Data/Fragment/ItemFragment_Questable.h"

bool UItemFragment_Questable::IsRelatedToQuest(int32 QuestID) const
{
	return RelatedQuestIDs.Contains(QuestID);
}
