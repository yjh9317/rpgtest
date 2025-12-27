// Source/RPGSystemEditor/Private/Debugger/SItemDebugger.cpp

#include "Debugger/SItemDebugger.h"

// Game Classes
#include "Item/WorldItem.h"
#include "Item/Data/ItemInstance.h"
#include "Item/Data/ItemDefinition.h"
#include "Item/Data/Fragment/ItemFragment.h"

// Engine / Editor Headers
#include "Editor/UnrealEdEngine.h"
#include "UnrealEdGlobals.h"
#include "Widgets/Input/SSpinBox.h"
#include "Engine/Selection.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"

#define LOCTEXT_NAMESPACE "SItemDebugger"

void SItemDebugger::Construct(const FArguments& InArgs)
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
				
				// 제목 섹션
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, 10)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Title", "World Item Inspector"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
					.Justification(ETextJustify::Center)
				]

				// 동적 컨텐츠가 들어갈 박스
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(ContentBox, SVerticalBox)
				]
			]
		]
	];
}

void SItemDebugger::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	CheckSelectionAndRefresh();
}

void SItemDebugger::CheckSelectionAndRefresh()
{
	// 1. 현재 선택된 AWorldItem 찾기
	AWorldItem* SelectedItem = nullptr;
	if (GEditor && GEditor->GetSelectedActors())
	{
		// 선택된 액터 중 가장 첫 번째 AWorldItem을 가져옴
		for (FSelectionIterator It(*GEditor->GetSelectedActors()); It; ++It)
		{
			if (AWorldItem* Item = Cast<AWorldItem>(*It))
			{
				SelectedItem = Item;
				break;
			}
		}
	}

	// 2. 변경 사항이 있으면 UI 리빌드
	// - 선택된 아이템이 바뀌었거나
	// - 같은 아이템이지만 내부 Instance가 (생성 등으로 인해) 바뀌었을 때
	UItemInstance* CurrentInstance = SelectedItem ? SelectedItem->GetItemInstance() : nullptr; // Getter 필요, 혹은 protected 접근 필요 시 friend 처리

	// *참고: AWorldItem의 ItemInstance가 protected라면 AWorldItem에 Getter를 추가하거나 이 클래스를 friend로 등록해야 합니다.
	// 여기서는 리플렉션으로 우회해서 가져오는 방식을 예시로 듭니다.
	if (SelectedItem && !CurrentInstance)
	{
		if (FProperty* Prop = AWorldItem::StaticClass()->FindPropertyByName("ItemInstance"))
		{
			if (FObjectProperty* ObjProp = CastField<FObjectProperty>(Prop))
			{
				CurrentInstance = Cast<UItemInstance>(ObjProp->GetObjectPropertyValue_InContainer(SelectedItem));
			}
		}
	}

	bool bSelectionChanged = (CachedTargetItem.Get() != SelectedItem);
	bool bInstanceChanged = (LastInstanceID.Get() != CurrentInstance);

	if (bSelectionChanged || bInstanceChanged)
	{
		CachedTargetItem = SelectedItem;
		LastInstanceID = CurrentInstance;
		RebuildContent();
	}
}

