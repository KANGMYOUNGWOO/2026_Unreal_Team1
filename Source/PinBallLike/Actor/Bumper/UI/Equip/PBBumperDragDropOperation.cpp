#include "PBBumperDragDropOperation.h"

void UPBBumperDragDropOperation::InitializeBumperDrag(
	const FName InBumperRowName)
{
	BumperRowName = InBumperRowName;
	SourceEquipSlot = EPBBumperEquipSlot::TopLeft;
	bHasSourceEquipSlot = false;
	Result = EPBBumperDragResult::Pending;
}

void UPBBumperDragDropOperation::InitializeEquippedBumperDrag(
	const FName InBumperRowName,
	const EPBBumperEquipSlot InSourceEquipSlot)
{
	BumperRowName = InBumperRowName;
	SourceEquipSlot = InSourceEquipSlot;
	bHasSourceEquipSlot = true;
	Result = EPBBumperDragResult::Pending;
	Pivot = EDragPivot::MouseDown;
	Offset = FVector2D::ZeroVector;
}

bool UPBBumperDragDropOperation::IsValidBumperDrag() const
{
	return !BumperRowName.IsNone();
}

bool UPBBumperDragDropOperation::WasDropHandled() const
{
	return Result != EPBBumperDragResult::Pending
		&& Result != EPBBumperDragResult::Cancelled
		&& Result != EPBBumperDragResult::Failed;
}

void UPBBumperDragDropOperation::SetResult(const EPBBumperDragResult InResult)
{
	if (Result == EPBBumperDragResult::Pending)
	{
		Result = InResult;
	}
}
