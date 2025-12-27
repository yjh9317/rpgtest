// Source/RPGSystem/Private/Inventory/UI/PlayerInventoryWidget.cpp

#include "Inventory/UI/PlayerInventoryWidget.h"

#include "Components/SizeBox.h"
#include "Components/UniformGridPanel.h"
#include "Inventory/InventoryCoreComponent.h"
#include "Inventory/InventoryFunctionLibrary.h"
#include "Inventory/UI/PlayerInventorySlotWidget.h"

void UPlayerInventoryWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UPlayerInventoryWidget::NativeDestruct()
{
    if (InventoryComponent)
    {
       InventoryComponent->OnInventoryChanged.RemoveAll(this);
       InventoryComponent->OnItemAdded.RemoveAll(this);
       InventoryComponent->OnItemRemoved.RemoveAll(this);
       InventoryComponent->OnWeightChanged.RemoveAll(this);
    }
    
    Super::NativeDestruct();
}

void UPlayerInventoryWidget::BindDelegateFunc()
{
	InventoryComponent->OnInventoryChanged.AddUObject(this, &UPlayerInventoryWidget::OnSlotChanged);
	// InventoryComponent->OnItemAdded.AddUObject(this, &UPlayerInventoryWidget::OnItemAddedToInventory);
	// InventoryComponent->OnItemRemoved.AddUObject(this, &UPlayerInventoryWidget::OnItemRemovedFromInventory);
	InventoryComponent->OnWeightChanged.AddUObject(this, &UPlayerInventoryWidget::OnWeightUpdated);
}

void UPlayerInventoryWidget::InitializeInventory(UInventoryCoreComponent* InComponent, FGuid InInventoryGuid)
{
    UE_LOG(LogTemp, Log, TEXT("[PlayerInventoryWidget] InitializeInventory (GUID) Called. Target GUID: %s"), *InInventoryGuid.ToString());

    if (!InComponent || !InInventoryGuid.IsValid())
    {
       UE_LOG(LogTemp, Warning, TEXT("[PlayerInventoryWidget] Initialization Aborted: Invalid Component or GUID"));
       return;
    }
    
    InventoryComponent = InComponent;
    CurrentInventoryGuid = InInventoryGuid;
    
    BindDelegateFunc();
    
    const FInventoryMetaData* MetaData = InventoryComponent->GetMetaData(CurrentInventoryGuid);
    if (MetaData)
    {
       UE_LOG(LogTemp, Log, TEXT("[PlayerInventoryWidget] Initialization Success. Creating %d Slots for Inventory: %s"), 
           MetaData->MaxSlots, *MetaData->InventoryName.ToString());

       CreateSlotWidgets(MetaData->MaxSlots);
       RefreshAllSlots();
       UpdateWeightDisplay();
    }
    else
    {
       UE_LOG(LogTemp, Error, TEXT("[PlayerInventoryWidget] Failed to retrieve MetaData for GUID: %s"), *InInventoryGuid.ToString());
    }
}

void UPlayerInventoryWidget::InitializeInventory(UInventoryCoreComponent* InComponent, FName InInventoryName)
{
    UE_LOG(LogTemp, Log, TEXT("[PlayerInventoryWidget] InitializeInventory (Name) Called. Target Name: %s"), *InInventoryName.ToString());

    if (!InComponent || InInventoryName.IsNone())
    {
       UE_LOG(LogTemp, Warning, TEXT("[PlayerInventoryWidget] Initialization Aborted: Invalid Component or Name is None"));
       return;
    }
    
    InventoryComponent = InComponent;
    CurrentInventoryGuid = InventoryComponent->FindInventoryGuid(InInventoryName);
    
    if (!CurrentInventoryGuid.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("[PlayerInventoryWidget] Could not find GUID for Inventory Name: %s"), *InInventoryName.ToString());
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[PlayerInventoryWidget] Found GUID %s for Name %s"), *CurrentInventoryGuid.ToString(), *InInventoryName.ToString());

    BindDelegateFunc();
    
    const FInventoryMetaData* MetaData = InventoryComponent->GetMetaData(CurrentInventoryGuid);
    if (MetaData)
    {
       CreateSlotWidgets(MetaData->MaxSlots);
       RefreshAllSlots();
       UpdateWeightDisplay();
    }
}

void UPlayerInventoryWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	// SizeBox가 연결되어 있다면, 변수 값으로 크기를 강제 설정
	if (Sizebox_Root)
	{
		Sizebox_Root->SetWidthOverride(InventoryTotalWidth);
		// Sizebox_Root->SetHeightOverride(InventoryTotalHeight);
	}
}

void UPlayerInventoryWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
}


void UPlayerInventoryWidget::OnInventoryRefresh(FGuid InventoryGuid)
{
    if (InventoryGuid != CurrentInventoryGuid) return;

    UE_LOG(LogTemp, Log, TEXT("[PlayerInventoryWidget] OnInventoryRefresh Triggered"));
    RefreshAllSlots();
}

