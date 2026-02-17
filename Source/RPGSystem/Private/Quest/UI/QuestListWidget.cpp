#include "Quest/UI/QuestListWidget.h"

#include "Components/ScrollBox.h"
#include "Quest/Data/RPGQuestData.h"
#include "Quest/UI/QuestEntryWidget.h"

void UQuestListWidget::RefreshList(const TArray<URPGQuestData*>& Quests)
{
	if (!QuestListContainer || !QuestEntryClass)
	{
		return;
	}

	QuestListContainer->ClearChildren();
	SelectedEntryWidget = nullptr;

	for (URPGQuestData* QuestData : Quests)
	{
		if (!QuestData)
		{
			continue;
		}

		UQuestEntryWidget* EntryWidget = CreateWidget<UQuestEntryWidget>(this, QuestEntryClass);
		if (!EntryWidget)
		{
			continue;
		}

		EntryWidget->SetupEntry(QuestData);
		EntryWidget->OnEntryClicked.AddUniqueDynamic(this, &UQuestListWidget::HandleEntrySelected);
		EntryWidget->SetIsSelected(false);
		QuestListContainer->AddChild(EntryWidget);
	}

	if (QuestListContainer->GetChildrenCount() > 0)
	{
		if (UQuestEntryWidget* FirstEntry = Cast<UQuestEntryWidget>(QuestListContainer->GetChildAt(0)))
		{
			HandleEntrySelected(FirstEntry->MyQuestData);
		}
	}
}

void UQuestListWidget::HandleEntrySelected(URPGQuestData* SelectedQuest)
{
	if (!QuestListContainer || !SelectedQuest)
	{
		return;
	}

	SelectedEntryWidget = nullptr;

	const int32 ChildCount = QuestListContainer->GetChildrenCount();
	for (int32 Index = 0; Index < ChildCount; ++Index)
	{
		UQuestEntryWidget* EntryWidget = Cast<UQuestEntryWidget>(QuestListContainer->GetChildAt(Index));
		if (!EntryWidget)
		{
			continue;
		}

		const bool bIsTarget = (EntryWidget->MyQuestData == SelectedQuest);
		EntryWidget->SetIsSelected(bIsTarget);
		if (bIsTarget)
		{
			SelectedEntryWidget = EntryWidget;
		}
	}

	if (SelectedEntryWidget)
	{
		OnQuestSelected.Broadcast(SelectedQuest);
	}
}
