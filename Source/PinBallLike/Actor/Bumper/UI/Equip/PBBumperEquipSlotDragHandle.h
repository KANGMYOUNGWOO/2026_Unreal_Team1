#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PinBallLike/Struct/Bumper/PBBumperEquipSlot.h"
#include "PBBumperEquipSlotDragHandle.generated.h"

class UBorder;
class UImage;
class USizeBox;
class UTexture2D;
class UPBBumperEquipUI;

UCLASS()
class PINBALLLIKE_API UPBBumperEquipSlotDragHandle : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeEquipSlot(UPBBumperEquipUI* InOwnerEquipUI, EPBBumperEquipSlot InEquipSlot);
	void SetBumperPresentation(FName InBumperRowName, UTexture2D* InIconTexture);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual FReply NativeOnPreviewMouseButtonDown(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent,
		UDragDropOperation*& OutOperation) override;
	virtual void NativeOnDragCancelled(
		const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation) override;

private:
	void ApplyPresentation();

	TWeakObjectPtr<UPBBumperEquipUI> OwnerEquipUI;

	UPROPERTY(Transient)
	TObjectPtr<USizeBox> RootSizeBox;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> IconFrame;

	UPROPERTY(Transient)
	TObjectPtr<UImage> IconImage;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> IconTexture;

	UPROPERTY(Transient)
	FName BumperRowName = NAME_None;

	EPBBumperEquipSlot EquipSlot = EPBBumperEquipSlot::TopLeft;
};
