#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperEffectBase.h"
#include "PinBallLike/Struct/Bumper/PBBumperRewardTypes.h"
#include "PBDirectRewardBumperEffect.generated.h"

/**
 * 생성 아이템을 거치지 않고 충돌한 Actor에 보상을 즉시 적용합니다.
 * 보상 종류와 대상 Resource는 파생 Blueprint가, 보상량은 Effect 시트 Power가 소유합니다.
 */
UCLASS(Blueprintable)
class PINBALLLIKE_API UPBDirectRewardBumperEffect : public UPBBumperEffectBase
{
	GENERATED_BODY()

public:
	virtual void ActivateEffectForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Reward")
	EPBBumperRewardType RewardType = EPBBumperRewardType::Resource;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Reward")
	FName ResourceName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Reward")
	FName StatusEffectId = NAME_None;
};
