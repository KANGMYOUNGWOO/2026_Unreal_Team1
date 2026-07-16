#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperEffectBase.h"
#include "PBBossTargetBumperEffectBase.generated.h"

/** 현재 전투의 보스 본체를 찾는 범퍼 효과 공통 베이스입니다. */
UCLASS(Abstract)
class PINBALLLIKE_API UPBBossTargetBumperEffectBase : public UPBBumperEffectBase
{
	GENERATED_BODY()

protected:
	AActor* FindBossTarget(const UObject* WorldContext) const;
	bool TryResolvePositivePower(int32& OutPower) const;
};
