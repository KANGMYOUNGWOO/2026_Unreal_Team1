#include "PBBumperDragDropOperation.h"

void UPBBumperDragDropOperation::InitializeBumperDrag(
	const FName InBumperRowName)
{
	BumperRowName = InBumperRowName;
}

bool UPBBumperDragDropOperation::IsValidBumperDrag() const
{
	return !BumperRowName.IsNone();
}
