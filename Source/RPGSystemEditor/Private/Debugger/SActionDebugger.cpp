// Source/RPGSystemEditor/Private/Debugger/SActionDebugger.cpp

#include "Debugger/SActionDebugger.h"

// Game Classes
#include "Combat/Action/Components/ActionComponent.h"
#include "Combat/Action/BaseAction.h"
#include "GameplayTagContainer.h" // GameplayTag 처리를 위해 필수

// Engine / Editor Headers
#include "Editor/UnrealEdEngine.h"
#include "UnrealEdGlobals.h"
#include "Engine/Selection.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Input/SCheckBox.h"

#define LOCTEXT_NAMESPACE "SActionDebugger"

void SActionDebugger::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBorder)
		.Padding(4.0f)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			.Padding(5.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 10)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Title", "Action System Debugger"))
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

void SActionDebugger::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	
	CheckSelection();

	if (CachedActionComp.IsValid())
	{
		int32 CurrentTagNum = CachedActionComp->GetActiveTags().Num();
		int32 CurrentActionNum = CachedActionComp->GetRegisteredActionTags().Num();

		if (CurrentTagNum != LastTagCount || CurrentActionNum != LastActionCount)
		{
			RebuildContent();
		}
	}
}

void SActionDebugger::CheckSelection()
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

	UActionComponent* NewComp = nullptr;
	if (SelectedActor)
	{
		NewComp = SelectedActor->FindComponentByClass<UActionComponent>();
	}

	if (NewComp != CachedActionComp.Get())
	{
		CachedActionComp = NewComp;
		LastTagCount = -1; // 강제 리빌드 트리거
		RebuildContent();
	}
}

void SActionDebugger::RebuildContent()
{
	ContentBox->ClearChildren();
	LastTagCount = 0;
	LastActionCount = 0;

	if (!CachedActionComp.IsValid())
	{
		ContentBox->AddSlot().AutoHeight().Padding(5)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("NoComp", "Select an Actor with ActionComponent"))
			.ColorAndOpacity(FLinearColor::Gray)
			.Justification(ETextJustify::Center)
		];
		return;
	}

	FGameplayTagContainer ActiveTags = CachedActionComp->GetActiveTags();
	LastTagCount = ActiveTags.Num();
	LastActionCount = CachedActionComp->GetRegisteredActionTags().Num();

	// === 1. Target Name ===
	ContentBox->AddSlot().AutoHeight()
	[
		MakeSectionHeader(FText::FromString(CachedActionComp->GetOwner()->GetActorLabel()), FLinearColor(0.2f, 1.0f, 0.2f))
	];

	// === 2. Gameplay Tags ===
	ContentBox->AddSlot().AutoHeight().Padding(0, 10, 0, 5)
	[
		MakeSectionHeader(FText::Format(LOCTEXT("TagHeader", "ACTIVE TAGS ({0})"), FText::AsNumber(ActiveTags.Num())), FLinearColor(1.0f, 0.4f, 0.8f))
	];
	DrawGameplayTags(ActiveTags, ContentBox);

	// === 3. Actions ===
	ContentBox->AddSlot().AutoHeight().Padding(0, 15, 0, 5)
	[
		MakeSectionHeader(LOCTEXT("ActionHeader", "ACTIONS"), FLinearColor(0.2f, 0.8f, 1.0f))
	];
	DrawActionsList(CachedActionComp.Get(), ContentBox);
}

void SActionDebugger::DrawGameplayTags(const FGameplayTagContainer& Tags, TSharedPtr<SVerticalBox> Container)
{
	if (Tags.Num() == 0)
	{
		Container->AddSlot().AutoHeight().Padding(5)
		[
			SNew(STextBlock).Text(LOCTEXT("NoTags", "No Active Tags")).ColorAndOpacity(FLinearColor::Gray)
		];
		return;
	}

	TSharedPtr<SWrapBox> TagWrapBox;
	Container->AddSlot().AutoHeight().Padding(2)
	[
		SAssignNew(TagWrapBox, SWrapBox)
		.UseAllottedSize(true) // [수정] UseAllottedWidth -> UseAllottedSize
	];

	for (const FGameplayTag& GameplayTag : Tags)
	{
		TagWrapBox->AddSlot().Padding(2)
		[
			SNew(SBorder)
			.Padding(FMargin(6.0f, 2.0f))
			.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
			[
				SNew(STextBlock)
				.Text(FText::FromString(GameplayTag.ToString()))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
				.ColorAndOpacity(FLinearColor::White)
			]
		];
	}
}

