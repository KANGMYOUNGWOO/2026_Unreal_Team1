#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBBumperReactiveRepairComponent.generated.h"

class UPBBaseResourceComponent;

/**
 * 지정한 시간 동안 HP 감소를 관찰해 제한된 횟수만큼 회복합니다.
 * Resource 구조/최대치 변경과 범퍼가 지불하는 의도적인 비용은 피해 처리에서 제외합니다.
 */
UCLASS(Transient)
class PINBALLLIKE_API UPBBumperReactiveRepairComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBBumperReactiveRepairComponent();

	bool Arm(
		FName ResourceName,
		int32 TriggerCount,
		float HealAmount,
		float Duration);

	/** 범퍼가 지불하는 HP 비용처럼 피해가 아닌 감소를 다음 한 번의 관찰에서 제외합니다. */
	void SuppressNextIntentionalDecrease(FName ResourceName);

	bool IsArmed() const { return bArmed; }

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPBBaseResourceComponent* ResolveResourceComponent();
	void Disarm();
	void HandleResourceStructureChanged(FName ChangedResourceName);
	void HandleResourceCurrentChanged(FName ChangedResourceName, float NewValue);

	TWeakObjectPtr<UPBBaseResourceComponent> CachedResourceComponent;
	FName WatchedResourceName = NAME_None;
	int32 RemainingTriggers = 0;
	float HealPerTrigger = 0.0f;
	float LastCurrentValue = 0.0f;
	int32 SuppressedDecreaseCount = 0;
	bool bArmed = false;
	bool bApplyingHeal = false;
	bool bDelegatesBound = false;
	FTimerHandle ExpirationTimer;
};
