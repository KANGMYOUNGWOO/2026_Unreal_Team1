#include "PBBumperEquipSlot.h"

namespace PBBumperEquipSlotUtils
{
	bool TryGetSlotType(
		const EPBBumperEquipSlot EquipSlot,
		EPBBumperSlotType& OutSlotType)
	{
		switch (EquipSlot)
		{
		case EPBBumperEquipSlot::TopLeft:
		case EPBBumperEquipSlot::TopRight:
			OutSlotType = EPBBumperSlotType::Top;
			return true;
		case EPBBumperEquipSlot::SideLeft:
		case EPBBumperEquipSlot::SideRight:
			OutSlotType = EPBBumperSlotType::Side;
			return true;
		case EPBBumperEquipSlot::ReboundLeft:
		case EPBBumperEquipSlot::ReboundRight:
			OutSlotType = EPBBumperSlotType::Rebound;
			return true;
		case EPBBumperEquipSlot::Special:
			OutSlotType = EPBBumperSlotType::Special;
			return true;
		default:
			return false;
		}
	}

	EPBBumperEquipSlot GetDefaultEquipSlot(const EPBBumperSlotType SlotType)
	{
		switch (SlotType)
		{
		case EPBBumperSlotType::Top:
			return EPBBumperEquipSlot::TopLeft;
		case EPBBumperSlotType::Side:
			return EPBBumperEquipSlot::SideLeft;
		case EPBBumperSlotType::Rebound:
			return EPBBumperEquipSlot::ReboundLeft;
		case EPBBumperSlotType::Special:
		default:
			return EPBBumperEquipSlot::Special;
		}
	}

	bool TryGetPositionId(
		const EPBBumperEquipSlot EquipSlot,
		EPBBumperPositionId& OutPositionId)
	{
		switch (EquipSlot)
		{
		case EPBBumperEquipSlot::TopLeft:
			OutPositionId = EPBBumperPositionId::TopTargetLeft;
			return true;
		case EPBBumperEquipSlot::TopRight:
			OutPositionId = EPBBumperPositionId::TopTargetRight;
			return true;
		case EPBBumperEquipSlot::SideLeft:
			OutPositionId = EPBBumperPositionId::SideLeft;
			return true;
		case EPBBumperEquipSlot::SideRight:
			OutPositionId = EPBBumperPositionId::SideRight;
			return true;
		case EPBBumperEquipSlot::ReboundLeft:
			OutPositionId = EPBBumperPositionId::ReboundLeft;
			return true;
		case EPBBumperEquipSlot::ReboundRight:
			OutPositionId = EPBBumperPositionId::ReboundRight;
			return true;
		case EPBBumperEquipSlot::Special:
			OutPositionId = EPBBumperPositionId::GateCenterMid;
			return true;
		default:
			OutPositionId = EPBBumperPositionId::None;
			return false;
		}
	}
}
