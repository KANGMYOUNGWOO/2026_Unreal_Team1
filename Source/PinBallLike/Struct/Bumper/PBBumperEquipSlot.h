#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Struct/Bumper/PBBumperTriggerSpawnInfo.h"
#include "PinBallLike/Struct/Bumper/PBBumperTypes.h"
#include "PBBumperEquipSlot.generated.h"

UENUM(BlueprintType)
enum class EPBBumperSlotType : uint8
{
	Rebound = 0 UMETA(DisplayName = "Rebound"),
	Side = 1 UMETA(DisplayName = "Side"),
	Special = 2 UMETA(DisplayName = "Special"),
	Top = 3 UMETA(DisplayName = "Top")
};

UENUM(BlueprintType)
enum class EPBBumperEquipSlot : uint8
{
	TopLeft UMETA(DisplayName = "Top Left"),
	TopRight UMETA(DisplayName = "Top Right"),
	SideLeft UMETA(DisplayName = "Side Left"),
	SideRight UMETA(DisplayName = "Side Right"),
	ReboundLeft UMETA(DisplayName = "Rebound Left"),
	ReboundRight UMETA(DisplayName = "Rebound Right"),
	Special UMETA(DisplayName = "Special")
};

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBEquippedBumperSlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Equip")
	EPBBumperEquipSlot EquipSlot = EPBBumperEquipSlot::TopLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Equip")
	FName BumperRowId = NAME_None;
};

namespace PBBumperEquipSlotUtils
{
	PINBALLLIKE_API bool TryGetSlotType(EPBBumperEquipSlot EquipSlot, EPBBumperSlotType& OutSlotType);

	PINBALLLIKE_API EPBBumperEquipSlot GetDefaultEquipSlot(EPBBumperSlotType SlotType);

	PINBALLLIKE_API bool TryGetPositionId(EPBBumperEquipSlot EquipSlot, EPBBumperPositionId& OutPositionId);

	PINBALLLIKE_API bool DoesBumperTypeMatchEquipSlot(EPBBumperType BumperType, EPBBumperEquipSlot EquipSlot);
}
