#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperEffectBase.h"
#include "PinBallLike/Struct/Bumper/PBBumperRewardTypes.h"
#include "PBDirectRewardBumperEffect.generated.h"

UENUM(BlueprintType)
enum class EPBDirectRewardTargetPolicy : uint8
{
	InteractionActor,

	LowestResourceRatioPartyMember,

	InteractionActorThenPartyOverflow
};

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