void SActionDebugger::DrawActionsList(UActionComponent* Comp, TSharedPtr<SVerticalBox> Container)
{
	TArray<FGameplayTag> ActionTags = Comp->GetRegisteredActionTags();
	if (ActionTags.Num() == 0)
	{
		Container->AddSlot().AutoHeight().Padding(5)
		[
			SNew(STextBlock).Text(LOCTEXT("NoActions", "No Actions Registered")).ColorAndOpacity(FLinearColor::Gray)
		];
		return;
	}

	for (const FGameplayTag& GameplayTag : ActionTags)
	{
		UBaseAction* Action = Comp->GetAction(GameplayTag);
		if (!Action) continue;

		bool bIsActive = Action->IsActive();
		FLinearColor TitleColor = bIsActive ? FLinearColor(0.4f, 1.0f, 0.4f) : FLinearColor(0.7f, 0.7f, 0.7f);
		FString StatusText = bIsActive ? TEXT("[RUNNING]") : TEXT("[Idle]");
		
		Container->AddSlot().AutoHeight().Padding(0, 5, 0, 0)
		[
			SNew(SBorder)
			.Padding(FMargin(5.0f))
			.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
			[
				SNew(SVerticalBox)
				// Action Title Line
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(STextBlock)
						.Text(FText::FromString(FString::Printf(TEXT("%s  %s"), *StatusText, *GameplayTag.ToString())))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
						.ColorAndOpacity(TitleColor)
					]
				]
				// Details
				+ SVerticalBox::Slot().AutoHeight().Padding(10, 2, 0, 0)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						MakeDebugRow(LOCTEXT("SrcLabel", "Source:"), Action->GetSourceObject() ? Action->GetSourceObject()->GetName() : TEXT("None"))
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						MakeDebugRow(LOCTEXT("TimeLabel", "Time Active:"), bIsActive ? FString::Printf(TEXT("%.2fs"), Action->GetTimeActive()) : TEXT("-"))
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						MakeDebugRow(LOCTEXT("CDLabel", "Cooldown Rem:"), FString::Printf(TEXT("%.2fs"), Action->GetCooldownRemaining()))
					]
				]
				// Reflection Properties
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 5, 0, 0)
				[
					SNew(SVerticalBox).Visibility(EVisibility::Visible) 
				]
			]
		];

		AddActionDetails(Action, Container);
	}
}

void SActionDebugger::AddActionDetails(UBaseAction* Action, TSharedPtr<SVerticalBox> Container)
{
	for (TFieldIterator<FProperty> PropIt(Action->GetClass()); PropIt; ++PropIt)
	{
		FProperty* Prop = *PropIt;
		if (Prop->HasAnyPropertyFlags(CPF_Transient | CPF_DisableEditOnInstance)) continue;
		if (Prop->GetName() == TEXT("ExecutePointer")) continue;
		if (Prop->GetOwnerClass() == UObject::StaticClass()) continue;

		Container->AddSlot().AutoHeight().Padding(20, 0, 5, 2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 10, 0)
			[
				SNew(STextBlock)
				.Text(FText::FromName(Prop->GetFName()))
				.MinDesiredWidth(120.0f)
				.ColorAndOpacity(FLinearColor::Gray)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 8))
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f)
			[
				MakePropertyWidget(Prop, Action)
			]
		];
	}
}

TSharedRef<SWidget> SActionDebugger::MakePropertyWidget(FProperty* Prop, UObject* Object)
{
	if (!Prop || !Object) return SNullWidget::NullWidget;
	
	TWeakObjectPtr<UObject> WeakObj = Object;

	// [수정] 1. FGameplayTag 처리 (StructProperty로 확인)
	if (FStructProperty* StructProp = CastField<FStructProperty>(Prop))
	{
		// 태그인지 확인 (이름으로 체크)
		if (StructProp->Struct->GetFName() == FName("GameplayTag"))
		{
			FGameplayTag* TagVal = StructProp->ContainerPtrToValuePtr<FGameplayTag>(Object);
			FString TagStr = TagVal ? TagVal->ToString() : TEXT("None");
			
			return SNew(STextBlock)
				.Text(FText::FromString(TagStr))
				.ColorAndOpacity(FLinearColor(1.0f, 0.6f, 1.0f)) // 핑크색
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 8));
		}
	}

	// 2. Boolean
	if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Prop))
	{
		return SNew(SCheckBox)
			.IsChecked_Lambda([WeakObj, BoolProp]() -> ECheckBoxState {
				if(WeakObj.IsValid()) return BoolProp->GetPropertyValue_InContainer(WeakObj.Get()) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
				return ECheckBoxState::Undetermined;
			})
			.IsEnabled(false); 
	}
	
	// 3. Numeric & String
	FString ValStr = TEXT("N/A");
	void* ValPtr = Prop->ContainerPtrToValuePtr<void>(Object);

	if (FNumericProperty* NumProp = CastField<FNumericProperty>(Prop))
	{
		if (NumProp->IsFloatingPoint()) ValStr = FString::Printf(TEXT("%.2f"), NumProp->GetFloatingPointPropertyValue(ValPtr));
		else if (NumProp->IsInteger()) ValStr = FString::Printf(TEXT("%lld"), NumProp->GetSignedIntPropertyValue(ValPtr));
	}
	else if (FStrProperty* StrProp = CastField<FStrProperty>(Prop))
	{
		ValStr = StrProp->GetPropertyValue(ValPtr);
	}
	else if (FNameProperty* NameProp = CastField<FNameProperty>(Prop))
	{
		ValStr = NameProp->GetPropertyValue(ValPtr).ToString();
	}

	return SNew(STextBlock)
		.Text(FText::FromString(ValStr))
		.ColorAndOpacity(FLinearColor::White)
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 8));
}

TSharedRef<SWidget> SActionDebugger::MakeSectionHeader(const FText& HeaderText, FLinearColor Color)
{
	return SNew(SBorder)
		.Padding(FMargin(5.0f, 2.0f))
		.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
		[
			SNew(STextBlock)
			.Text(HeaderText)
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
			.ColorAndOpacity(Color)
		];
}

TSharedRef<SWidget> SActionDebugger::MakeDebugRow(const FText& Label, const FString& ValueStr)
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 10, 0)
		[
			SNew(STextBlock)
			.Text(Label)
			.MinDesiredWidth(100.0f)
			.ColorAndOpacity(FLinearColor::Gray)
		]
		+ SHorizontalBox::Slot().FillWidth(1.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(ValueStr))
			.ColorAndOpacity(FLinearColor::White)
		];
}

#undef LOCTEXT_NAMESPACE