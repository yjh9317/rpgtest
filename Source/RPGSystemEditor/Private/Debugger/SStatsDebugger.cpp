// Source/RPGSystemEditor/Private/Debugger/SStatsDebugger.cpp

#include "Debugger/SStatsDebugger.h"

#include "Selection.h"
#include "Status/StatsComponent.h" // StatsComponent 헤더 경로 확인 필요
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Input/SSpinBox.h"

#define LOCTEXT_NAMESPACE "SStatsDebugger"

void SStatsDebugger::Construct(const FArguments& InArgs)
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
					.Text(LOCTEXT("Title", "Stats Inspector"))
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

void SStatsDebugger::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	CheckSelection();
    // StatsComponent는 값이 자주 바뀌므로 매번 리빌드하거나, 
    // Slate의 Attribute 바인딩(람다)을 적극 활용하는 것이 좋습니다.
    if (CachedStatsComp.IsValid())
    {
         RebuildContent(); // 간단하게 매번 리빌드 (최적화 필요시 람다 바인딩으로 변경)
    }
}

void SStatsDebugger::CheckSelection()
{
	// (기존 디버거들과 동일한 선택 로직)
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

	UStatsComponent* NewComp = SelectedActor ? SelectedActor->FindComponentByClass<UStatsComponent>() : nullptr;
	if (NewComp != CachedStatsComp.Get())
	{
		CachedStatsComp = NewComp;
		RebuildContent();
	}
}

void SStatsDebugger::RebuildContent()
{
	ContentBox->ClearChildren();

	if (!CachedStatsComp.IsValid())
	{
		ContentBox->AddSlot().AutoHeight().Padding(5)
		[
			SNew(STextBlock).Text(LOCTEXT("NoComp", "Select Actor with StatsComponent"))
            .Justification(ETextJustify::Center).ColorAndOpacity(FLinearColor::Gray)
		];
		return;
	}

	// === Header ===
	ContentBox->AddSlot().AutoHeight()
	[
		MakeSectionHeader(FText::FromString(CachedStatsComp->GetOwner()->GetActorLabel()), FLinearColor(0.2f, 1.0f, 0.2f))
	];

    // === Stats List ===
    // StatsComponent에 등록된 태그들을 가져오는 로직 필요
    // 예: TArray<FGameplayTag> StatTags = CachedStatsComp->GetAllRegisteredStats(); 
    // 만약 함수가 없다면 StatsComponent.h에 "TArray<FGameplayTag> GetRegisteredStats() const;" 추가 필요
    
    // (임시) 예시 태그들로 순회한다고 가정
    TArray<FGameplayTag> StatTags; 
    // StatTags.Add(FGameplayTag::RequestGameplayTag("Stats.Health"));
    // StatTags.Add(FGameplayTag::RequestGameplayTag("Stats.Stamina"));
    // 실제로는 컴포넌트에서 가져와야 함!

    if (StatTags.Num() == 0)
    {
        ContentBox->AddSlot().AutoHeight().Padding(5)
        [
            SNew(STextBlock).Text(LOCTEXT("NoStats", "No Stats Found (Check GetComponent logic)"))
        ];
    }

	for (const FGameplayTag& GameplayTag : StatTags)
	{
        // 값 가져오기 (StatsComponent에 해당 함수들이 있다고 가정)
        // float Base = CachedStatsComp->GetBaseStat(Tag);
        // float Current = CachedStatsComp->GetStat(Tag);
        
        float Base = 100.0f; // Dummy
        float Current = 85.0f; // Dummy

		ContentBox->AddSlot().AutoHeight().Padding(0, 2)
		[
			MakeStatRow(GameplayTag, Base, Current)
		];
	}
}

TSharedRef<SWidget> SStatsDebugger::MakeStatRow(const FGameplayTag& StatTag, float BaseValue, float CurrentValue)
{
    // 현재 값이 베이스보다 낮으면 빨강, 높으면 초록, 같으면 흰색
    FLinearColor ValColor = FLinearColor::White;
    if (CurrentValue < BaseValue) ValColor = FLinearColor(1.0f, 0.4f, 0.4f); // Damaged
    else if (CurrentValue > BaseValue) ValColor = FLinearColor(0.4f, 1.0f, 0.4f); // Buffed

	return SNew(SBorder)
        .Padding(FMargin(5.0f))
        .BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
        [
            SNew(SHorizontalBox)
            // 1. Tag Name
            + SHorizontalBox::Slot().FillWidth(0.4f).VAlign(VAlign_Center)
            [
                SNew(STextBlock)
                .Text(FText::FromString(StatTag.ToString()))
                .Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
                .ColorAndOpacity(FLinearColor::White)
            ]
            // 2. Base Value (ReadOnly)
            + SHorizontalBox::Slot().FillWidth(0.2f).Padding(5, 0).VAlign(VAlign_Center)
            [
                SNew(STextBlock)
                .Text(FText::FromString(FString::Printf(TEXT("Base: %.0f"), BaseValue)))
                .ColorAndOpacity(FLinearColor::Gray)
                .Font(FCoreStyle::GetDefaultFontStyle("Regular", 8))
            ]
            // 3. Current Value (Editable SpinBox)
            + SHorizontalBox::Slot().FillWidth(0.4f).Padding(5, 0)
            [
	            SNew(SBorder)
            	.BorderImage(FAppStyle::GetBrush("NoBrush")) // 테두리는 투명하게
	            .ForegroundColor(ValColor) // 여기서 색상 지정!
            	.Padding(0) // 불필요한 여백 제거
	            [
		            SNew(SSpinBox<float>)
	            	.Value(CurrentValue)
	            	.MinValue(0.0f)
	            	.MaxValue(9999.0f)
	            	.Delta(1.0f)
	            	.OnValueChanged_Lambda([this, StatTag](float NewVal)
	            	{
	            		if (CachedStatsComp.IsValid())
	            		{
	            			// CachedStatsComp->SetStat(StatTag, NewVal);
	            		}
	            	})
	            ]
            ]
        ];
}

TSharedRef<SWidget> SStatsDebugger::MakeSectionHeader(const FText& HeaderText, FLinearColor Color)
{
    // (이전 디버거들과 동일한 코드)
    return SNew(SBorder)
        .Padding(FMargin(5.0f, 2.0f))
        .BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
        [
            SNew(STextBlock).Text(HeaderText).ColorAndOpacity(Color).Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
        ];
}

#undef LOCTEXT_NAMESPACE