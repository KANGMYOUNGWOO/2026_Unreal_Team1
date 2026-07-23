#include "PBBumperEquipSlotDragHandle.h"

#include "PBBumperDragDropOperation.h"
#include "PBBumperEquipUI.h"
#include "PBBumperEquipUIBuilder.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Engine/Texture2D.h"
#include "InputCoreTypes.h"

namespace
{
	constexpr float EquippedSlotIconSize = 72.0f;
}

void UPBBumperEquipSlotDragHandle::InitializeEquipSlot(
	UPBBumperEquipUI* InOwnerEquipUI,
	const EPBBumperEquipSlot InEquipSlot)
{
	OwnerEquipUI = InOwnerEquipUI;
	EquipSlot = InEquipSlot;
}

void UPBBumperEquipSlotDragHandle::SetBumperPresentation(
	const FName InBumperRowName,
	UTexture2D* InIconTexture)
{
	BumperRowName = InBumperRowName;
	IconTexture = InIconTexture;
	ApplyPresentation();
}

TSharedRef<SWidget> UPBBumperEquipSlotDragHandle::RebuildWidget()
{
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	}

	if (!WidgetTree->RootWidget)
	{
		RootSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("RootSizeBox"));
		IconFrame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("IconFrame"));
		IconImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("IconImage"));
		WidgetTree->RootWidget = RootSizeBox;

		RootSizeBox->SetWidthOverride(EquippedSlotIconSize);
		RootSizeBox->SetHeightOverride(EquippedSlotIconSize);
		RootSizeBox->SetClipping(EWidgetClipping::ClipToBounds);
		RootSizeBox->AddChild(IconFrame);

		IconFrame->SetPadding(FMargin(2.0f));
		IconFrame->SetBrush(PBBumperEquipUIBuilder::MakeIconFrameBrush());
		IconFrame->SetBrushColor(FLinearColor::White);
		IconFrame->SetClipping(EWidgetClipping::ClipToBounds);
		IconFrame->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		IconFrame->AddChild(IconImage);

		IconImage->SetColorAndOpacity(FLinearColor::White);
		IconImage->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		RootSizeBox = Cast<USizeBox>(WidgetTree->FindWidget(TEXT("RootSizeBox")));
		IconFrame = Cast<UBorder>(WidgetTree->FindWidget(TEXT("IconFrame")));
		IconImage = Cast<UImage>(WidgetTree->FindWidget(TEXT("IconImage")));
	}

	ApplyPresentation();
	return Super::RebuildWidget();
}

FReply UPBBumperEquipSlotDragHandle::NativeOnPreviewMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton
		|| BumperRowName.IsNone()
		|| !IsValid(IconTexture)
		|| !OwnerEquipUI.IsValid())
	{
		return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
	}

	OwnerEquipUI->SelectBumperEquipSlot(EquipSlot);
	return UWidgetBlueprintLibrary::DetectDragIfPressed(
		InMouseEvent,
		this,
		EKeys::LeftMouseButton).NativeReply;
}

void UPBBumperEquipSlotDragHandle::NativeOnDragDetected(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (BumperRowName.IsNone() || !IsValid(IconTexture) || !OwnerEquipUI.IsValid())
	{
		return;
	}

	UPBBumperDragDropOperation* DragOperation = NewObject<UPBBumperDragDropOperation>(this);
	UPBBumperEquipSlotDragHandle* DragVisual = CreateWidget<UPBBumperEquipSlotDragHandle>(
		this,
		StaticClass());
	if (!IsValid(DragOperation) || !IsValid(DragVisual))
	{
		return;
	}

	DragVisual->InitializeEquipSlot(nullptr, EquipSlot);
	DragVisual->SetBumperPresentation(BumperRowName, IconTexture);
	DragVisual->SetVisibility(ESlateVisibility::HitTestInvisible);
	DragVisual->SetRenderOpacity(0.9f);

	DragOperation->InitializeEquippedBumperDrag(BumperRowName, EquipSlot);
	DragOperation->DefaultDragVisual = DragVisual;
	OutOperation = DragOperation;
}

void UPBBumperEquipSlotDragHandle::NativeOnDragCancelled(
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

	UPBBumperDragDropOperation* DragOperation = Cast<UPBBumperDragDropOperation>(InOperation);
	if (!IsValid(DragOperation)
		|| !DragOperation->HasSourceEquipSlot()
		|| DragOperation->SourceEquipSlot != EquipSlot)
	{
		return;
	}

	DragOperation->SetResult(EPBBumperDragResult::Cancelled);
}

void UPBBumperEquipSlotDragHandle::ApplyPresentation()
{
	const bool bHasPresentation = !BumperRowName.IsNone() && IsValid(IconTexture);
	SetVisibility(bHasPresentation ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

	if (IsValid(IconImage))
	{
		IconImage->SetBrushFromTexture(bHasPresentation ? IconTexture.Get() : nullptr, true);
	}
}
