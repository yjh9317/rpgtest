// Source/RPGSystemEditor/Private/Debugger/SQuestDebugger.cpp

#include "Debugger/SQuestDebugger.h"
#include "Quest/Components/QuestManagerComponent.h" // 경로 확인 필요
#include "Quest/RPGQuest.h" // 경로 확인 필요
#include "Widgets/Layout/SScrollBox.h"
#include "Editor/UnrealEdEngine.h"
#include "UnrealEdGlobals.h"
#include "Engine/Selection.h"

#define LOCTEXT_NAMESPACE "SQuestDebugger"

void SQuestDebugger::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBorder)
		.Padding(4.0f)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot().Padding(5.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 10)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Title", "Quest System Inspector"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
					.Justification(ETextJustify::Center)
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SAssignNew(ContentBox, SVerticalBox)
				]
			]
		]
	];
}

void SQuestDebugger::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	CheckSelection();
	
	// Quest 상태는 자주 바뀌지 않으므로 0.5초마다 갱신하거나, 필요할 때만 갱신
	UpdateCounter++;
	if (UpdateCounter % 30 == 0 && CachedQuestComp.IsValid()) 
	{
		RebuildContent();
	}
}

void SQuestDebugger::CheckSelection()
{
	AActor* SelectedActor = nullptr;
	if (GEditor && GEditor->GetSelectedActors())
	{
		for (FSelectionIterator It(*GEditor->GetSelectedActors()); It; ++It)
		{
			if (AActor* Actor = Cast<AActor>(*It))
			{
				SelectedActor = Actor;
				break;
			}
		}
	}

	UQuestManagerComponent* NewComp = SelectedActor ? SelectedActor->FindComponentByClass<UQuestManagerComponent>() : nullptr;
	if (NewComp != CachedQuestComp.Get())
	{
		CachedQuestComp = NewComp;
		RebuildContent();
	}
}

void SQuestDebugger::RebuildContent()
{
	ContentBox->ClearChildren();

	if (!CachedQuestComp.IsValid())
	{
		ContentBox->AddSlot().AutoHeight().Padding(5)
		[
			SNew(STextBlock).Text(LOCTEXT("NoComp", "Select Actor with QuestManagerComponent"))
			.Justification(ETextJustify::Center).ColorAndOpacity(FLinearColor::Gray)
		];
		return;
	}

	// === Header ===
	ContentBox->AddSlot().AutoHeight()
	[
		MakeSectionHeader(FText::FromString(CachedQuestComp->GetOwner()->GetActorLabel()), FLinearColor(0.2f, 1.0f, 0.2f))
	];

	// === Active Quests ===
	// "ActiveQuests"라는 멤버 변수 이름(TArray<URPGQuest*>)을 가정합니다.
	DrawQuestList(TEXT("ACTIVE QUESTS"), TEXT("ActiveQuests"), FLinearColor(1.0f, 0.8f, 0.2f));

	// === Completed Quests ===
	// "CompletedQuests"라는 멤버 변수 이름(TArray<URPGQuest*>)을 가정합니다.
	DrawQuestList(TEXT("COMPLETED QUESTS"), TEXT("CompletedQuests"), FLinearColor(0.6f, 0.6f, 0.6f));
}

void SQuestDebugger::DrawQuestList(const FString& Title, const FName& PropertyName, FLinearColor TitleColor)
{
	ContentBox->AddSlot().AutoHeight().Padding(0, 10, 0, 5)
	[
		MakeSectionHeader(FText::FromString(Title), TitleColor)
	];

	// Reflection을 사용하여 TArray<URPGQuest*> 가져오기
	FArrayProperty* ArrayProp = CastField<FArrayProperty>(CachedQuestComp->GetClass()->FindPropertyByName(PropertyName));
	if (!ArrayProp)
	{
		ContentBox->AddSlot().AutoHeight().Padding(5) [ SNew(STextBlock).Text(FText::FromString(TEXT("Property Not Found: ") + PropertyName.ToString())).ColorAndOpacity(FLinearColor::Red) ];
		return;
	}

	FScriptArrayHelper Helper(ArrayProp, ArrayProp->ContainerPtrToValuePtr<void>(CachedQuestComp.Get()));
	if (Helper.Num() == 0)
	{
		ContentBox->AddSlot().AutoHeight().Padding(5) [ SNew(STextBlock).Text(LOCTEXT("Empty", "None")).ColorAndOpacity(FLinearColor::Gray) ];
		return;
	}

	for (int32 i = 0; i < Helper.Num(); ++i)
	{
		// 배열 내부의 UObject* 포인터 가져오기
		UObject* QuestObj = nullptr;
		if (FObjectProperty* InnerProp = CastField<FObjectProperty>(ArrayProp->Inner))
		{
			QuestObj = InnerProp->GetObjectPropertyValue(Helper.GetRawPtr(i));
		}

		if (QuestObj)
		{
			FString QuestName = QuestObj->GetName();
			
			// URPGQuest의 "QuestName"이나 "QuestTitle" FText 프로퍼티가 있다면 가져오기
			if (FTextProperty* TitleProp = CastField<FTextProperty>(QuestObj->GetClass()->FindPropertyByName(TEXT("QuestTitle"))))
			{
				QuestName = TitleProp->GetPropertyValue_InContainer(QuestObj).ToString();
			}

			ContentBox->AddSlot().AutoHeight().Padding(2)
			[
				SNew(SBorder)
				.Padding(FMargin(5.0f))
				.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock).Text(FText::FromString(QuestName)).ColorAndOpacity(FLinearColor::White).Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
					]
					// 여기에 세부 목표(Objectives) 리스트를 추가로 순회하여 표시 가능
				]
			];
		}
	}
}

TSharedRef<SWidget> SQuestDebugger::MakeSectionHeader(const FText& HeaderText, FLinearColor Color)
{
	return SNew(SBorder).Padding(FMargin(5.0f, 2.0f)).BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
		[ SNew(STextBlock).Text(HeaderText).ColorAndOpacity(Color).Font(FCoreStyle::GetDefaultFontStyle("Bold", 10)) ];
}

#undef LOCTEXT_NAMESPACE