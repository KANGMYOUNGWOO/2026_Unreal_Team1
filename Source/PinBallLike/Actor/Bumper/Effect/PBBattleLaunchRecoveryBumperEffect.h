#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperEffectBase.h"
#include "PBBattleLaunchRecoveryBumperEffect.generated.h"

/** 전투 중 남은 파티 발사 횟수를 Effect 시트의 Power만큼 회복합니다. */
UCLASS(Blueprintable)
class PINBALLLIKE_API UPBBattleLaunchRecoveryBumperEffect : public UPBBumperEffectBase
{
	GENERATED_BODY()

public:
	virtual void ActivateEffectForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;

protected:
	/** 반복 충돌로 발사 횟수를 무한히 비축하지 못하도록 둔 전투 내 상한입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Battle Launch",
		meta = (ClampMin = "1", ClampMax = "99"))
	int32 MaxRemainingLaunchCount = 9;
};
