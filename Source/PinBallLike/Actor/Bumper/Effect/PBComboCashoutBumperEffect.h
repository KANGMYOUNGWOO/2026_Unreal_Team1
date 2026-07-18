#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBossTargetBumperEffectBase.h"
#include "PBComboCashoutBumperEffect.generated.h"

/**
 * 충돌한 Ball의 현재 콤보를 설정된 상한까지만 소비하고 콤보 x Effect Power 유도탄을 발사합니다.
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

protected:
	/** 한 번 발동할 때 소비할 수 있는 최대 콤보입니다. 남은 콤보는 다음 선택을 위해 보존합니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Combo Cashout",
		meta = (ClampMin = "1", ClampMax = "1000"))
	int32 MaxComboConsumption = 20;
};
