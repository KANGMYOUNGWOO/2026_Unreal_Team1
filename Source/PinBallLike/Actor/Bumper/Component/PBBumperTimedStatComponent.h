#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBBumperTimedStatComponent.generated.h"

class UPBBaseStatComponent;

UCLASS(Transient)
class PINBALLLIKE_API UPBBumperTimedStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBBumperTimedStatComponent();

	bool ApplyTimedAttackPercent(
		FName SourceId,
		float Percent,
		float Duration,
		int32& OutSourceBonus);

	bool ApplyTimedStatPercent(
		FName SourceId,
		FName StatName,
		float Percent,
		float Duration,
		int32& OutSourceBonus);

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	struct FActiveStatPercent
	{
		FName StatName = NAME_None;
		float Percent = 0.0f;
		FTimerHandle ExpirationTimer;
	};

	UPBBaseStatComponent* ResolveStatComponent();
	void RecalculateAppliedBonus(FName StatName);
	void ExpireSource(FName SourceId);
	void ClearAllBonuses();

	TMap<FName, FActiveStatPercent> ActiveBonuses;
	TMap<FName, int32> AppliedBonusesByStat;
	TWeakObjectPtr<UPBBaseStatComponent> CachedStatComponent;
};
