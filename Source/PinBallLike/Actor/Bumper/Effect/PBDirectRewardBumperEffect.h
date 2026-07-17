#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperEffectBase.h"
#include "PinBallLike/Struct/Bumper/PBBumperRewardTypes.h"
#include "PBDirectRewardBumperEffect.generated.h"

/** 즉시 Resource 보상을 파티 안에서 어떤 순서로 배분할지 결정합니다. */
UENUM(BlueprintType)
enum class EPBDirectRewardTargetPolicy : uint8
{
	/** 기존 동작처럼 충돌한 Actor만 대상으로 삼습니다. */
	InteractionActor,

	/** 현재 Resource 비율이 가장 낮은 생존 파티원을 대상으로 삼습니다. */
	LowestResourceRatioPartyMember,

	/** 충돌 Actor를 먼저 회복하고 남은 양을 Resource 비율이 낮은 파티원부터 전달합니다. */
	InteractionActorThenPartyOverflow
};

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Reward")
	EPBDirectRewardTargetPolicy TargetPolicy = EPBDirectRewardTargetPolicy::InteractionActor;
};
