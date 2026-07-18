#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "PinBallLike/Struct/Bumper/PBBumperEquipSlot.h"
#include "PBBumperLoadoutSaveGame.generated.h"

/** 플레이어가 선택한 범퍼 물리 슬롯 구성을 버전과 함께 저장합니다. */
UCLASS()
class PINBALLLIKE_API UPBBumperLoadoutSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	static constexpr int32 CurrentSaveVersion = 1;

	UPROPERTY(SaveGame)
	int32 SaveVersion = CurrentSaveVersion;

	UPROPERTY(SaveGame)
	TArray<FPBEquippedBumperSlot> EquippedSlots;
};