void SItemDebugger::RebuildContent()
{
	ContentBox->ClearChildren();

	if (!CachedTargetItem.IsValid())
	{
		ContentBox->AddSlot().AutoHeight().Padding(5)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("NoSelection", "No WorldItem Selected"))
			.ColorAndOpacity(FLinearColor::Gray)
			.Justification(ETextJustify::Center)
		];
		return;
	}

	// === 1. Target Info ===
	FString ActorLabel = CachedTargetItem->GetActorLabel();
	FString WorldStatus = CachedTargetItem->GetWorld()->IsGameWorld() ? TEXT("(PIE/Game)") : TEXT("(Editor)");
	
	ContentBox->AddSlot().AutoHeight()
	[
		MakeSectionHeader(FText::FromString(FString::Printf(TEXT("%s %s"), *ActorLabel, *WorldStatus)), FLinearColor(0.2f, 1.0f, 0.2f))
	];

	// ---------------------------------------------------------
	// 데이터 가져오기 (Instance 우선, 없으면 Definition 로드)
	// ---------------------------------------------------------
	UItemInstance* Instance = LastInstanceID.Get();
	const UItemDefinition* Def = nullptr;
	bool bIsRuntimeData = (Instance != nullptr);

	if (Instance)
	{
		Def = Instance->GetItemDef();
	}
	else
	{
		// 런타임 인스턴스가 없으면 Reflection으로 DefaultItemDef(SoftPtr) 읽기
		if (FProperty* Prop = AWorldItem::StaticClass()->FindPropertyByName("DefaultItemDef"))
		{
			if (FSoftObjectProperty* SoftProp = CastField<FSoftObjectProperty>(Prop))
			{
				void* ValuePtr = SoftProp->ContainerPtrToValuePtr<void>(CachedTargetItem.Get());
				// SoftObjectPtr 값 가져오기
				FSoftObjectPtr SoftVal = FSoftObjectPtr(SoftProp->GetObjectPropertyValue(ValuePtr));
				// 동기 로드 (에디터이므로 괜찮음)
				Def = Cast<UItemDefinition>(SoftVal.LoadSynchronous());
			}
		}
	}

	// === 2. Item Definition Info ===
	ContentBox->AddSlot().AutoHeight().Padding(0, 10, 0, 0)
	[
		MakeSectionHeader(LOCTEXT("DefHeader", "DEFINITION"), FLinearColor(1.0f, 0.8f, 0.2f))
	];

	FString DefName = Def ? Def->GetName() : TEXT("None (or Null)");
	ContentBox->AddSlot().AutoHeight() [ MakeDebugRow(LOCTEXT("DefLabel", "Item Def:"), DefName) ];

	if (Def)
	{
		ContentBox->AddSlot().AutoHeight() [ MakeDebugRow(LOCTEXT("NameLabel", "Display Name:"), Def->ItemName.ToString()) ];
	}
	else
	{
		ContentBox->AddSlot().AutoHeight().Padding(5)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("NoDef", "No Item Definition Assigned"))
			.ColorAndOpacity(FLinearColor::Red)
		];
		return; // Definition도 없으면 중단
	}

	// === 3. Fragments Info ===
	FLinearColor HeaderColor = bIsRuntimeData ? FLinearColor(0.2f, 0.8f, 1.0f) : FLinearColor(1.0f, 0.5f, 0.2f);
	FString HeaderText = bIsRuntimeData ? TEXT("FRAGMENTS (Runtime)") : TEXT("FRAGMENTS (Asset Default)");

	ContentBox->AddSlot().AutoHeight().Padding(0, 10, 0, 0)
	[
		MakeSectionHeader(FText::FromString(HeaderText), HeaderColor)
	];

	// [CASE A] 런타임 인스턴스가 있을 때
	if (bIsRuntimeData)
	{
		if (Instance->ItemFragments.Entries.Num() == 0)
		{
			ContentBox->AddSlot().AutoHeight().Padding(5)
			[
				SNew(STextBlock).Text(LOCTEXT("NoFrags", "No Runtime Fragments")).ColorAndOpacity(FLinearColor::Gray)
			];
		}
		else
		{
			for (const FItemFragmentEntry& Entry : Instance->ItemFragments.Entries)
			{
				if (Entry.Fragment)
				{
					AddFragmentSection(Entry.Fragment, ContentBox);
				}
			}
		}
	}
	// [CASE B] 에디터 모드 (Definition의 기본 Fragment 보여주기)
	else
	{
		if (Def->Fragments.Num() == 0)
		{
			ContentBox->AddSlot().AutoHeight().Padding(5)
			[
				SNew(STextBlock).Text(LOCTEXT("NoDefFrags", "Definition has no Fragments")).ColorAndOpacity(FLinearColor::Gray)
			];
		}
		else
		{
			for (const auto& FragPtr : Def->Fragments)
			{
				if (FragPtr)
				{
					// 주의: 여기서 수정하면 실제 데이터 에셋(.uasset)이 변경됩니다!
					AddFragmentSection(FragPtr, ContentBox);
				}
			}
			
			// 경고 문구 추가
			ContentBox->AddSlot().AutoHeight().Padding(5)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("AssetWarning", "* Viewing Asset Defaults. Changes here will modify the ItemDefinition asset."))
				.ColorAndOpacity(FLinearColor::Yellow)
				.Font(FCoreStyle::GetDefaultFontStyle("Italic", 9))
			];
		}
	}
}

