#include "PBBumperCatalogEntryWidget.h"

#include "PBBumperDragDropOperation.h"
#include "PBBumperListItemObject.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/TextBlock.h"
#include "InputCoreTypes.h"

void UPBBumperCatalogEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	BumperListItem = Cast<UPBBumperListItemObject>(ListItemObject);
	if (BumperName && IsValid(BumperListItem))
	{
		BumperName->SetAutoWrapText(false);
		BumperName->SetTextOverflowPolicy(ETextOverflowPolicy::Ellipsis);
		BumperName->SetClipping(EWidgetClipping::ClipToBounds);
		BumperName->SetToolTipText(BumperListItem->DisplayName);
	}

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