void UPlayerInventoryWidget::OnSlotChanged(FGuid InventoryGuid, int32 SlotIndex)
{
    if (InventoryGuid != CurrentInventoryGuid) return;

    if (SlotIndex == -1)
    {
       UE_LOG(LogTemp, Log, TEXT("[PlayerInventoryWidget] Full Refresh Requested (SlotIndex -1)"));
       RefreshAllSlots();
    }
    else
    {
       RefreshSingleSlot(SlotIndex);
    }
}

void UPlayerInventoryWidget::OnItemAddedToInventory(FGuid InventoryGuid, int32 SlotIndex,
    TSubclassOf<UItemDefinition> ItemDefClass)
{
    if (InventoryGuid != CurrentInventoryGuid) return;
    
    UE_LOG(LogTemp, Log, TEXT("[PlayerInventoryWidget] Item Added to Slot %d"), SlotIndex);

    // 1. 슬롯 갱신
    RefreshSingleSlot(SlotIndex);
    
    // 2. 시각 효과 (선택적)
    if (SlotIndex >= 0 && SlotIndex < SlotWidgets.Num())
    {
       // SlotWidgets[SlotIndex]->PlayAddAnimation();
    }
}

void UPlayerInventoryWidget::OnItemRemovedFromInventory(FGuid InventoryGuid, int32 SlotIndex,
    TSubclassOf<UItemDefinition> ItemDefClass)
{
    if (InventoryGuid != CurrentInventoryGuid) return;
    
    UE_LOG(LogTemp, Log, TEXT("[PlayerInventoryWidget] Item Removed from Slot %d"), SlotIndex);

    // 1. 슬롯 갱신
    RefreshSingleSlot(SlotIndex);
    
    // 2. 시각 효과
    if (SlotIndex >= 0 && SlotIndex < SlotWidgets.Num())
    {
       // SlotWidgets[SlotIndex]->PlayRemoveAnimation();
    }
}

void UPlayerInventoryWidget::OnWeightUpdated(FGuid InventoryGuid, float CurrentWeight)
{
    // UE_LOG(LogTemp, Verbose, TEXT("[PlayerInventoryWidget] Weight Updated: %f"), CurrentWeight);
    UpdateWeightDisplay();
}

void UPlayerInventoryWidget::CreateSlotWidgets(int32 SlotCount)
{
	if (!SlotGrid || !SlotWidgetClass) return;
    
	SlotGrid->ClearChildren();
	SlotWidgets.Empty();
    
	// 1. 슬롯 크기 계산
	// 공식: (전체너비 - (여백 * (칸수 - 1))) / 칸수
	// 간단하게는: 전체너비 / 칸수 (여백이 Slot 안에 포함된 경우)
	float CalculatedSlotSize = (InventoryTotalWidth / (float)ColumnsPerRow) - (SlotPadding * 2.0f);
    
	UE_LOG(LogTemp, Log, TEXT("[Inventory] Calculated Slot Size: %f"), CalculatedSlotSize);

	SlotGrid->SetSlotPadding(FMargin(SlotPadding));

	for (int32 i = 0; i < SlotCount; ++i)
	{
		UPlayerInventorySlotWidget* SlotWidget = CreateWidget<UPlayerInventorySlotWidget>(this, SlotWidgetClass);
		if (SlotWidget)
		{
			SlotWidget->SetSlotIndex(i);
			SlotWidget->SetOwningInventoryWidget(this);
     
			SlotWidget->SetSlotSize(CalculatedSlotSize);
            
			const int32 Row = i / ColumnsPerRow;
			const int32 Column = i % ColumnsPerRow;
          
			SlotGrid->AddChildToUniformGrid(SlotWidget, Row, Column);
			SlotWidgets.Add(SlotWidget);
		}
	}
}

void UPlayerInventoryWidget::RefreshAllSlots()
{
    if (!InventoryComponent) return;

    const TArray<FInventorySlot>* Slots = InventoryComponent->GetInventory(CurrentInventoryGuid);
    if (!Slots) 
    {
        UE_LOG(LogTemp, Warning, TEXT("[PlayerInventoryWidget] RefreshAllSlots: Failed to get inventory slots array."));
        return;
    }
    
    // 모든 슬롯 위젯 업데이트
    for (int32 i = 0; i < SlotWidgets.Num(); ++i)
    {
       if (i < Slots->Num())
       {
          SlotWidgets[i]->UpdateSlot((*Slots)[i]);
       }
       else
       {
          SlotWidgets[i]->ClearSlot();
       }
    }
}

void UPlayerInventoryWidget::RefreshSingleSlot(int32 SlotIndex)
{
    if (SlotIndex < 0 || SlotIndex >= SlotWidgets.Num()) 
    {
        UE_LOG(LogTemp, Warning, TEXT("[PlayerInventoryWidget] RefreshSingleSlot: Invalid Slot Index %d"), SlotIndex);
        return;
    }
    
    if (!InventoryComponent) return;

    const FInventorySlot* InventorySlot = InventoryComponent->GetSlot(CurrentInventoryGuid, SlotIndex);
    if (InventorySlot)
    {
       SlotWidgets[SlotIndex]->UpdateSlot(*InventorySlot);
    }
    else
    {
       SlotWidgets[SlotIndex]->ClearSlot();
    }
}

void UPlayerInventoryWidget::UpdateWeightDisplay()
{
    // (기존 코드 주석 처리됨)
}