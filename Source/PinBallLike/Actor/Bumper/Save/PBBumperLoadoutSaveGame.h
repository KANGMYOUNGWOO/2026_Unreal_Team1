#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "PinBallLike/Struct/Bumper/PBBumperEquipSlot.h"
#include "PBBumperLoadoutSaveGame.generated.h"

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
