#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBossTargetBumperEffectBase.h"
#include "PBComboCashoutBumperEffect.generated.h"

/**
 * 충돌한 Ball의 현재 콤보를 모두 소비하고 콤보 x Effect Power를 담은 유도탄을 발사합니다.
 * 탄환 생성이 실패하면 콤보를 복구하며, 정상 발사 후 빗나간 경우에는 소비가 유지됩니다.
 */
UCLASS(Blueprintable)
class PINBALLLIKE_API UPBComboCashoutBumperEffect : public UPBBossTargetBumperEffectBase
{
	GENERATED_BODY()

public:
	virtual void ActivateEffectForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;
};
