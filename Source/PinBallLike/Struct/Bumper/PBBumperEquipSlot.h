#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Struct/Bumper/PBBumperTriggerSpawnInfo.h"
#include "PinBallLike/Struct/Bumper/PBBumperTypes.h"
#include "PBBumperEquipSlot.generated.h"

/** UI 목록을 나누는 범퍼 카테고리입니다. 열거형의 기존 숫자값은 Blueprint 호환을 위해 유지합니다. */
UENUM(BlueprintType)
enum class EPBBumperSlotType : uint8
{
	Rebound = 0 UMETA(DisplayName = "Rebound"),
	Side = 1 UMETA(DisplayName = "Side"),
	Special = 2 UMETA(DisplayName = "Special"),
	Top = 3 UMETA(DisplayName = "Top")
};

/** 플레이어가 범퍼 하나를 장착하는 실제 보드 위치입니다. */
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

/** 전투 스포너에 전달하는 물리 슬롯과 범퍼 RowName의 한 쌍입니다. */
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
	/** 물리 장착 위치가 속한 UI 카테고리를 반환합니다. */
	PINBALLLIKE_API bool TryGetSlotType(EPBBumperEquipSlot EquipSlot, EPBBumperSlotType& OutSlotType);

	/** 카테고리를 처음 선택할 때 사용할 대표 물리 위치를 반환합니다. */
	PINBALLLIKE_API EPBBumperEquipSlot GetDefaultEquipSlot(EPBBumperSlotType SlotType);

	/** 물리 장착 위치를 전투 맵 Anchor의 PositionId로 변환합니다. */
	PINBALLLIKE_API bool TryGetPositionId(EPBBumperEquipSlot EquipSlot, EPBBumperPositionId& OutPositionId);

	/** 범퍼 테이블의 타입이 지정 물리 슬롯에 장착 가능한지 확인합니다. */
	PINBALLLIKE_API bool DoesBumperTypeMatchEquipSlot(EPBBumperType BumperType, EPBBumperEquipSlot EquipSlot);
}
