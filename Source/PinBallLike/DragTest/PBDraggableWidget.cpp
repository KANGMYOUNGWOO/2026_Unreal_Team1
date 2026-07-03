#include "PBDraggableWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "PBDragDropOperation.h"
#include "../Struct/GamePlayMessage/PBDragMessage.h"
#include "../GamePlayTag/GamePlayTags.h"

FReply UPBDraggableWidget::NativeOnMouseButtonDown(
    const FGeometry& InGeometry,
    const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

    if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
    {
        return UWidgetBlueprintLibrary::DetectDragIfPressed(
            InMouseEvent,
            this,
            EKeys::LeftMouseButton
        ).NativeReply;
    }

    return FReply::Unhandled();
}

void UPBDraggableWidget::NativeOnDragDetected(
    const FGeometry& InGeometry,
    const FPointerEvent& InMouseEvent,
    UDragDropOperation*& OutOperation)
{
    Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

    UPBDragDropOperation* DragOp =
        Cast<UPBDragDropOperation>(
            UWidgetBlueprintLibrary::CreateDragDropOperation(
                UPBDragDropOperation::StaticClass()));

    if (!DragOp)
    {
        return;
    }

    DragOp->ItemId = ItemId;
    DragOp->SourceObject = this;
    DragOp->Payload = this;
    DragOp->DefaultDragVisual = this;
    DragOp->Pivot = EDragPivot::MouseDown;

    OutOperation = DragOp;

    BroadcastDragStarted();
}



void UPBDraggableWidget::BroadcastDragStarted()
{
    FPBDragStartedMessage Message;
    Message.ItemId = ItemId;
    Message.SourceObject = this;

    UGameplayMessageSubsystem::Get(this).BroadcastMessage(
        GameplayTags::Event_UI_Drag_Started,
        Message);

    UE_LOG(LogTemp, Warning, TEXT("Broadcast Drag Started ItemId=%d"), ItemId);
}

