// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/UI/PlayerInventorySlotWidget.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Inventory/InventoryCoreComponent.h"
#include "Inventory/DragDrop/ItemDragAndDropWidget.h"
#include "Inventory/DragDrop/InventoryDragDropOperation.h"
#include "Inventory/UI/PlayerInventoryWidget.h"
#include "UI/Context/ContextMenuDefinitions.h"
#include "UI/Context/ContextMenuWidget.h"

void UPlayerInventorySlotWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	if (IsDesignTime()) 
	{
		if (Image_ItemIcon)
		{
			Image_ItemIcon->SetRenderOpacity(0.0f);
		}
		
		if (TextBlock_Quantity)
		{
			TextBlock_Quantity->SetText(FText::FromString("99")); // 테스트용으로 보이게
		}
	}
}

void UPlayerInventorySlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
}


void UPlayerInventorySlotWidget::UpdateSlot(const FInventorySlot& NewSlot)
{
	CachedSlot = NewSlot;
    
	if (NewSlot.IsEmpty())
	{
		ClearSlot();
		return;
	}

	const UItemDefinition* LocalItemDef = NewSlot.GetItemDefinition();
	
	if (Image_ItemIcon)
	{
		if (LocalItemDef && !LocalItemDef->Icon.IsNull())
		{
			UE_LOG(LogTemp, Log, TEXT("[UpdateSlot] Loading Icon for %s: %s"),*LocalItemDef->GetName(), *LocalItemDef->Icon.ToString());

			Image_ItemIcon->SetBrushFromSoftTexture(LocalItemDef->Icon);
			Image_ItemIcon->SetRenderOpacity(1.0f);
			Image_ItemIcon->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[UpdateSlot] Item %s has NO ICON (or ItemDef is null). Hiding Image."), 
				LocalItemDef ? *LocalItemDef->GetName() : TEXT("Null"));

			Image_ItemIcon->SetRenderOpacity(0.0f);
		}
	}
    
	// 수량 텍스트 업데이트
	if (TextBlock_Quantity)
	{
		if (NewSlot.Quantity >= 1)
		{
			TextBlock_Quantity->SetText(FText::AsNumber(NewSlot.Quantity));
			TextBlock_Quantity->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			TextBlock_Quantity->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	
}

void UPlayerInventorySlotWidget::ClearSlot()
{
	CachedSlot.Clear();
    
	if (Image_ItemIcon)
	{
		Image_ItemIcon->SetRenderOpacity(0.0f);
	}
    
	if (TextBlock_Quantity)
	{
		TextBlock_Quantity->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPlayerInventorySlotWidget::SetOwningInventoryWidget(UPlayerInventoryWidget* InWidget)
{
	OwningInventoryWidget = InWidget;
}



void UPlayerInventorySlotWidget::SetSlotSize(float NewSize)
{
	if (SizeBox_Root)
	{
		SizeBox_Root->SetWidthOverride(NewSize);
		SizeBox_Root->SetHeightOverride(NewSize);
	}
}

FReply UPlayerInventorySlotWidget::NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	
	return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UPlayerInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 좌클릭일 때만 롱프레스 체크
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		// // 타이머 시작
		// if (GetWorld())
		// {
		// 	GetWorld()->GetTimerManager().SetTimer(
		// 		LongPressTimerHandle, 
		// 		this, 
		// 		&UPlayerInventorySlotWidget::OnLongPressCompleted, 
		// 		LongPressDuration, 
		// 		false
		// 	);
		// }
		// // 드래그 감지 시작 (롱프레스 실패 시 드래그로 전환되도록)
		return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}
	
	// 우클릭 감지
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (ContextMenuClass)
		{
			UContextMenuWidget* Menu = CreateWidget<UContextMenuWidget>(GetOwningPlayer(), ContextMenuClass);
			if (Menu)
			{
				// 1. 헤더 추가 (예: "Assign to Bag")
				FContextMenuItemData HeaderData;
				HeaderData.Type = EContextMenuItemType::Header;
				HeaderData.Text = FText::FromString("Assign to Bag:");
				Menu->AddMenuItem(HeaderData);

				// 2. 체크박스 추가 (예: "Equipment")
				FContextMenuItemData EquipData;
				EquipData.Type = EContextMenuItemType::Checkbox;
				EquipData.Text = FText::FromString("Equipment");
				EquipData.bIsChecked = true; // 실제로는 Inventory 메타데이터 확인 필요
				
				// 람다로 로직 바인딩 (C++에서 람다 바인딩은 약간 복잡하므로, 여기선 예시만 설명)
				// 실제로는 UObject의 함수를 바인딩해야 합니다.
				// EquipData.OnClicked.BindDynamic(this, &UPlayerInventorySlotWidget::ToggleEquipmentFilter);
				
				Menu->AddMenuItem(EquipData);

				// 3. 구분선 및 닫기
				FContextMenuItemData Separator;
				Separator.Type = EContextMenuItemType::Separator;
				Menu->AddMenuItem(Separator);
				
				FContextMenuItemData Cleanup;
				Cleanup.Text = FText::FromString("Cleanup");
				// Cleanup.OnClicked.BindDynamic(InventoryComp, &UInventoryCoreComponent::SortInventory); // 예시
				Menu->AddMenuItem(Cleanup);

				// 화면에 표시
				Menu->ShowAtMousePosition();
			}
		}
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UPlayerInventorySlotWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// if (GetWorld())
	// {
	// 	GetWorld()->GetTimerManager().ClearTimer(LongPressTimerHandle);
	// }
	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void UPlayerInventorySlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
}

void UPlayerInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
                                                      UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);	
	
	if (CachedSlot.IsEmpty())
	{
		OutOperation = nullptr;
		UE_LOG(LogTemp,Warning,TEXT("NativeOnDragDetected CachedSlot is empty"));
		return;
	}

	UItemDragAndDropWidget* DragVisual = CreateDragVisualWidget();
	if (!DragVisual)
	{
		UE_LOG(LogTemp,Warning,TEXT("NativeOnDragDetected DragVisual is nullptr"));
		return;
	}

	// 4. 드래그 오퍼레이션 생성 및 데이터 주입 (함수 분리)
	UDragDropOperation* DragOp = CreateDragDropOperation(DragVisual);
    
	// 5. 결과 반환
	OutOperation = DragOp;
}