void SItemDebugger::AddFragmentSection(UItemFragment* Fragment, TSharedPtr<SVerticalBox> Container)
{
	if (!Fragment) return;

	Container->AddSlot().AutoHeight().Padding(0, 5, 0, 2)
	[
		SNew(SBorder)
		.Padding(FMargin(10.0f, 2.0f))
		.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
		[
			SNew(STextBlock)
			.Text(FText::FromString(Fragment->GetClass()->GetName()))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
			.ColorAndOpacity(FLinearColor(0.6f, 0.8f, 1.0f))
		]
	];

	for (TFieldIterator<FProperty> PropIt(Fragment->GetClass()); PropIt; ++PropIt)
	{
		FProperty* Prop = *PropIt;
        
		if (Prop->HasAnyPropertyFlags(CPF_Transient | CPF_DisableEditOnInstance)) continue;
		if (Prop->GetName() == TEXT("ExecutePointer")) continue;

		// [변경] 위젯 생성 함수 호출
		Container->AddSlot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 2)
			[
				SNew(STextBlock)
				.Text(FText::FromName(Prop->GetFName()))
				.MinDesiredWidth(140.0f)
				.ColorAndOpacity(FLinearColor::Gray)
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(10, 2, 0, 2)
			[
				MakePropertyWidget(Prop, Fragment)
			]
		];
	}
}

TSharedRef<SWidget> SItemDebugger::MakeSectionHeader(const FText& HeaderText, FLinearColor Color)
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

TSharedRef<SWidget> SItemDebugger::MakeDebugRow(const FText& Label, const FString& ValueStr)
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0, 2)
		[
			SNew(STextBlock)
			.Text(Label)
			.MinDesiredWidth(140.0f)
			.ColorAndOpacity(FLinearColor::Gray)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(10, 2, 0, 2)
		[
			SNew(STextBlock)
			.Text(FText::FromString(ValueStr))
			.ColorAndOpacity(FLinearColor::White)
			.AutoWrapText(true)
		];
}

TSharedRef<SWidget> SItemDebugger::MakePropertyWidget(FProperty* Prop, UObject* Object)
{
	if (!Prop || !Object) return SNullWidget::NullWidget;

    // 안전을 위해 WeakPtr 사용
    TWeakObjectPtr<UObject> WeakObj = Object;

    // 1. Boolean (체크박스)
    if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Prop))
    {
        return SNew(SCheckBox)
            .IsChecked_Lambda([WeakObj, BoolProp]() -> ECheckBoxState
            {
                if (WeakObj.IsValid())
                {
                    return BoolProp->GetPropertyValue_InContainer(WeakObj.Get()) 
                        ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
                }
                return ECheckBoxState::Undetermined;
            })
            .OnCheckStateChanged_Lambda([WeakObj, BoolProp](ECheckBoxState NewState)
            {
                if (WeakObj.IsValid())
                {
                    // 값 변경 (Setter)
                    BoolProp->SetPropertyValue_InContainer(WeakObj.Get(), NewState == ECheckBoxState::Checked);
                }
            });
    }

    // 2. Float / Double (스핀 박스)
    if (FNumericProperty* NumProp = CastField<FNumericProperty>(Prop))
    {
        if (NumProp->IsFloatingPoint())
        {
            return SNew(SSpinBox<double>)
                .Value_Lambda([WeakObj, NumProp]() -> double
                {
                    if (WeakObj.IsValid())
                        return NumProp->GetFloatingPointPropertyValue(NumProp->ContainerPtrToValuePtr<void>(WeakObj.Get()));
                    return 0.0;
                })
                .OnValueChanged_Lambda([WeakObj, NumProp](double NewValue)
                {
                    if (WeakObj.IsValid())
                        NumProp->SetFloatingPointPropertyValue(NumProp->ContainerPtrToValuePtr<void>(WeakObj.Get()), NewValue);
                })
                .Delta(0.1f); // 증감 단위
        }
        // 3. Integer (스핀 박스)
        else if (NumProp->IsInteger())
        {
            return SNew(SSpinBox<int64>)
                .Value_Lambda([WeakObj, NumProp]() -> int64
                {
                    if (WeakObj.IsValid())
                        return NumProp->GetSignedIntPropertyValue(NumProp->ContainerPtrToValuePtr<void>(WeakObj.Get()));
                    return 0;
                })
                .OnValueChanged_Lambda([WeakObj, NumProp](int64 NewValue)
                {
                    if (WeakObj.IsValid())
                        NumProp->SetIntPropertyValue(NumProp->ContainerPtrToValuePtr<void>(WeakObj.Get()), NewValue);
                })
                .MinFractionalDigits(0); // 정수만 표시
        }
    }

    // 4. String / Name / Text (텍스트 박스)
    if (Prop->IsA<FStrProperty>() || Prop->IsA<FNameProperty>() || Prop->IsA<FTextProperty>())
    {
        return SNew(SEditableTextBox)
            .Text_Lambda([WeakObj, Prop]() -> FText
            {
                if (!WeakObj.IsValid()) return FText::GetEmpty();
                
                void* ValuePtr = Prop->ContainerPtrToValuePtr<void>(WeakObj.Get());
                
                if (FStrProperty* StrProp = CastField<FStrProperty>(Prop))
                    return FText::FromString(StrProp->GetPropertyValue(ValuePtr));
                
                if (FNameProperty* NameProp = CastField<FNameProperty>(Prop))
                    return FText::FromName(NameProp->GetPropertyValue(ValuePtr));
                
                if (FTextProperty* TextProp = CastField<FTextProperty>(Prop))
                    return TextProp->GetPropertyValue(ValuePtr);
                
                return FText::GetEmpty();
            })
            .OnTextCommitted_Lambda([WeakObj, Prop](const FText& NewText, ETextCommit::Type CommitType)
            {
                if (!WeakObj.IsValid()) return;
                
                void* ValuePtr = Prop->ContainerPtrToValuePtr<void>(WeakObj.Get());

                if (FStrProperty* StrProp = CastField<FStrProperty>(Prop))
                    StrProp->SetPropertyValue(ValuePtr, NewText.ToString());
                
                else if (FNameProperty* NameProp = CastField<FNameProperty>(Prop))
                    NameProp->SetPropertyValue(ValuePtr, FName(*NewText.ToString()));
                
                else if (FTextProperty* TextProp = CastField<FTextProperty>(Prop))
                    TextProp->SetPropertyValue(ValuePtr, NewText);
            });
    }

    // 지원하지 않는 타입(Struct, Array 등)은 읽기 전용 텍스트로 표시
    FString ReadOnlyVal = TEXT("(Read Only / Complex Type)");
    // ... (기존 PropertyToString 로직 일부 재사용하여 값 표시 가능) ...
    
    return SNew(STextBlock)
        .Text(FText::FromString(ReadOnlyVal))
        .ColorAndOpacity(FLinearColor::Gray);
}

