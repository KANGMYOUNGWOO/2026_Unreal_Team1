#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBSummonBumperEffect.h"
#include "PinBallLike/Struct/Bumper/PBBumperRewardTypes.h"
#include "PBPickupSpawnBumperEffect.generated.h"

UCLASS(Blueprintable)
class PINBALLLIKE_API UPBPickupSpawnBumperEffect : public UPBSummonBumperEffect
{
	GENERATED_BODY()

public:
	UPBPickupSpawnBumperEffect();

	virtual void ActivateEffectForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Pickup")
	EPBBumperRewardType RewardType = EPBBumperRewardType::Resource;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Pickup")
	FName ResourceName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Pickup")
	FName StatusEffectId = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Pickup",
		meta = (ClampMin = "0.5", ClampMax = "60.0", Units = "s"))
	float PickupLifetime = 8.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Pickup",
		meta = (ClampMin = "0.0", ClampMax = "1000.0", Units = "cm"))
	float SpawnDistance = 150.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Pickup")
	FLinearColor PickupColor = FLinearColor(0.0f, 0.8f, 1.0f, 1.0f);
};
