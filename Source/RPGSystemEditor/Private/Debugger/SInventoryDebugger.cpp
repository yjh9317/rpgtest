// Source/RPGSystemEditor/Private/Debugger/SInventoryDebugger.cpp

#include "Debugger/SInventoryDebugger.h"
#include "Inventory/InventoryCoreComponent.h"
#include "Item/Data/ItemInstance.h"
#include "Item/Data/ItemDefinition.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Editor/UnrealEdEngine.h"
#include "UnrealEdGlobals.h"
#include "Engine/Selection.h"

#define LOCTEXT_NAMESPACE "SInventoryDebugger"

void SInventoryDebugger::Construct(const FArguments& InArgs)
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
					.Text(LOCTEXT("Title", "Inventory Inspector"))
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

void SInventoryDebugger::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	CheckSelection();
	
	UpdateCounter++;
	if (UpdateCounter % 30 == 0 && CachedInvComp.IsValid()) 
	{
		RebuildContent();
	}
}

void SInventoryDebugger::CheckSelection()
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

	UInventoryCoreComponent* NewComp = SelectedActor ? SelectedActor->FindComponentByClass<UInventoryCoreComponent>() : nullptr;
	if (NewComp != CachedInvComp.Get())
	{
		CachedInvComp = NewComp;
		RebuildContent();
	}
}

void SInventoryDebugger::RebuildContent()
{
	ContentBox->ClearChildren();

	if (!CachedInvComp.IsValid())
	{
		ContentBox->AddSlot().AutoHeight().Padding(5)
		[
			SNew(STextBlock).Text(LOCTEXT("NoComp", "Select Actor with InventoryCoreComponent"))
			.Justification(ETextJustify::Center).ColorAndOpacity(FLinearColor::Gray)
		];
		return;
	}

	ContentBox->AddSlot().AutoHeight()
	[
		MakeSectionHeader(FText::FromString(CachedInvComp->GetOwner()->GetActorLabel()), FLinearColor(0.2f, 1.0f, 0.2f))
	];

	// Reflection을 이용해 "Inventories" (TMap 또는 TArray) 찾기
	// 예시: TMap<FGameplayTag, FInventoryList> 형태라고 가정하거나,
	// 단순히 컴포넌트의 Getter를 사용할 수 있다면 그게 베스트입니다.
	// 여기서는 UInventoryCoreComponent에 "GetAllInventories()" 같은게 있다고 가정하거나
	// "InventoryData" 같은 Array 프로퍼티를 순회합니다.
	
	// 예시: FInventoryList라는 구조체를 가진 프로퍼티 검색
	FProperty* InvListProp = CachedInvComp->GetClass()->FindPropertyByName(TEXT("InventoryData")); // *실제 변수명으로 변경 필요*
	
	if (!InvListProp) 
	{
		// 변수명을 모를 때를 대비한 메시지
		ContentBox->AddSlot().AutoHeight().Padding(5) 
		[ 
			SNew(STextBlock).Text(FText::FromString(TEXT("Cannot find 'InventoryData' property via Reflection"))).ColorAndOpacity(FLinearColor::Red) 
		];
		return;
	}

	// *주의*: 인벤토리 데이터 구조(Struct, Map, Array)에 따라 이 부분 파싱 로직은 달라져야 합니다.
	// 만약 TArray<FInventoryEntry> 형태라면:
	/*
	FArrayProperty* ArrayProp = CastField<FArrayProperty>(InvListProp);
	if (ArrayProp)
	{
		FScriptArrayHelper Helper(ArrayProp, ArrayProp->ContainerPtrToValuePtr<void>(CachedInvComp.Get()));
		for(int32 i=0; i<Helper.Num(); ++i) { ... }
	}
	*/
	
	// 임시: "Getter 함수가 있다"고 가정하고 코드 작성 (가장 깔끔함)
	// 실제 구현시 InventoryCoreComponent.h에 "const TArray<FInventoryItem>& GetItems() const" 같은 함수를 추가하는 것을 추천합니다.
	// 아래는 더미 UI 출력 코드입니다.
	
	ContentBox->AddSlot().AutoHeight().Padding(0, 10, 0, 5)
	[
		MakeSectionHeader(LOCTEXT("InvHeader", "ITEMS"), FLinearColor(0.2f, 0.8f, 1.0f))
	];

	// (예시 데이터)
	// MakeSlotRow(0, "Rusty Sword", 1);
	// MakeSlotRow(1, "Health Potion", 5);
}

TSharedRef<SWidget> SInventoryDebugger::MakeSlotRow(int32 SlotIdx, const FString& ItemName, int32 Quantity)
{
	return SNew(SBorder)
		.Padding(FMargin(5.0f))
		.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
			[
				SNew(STextBlock).Text(FText::AsNumber(SlotIdx)).ColorAndOpacity(FLinearColor::Gray)
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(10, 0)
			[
				SNew(STextBlock).Text(FText::FromString(ItemName)).ColorAndOpacity(FLinearColor::White)
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(STextBlock).Text(FText::Format(LOCTEXT("Qty", "x{0}"), FText::AsNumber(Quantity)))
				.ColorAndOpacity(FLinearColor::Yellow)
			]
		];
}

TSharedRef<SWidget> SInventoryDebugger::MakeSectionHeader(const FText& HeaderText, FLinearColor Color)
{
	return SNew(SBorder).Padding(FMargin(5.0f, 2.0f)).BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
		[ SNew(STextBlock).Text(HeaderText).ColorAndOpacity(Color).Font(FCoreStyle::GetDefaultFontStyle("Bold", 10)) ];
}

#undef LOCTEXT_NAMESPACE