// FString SItemDebugger::PropertyToString(const FProperty* Prop, const void* ValuePtr) const
// {
// 	if (!Prop || !ValuePtr) return TEXT("Err");
//
// 	if (const FNumericProperty* NumProp = CastField<const FNumericProperty>(Prop))
// 	{
// 		if (NumProp->IsFloatingPoint())
// 			return FString::Printf(TEXT("%.2f"), NumProp->GetFloatingPointPropertyValue(ValuePtr));
// 		if (NumProp->IsInteger())
// 			return FString::Printf(TEXT("%lld"), NumProp->GetSignedIntPropertyValue(ValuePtr));
// 	}
// 	else if (const FBoolProperty* BoolProp = CastField<const FBoolProperty>(Prop))
// 	{
// 		return BoolProp->GetPropertyValue(ValuePtr) ? TEXT("True") : TEXT("False");
// 	}
// 	else if (const FNameProperty* NameProp = CastField<const FNameProperty>(Prop))
// 	{
// 		return NameProp->GetPropertyValue(ValuePtr).ToString();
// 	}
// 	else if (const FStrProperty* StrProp = CastField<const FStrProperty>(Prop))
// 	{
// 		return StrProp->GetPropertyValue(ValuePtr);
// 	}
// 	else if (const FObjectProperty* ObjProp = CastField<const FObjectProperty>(Prop))
// 	{
// 		UObject* Obj = ObjProp->GetObjectPropertyValue(ValuePtr);
// 		return Obj ? Obj->GetName() : TEXT("None");
// 	}
// 	else if (const FStructProperty* StructProp = CastField<const FStructProperty>(Prop))
// 	{
// 		// FGameplayTag 등 구조체 처리
// 		if (StructProp->Struct->GetFName() == FName("GameplayTag"))
// 		{
// 			const FGameplayTag* ItemTag = (const FGameplayTag*)ValuePtr;
// 			return ItemTag->ToString();
// 		}
// 		// FVector
// 		if (StructProp->Struct->GetFName() == FName("Vector"))
// 		{
// 			const FVector* Vec = (const FVector*)ValuePtr;
// 			return Vec->ToString();
// 		}
// 		return FString::Printf(TEXT("{Struct: %s}"), *StructProp->Struct->GetName());
// 	}
// 	else if (const FEnumProperty* EnumProp = CastField<const FEnumProperty>(Prop))
// 	{
// 		// Enum 처리 (내부 Integer Property를 통해 값 얻기)
// 		const FNumericProperty* UnderlyingProp = EnumProp->GetUnderlyingProperty();
// 		int64 Val = UnderlyingProp->GetSignedIntPropertyValue(ValuePtr);
// 		return EnumProp->GetEnum()->GetNameStringByValue(Val);
// 	}
//
// 	return TEXT("(Unsupported Type)");
// }

#undef LOCTEXT_NAMESPACE