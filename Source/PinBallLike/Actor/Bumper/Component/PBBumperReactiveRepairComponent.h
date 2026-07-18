#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBBumperReactiveRepairComponent.generated.h"

class UPBBaseResourceComponent;

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
