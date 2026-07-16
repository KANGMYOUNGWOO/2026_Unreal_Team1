#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBSummonBumperEffect.h"
#include "PinBallLike/Struct/Bumper/PBBumperRewardTypes.h"
#include "PBGateSupportFieldBumperEffect.generated.h"

/** 관문 앞에 HP, MP, 상태효과 중 하나를 제공하는 한시적 지원 영역을 생성합니다. */
UCLASS(Blueprintable)
class PINBALLLIKE_API UPBGateSupportFieldBumperEffect : public UPBSummonBumperEffect
{
	GENERATED_BODY()

public:
	UPBGateSupportFieldBumperEffect();

	virtual void ActivateEffectForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Support Field")
	EPBBumperRewardType RewardType = EPBBumperRewardType::Resource;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Support Field")
	FName ResourceName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Support Field")
	FName StatusEffectId = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Support Field",
		meta = (ClampMin = "0.5", ClampMax = "60.0", Units = "s"))
	float FieldDuration = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Support Field")
	FLinearColor FieldColor = FLinearColor(0.2f, 1.0f, 0.4f, 1.0f);
};
