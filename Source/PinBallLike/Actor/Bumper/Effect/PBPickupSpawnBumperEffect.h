#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBSummonBumperEffect.h"
#include "PinBallLike/Struct/Bumper/PBBumperRewardTypes.h"
#include "PBPickupSpawnBumperEffect.generated.h"

/**
 * 발동한 Trigger 앞에 일정 시간 유지되는 획득 아이템을 생성합니다.
 * 보상 종류는 파생 Effect Blueprint가, 보상량은 Effect 시트의 Power가 소유합니다.
 */
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

	/** Trigger의 전방으로 띄워 아이템이 발동 Ball에 즉시 먹히는 빈도를 줄입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Pickup",
		meta = (ClampMin = "0.0", ClampMax = "1000.0", Units = "cm"))
	float SpawnDistance = 150.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Pickup")
	FLinearColor PickupColor = FLinearColor(0.0f, 0.8f, 1.0f, 1.0f);
};
