#include "PBBumperCatalogEntryWidget.h"

#include "PBBumperDragDropOperation.h"
#include "PBBumperListItemObject.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "InputCoreTypes.h"

void UPBBumperCatalogEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	BumperListItem = Cast<UPBBumperListItemObject>(ListItemObject);

	// 기존 WBP_BumperSlotEntry의 OnListItemObjectSet 그래프와 MVVM 주입을 그대로 실행합니다.
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);
}

FReply UPBBumperCatalogEntryWidget::NativeOnPreviewMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (!IsValid(BumperListItem)
		|| BumperListItem->RowName.IsNone()
		|| InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
	}

	// 드래그를 시작해도 상세 정보가 즉시 해당 카드로 바뀌도록 클릭 선택을 먼저 반영합니다.
	BumperListItem->BroadcastClicked();
	return UWidgetBlueprintLibrary::DetectDragIfPressed(
		InMouseEvent,
		this,
		EKeys::LeftMouseButton).NativeReply;
}

void UPBBumperCatalogEntryWidget::NativeOnDragDetected(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (!IsValid(BumperListItem) || BumperListItem->RowName.IsNone())
	{
		return;
	}

	UPBBumperDragDropOperation* DragOperation = NewObject<UPBBumperDragDropOperation>(this);
	if (!IsValid(DragOperation))
	{
		return;
	}

	DragOperation->InitializeBumperDrag(BumperListItem->RowName);
	DragOperation->Pivot = EDragPivot::MouseDown;
	DragOperation->Payload = BumperListItem;

	if (UPBBumperCatalogEntryWidget* DragVisual =
		CreateWidget<UPBBumperCatalogEntryWidget>(this, GetClass()))
	{
		DragVisual->NativeOnListItemObjectSet(BumperListItem);
		DragVisual->SetVisibility(ESlateVisibility::HitTestInvisible);
		DragVisual->SetRenderOpacity(0.88f);
		DragOperation->DefaultDragVisual = DragVisual;
	}

	OutOperation = DragOperation;
}
