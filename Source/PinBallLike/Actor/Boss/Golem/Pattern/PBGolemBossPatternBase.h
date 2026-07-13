#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Boss/Golem/PBGolemBossTypes.h"
#include "PinBallLike/Actor/Boss/Pattern/PBBossPatternBase.h"
#include "PBGolemBossPatternBase.generated.h"

class APBGolemBoss;
class APBGolemBossHand;

UCLASS(Abstract, Blueprintable)
class PINBALLLIKE_API UPBGolemBossPatternBase : public UPBBossPatternBase
{
	GENERATED_BODY()

protected:
	APBGolemBoss* GetGolemBoss() const;
	APBGolemBoss* GetGolemBoss(APBBossBase* Boss) const;
	APBGolemBossHand* GetGolemHand(EPBGolemBossHandType HandType) const;
};