bool UPlayerInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	UInventoryDragDropOperation* RPGOp = Cast<UInventoryDragDropOperation>(InOperation);
	if (!RPGOp || !OwningInventoryWidget) return false;

	FGuid FromGuid = RPGOp->SourceInventoryGuid;
	int32 FromIndex = RPGOp->SourceSlotIndex;
	FGuid ToGuid = OwningInventoryWidget->CurrentInventoryGuid; // 내 인벤토리 GUID
	int32 ToIndex = SlotIndex; // 내 슬롯 인덱스

	UInventoryCoreComponent* CoreComp = OwningInventoryWidget->GetInventoryComponent();
	if (!CoreComp) return false;

	// 자기 자신에게 드랍한 경우 무시
	if (FromGuid == ToGuid && FromIndex == ToIndex) return false;

	// 이미 `InventoryCoreComponent::ServerMoveItemToSlot` 로직에 
	// 빈 곳이면 이동, 있으면 교환/스택 로직이 있으므로 단순히 호출만 하면 됩니다.
    
	// (네트워크 환경이라면 Server RPC 호출 필요)
	CoreComp->ServerMoveItemToSlot(FromGuid, FromIndex, ToGuid, ToIndex);

	return true;
}

bool UPlayerInventorySlotWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	UInventoryDragDropOperation* DragOp = Cast<UInventoryDragDropOperation>(InOperation);
	if (!DragOp) return false;

	UItemDragAndDropWidget* DragVisual = Cast<UItemDragAndDropWidget>(DragOp->DefaultDragVisual);
	if (!DragVisual) return false;

	if (CachedSlot.IsEmpty())
	{
		DragVisual->ShowDropState();
	}
	else
	{
		// 아이템이 있다면 교환 가능 여부 등을 판단 (예: 같은 아이템이면 스택, 아니면 교환)
		// 여기서는 단순하게 '교환(파란색)' 상태로 예시를 듭니다.
		DragVisual->ShowSwapState();
        
		// 만약 특정 조건에서 드랍 불가능하다면:
		// DragVisual->ShowWrongState();
	}

	return true;
}

void UPlayerInventorySlotWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);
}

UItemDragAndDropWidget* UPlayerInventorySlotWidget::CreateDragVisualWidget()
{
	if (!DragVisualClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("DragVisualClass is not set in PlayerInventorySlotWidget"));
		return nullptr;
	}

	UItemDragAndDropWidget* DragVisual = CreateWidget<UItemDragAndDropWidget>(GetOwningPlayer(), DragVisualClass);
	if (!DragVisual) return nullptr;

	UTexture2D* ItemIcon = nullptr;
	const UItemDefinition* ItemDef = CachedSlot.GetItemDefinition();
   
	if (ItemDef && !ItemDef->Icon.IsNull())
	{
		ItemIcon = ItemDef->Icon.LoadSynchronous();
	}

	// 비주얼 초기화 (아이콘 설정, 크기 설정)
	// 크기는 슬롯 크기와 동일하게 하거나 고정값(64x64) 사용
	DragVisual->InitWidget(ItemIcon, FVector2D(64.f, 64.f));

	return DragVisual;
}

UDragDropOperation* UPlayerInventorySlotWidget::CreateDragDropOperation(UItemDragAndDropWidget* DragVisual)
{
	if (!DragDropOperationClass)
	{
		// 기본 클래스로라도 생성
		// return NewObject<UDragDropOperation>(this);
		return nullptr;
	}

	// 오퍼레이션 객체 생성
	UInventoryDragDropOperation* DragOp = NewObject<UInventoryDragDropOperation>(this, DragDropOperationClass);
    
	if (DragOp)
	{
		// 1. 비주얼 연결
		DragOp->DefaultDragVisual = DragVisual;
		DragOp->Pivot = EDragPivot::CenterCenter; // 마우스 중앙에 위치

		// 2. 핵심 데이터 채우기
		// OwningInventoryWidget이 유효한지 확인 필요
		if (OwningInventoryWidget)
		{
			DragOp->SourceInventoryGuid = OwningInventoryWidget->CurrentInventoryGuid;
		}
        
		DragOp->SourceSlotIndex = SlotIndex;
		DragOp->SlotData = CachedSlot;

		// 3. Payload (선택사항: 자신을 넘겨서 나중에 참조 가능하게 함)
		DragOp->Payload = this;
	}

	return DragOp;
}

void UPlayerInventorySlotWidget::OnLongPressCompleted()
{
	UE_LOG(LogTemp, Log, TEXT("Long Press Detected on Slot %d"), SlotIndex);
}
