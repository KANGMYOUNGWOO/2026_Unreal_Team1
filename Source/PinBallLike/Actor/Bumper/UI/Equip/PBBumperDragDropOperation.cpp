#include "PBBumperDragDropOperation.h"

void UPBBumperDragDropOperation::InitializeBumperDrag(
	const FName InBumperRowName)
{
	BumperRowName = InBumperRowName;
	SourceEquipSlot = EPBBumperEquipSlot::TopLeft;
	bHasSourceEquipSlot = false;
}

void UPBBumperDragDropOperation::InitializeEquippedBumperDrag(
	const FName InBumperRowName,
	const EPBBumperEquipSlot InSourceEquipSlot)
{
	BumperRowName = InBumperRowName;
	SourceEquipSlot = InSourceEquipSlot;
	bHasSourceEquipSlot = true;
	Pivot = EDragPivot::MouseDown;
	Offset = FVector2D::ZeroVector;
}

bool UPBBumperDragDropOperation::IsValidBumperDrag() const
{
	return !BumperRowName.IsNone();
}
