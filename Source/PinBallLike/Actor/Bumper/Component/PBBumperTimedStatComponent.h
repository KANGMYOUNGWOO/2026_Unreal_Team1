#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBBumperTimedStatComponent.generated.h"

class UPBBaseStatComponent;

/**
 * 범퍼가 부여한 임시 스탯 비율 보정을 대상 Actor별로 관리합니다.
 * 같은 SourceId는 중첩하지 않고 지속시간만 갱신하며, 서로 다른 SourceId는 스탯별로 합산합니다.
 */
UCLASS(Transient)
class PINBALLLIKE_API UPBBumperTimedStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBBumperTimedStatComponent();

	/** 보정을 적용하거나 같은 SourceId의 기존 보정을 갱신합니다. */
	bool ApplyTimedAttackPercent(
		FName SourceId,
		float Percent,
		float Duration,
		int32& OutSourceBonus);

	/** Attack 외에 ManaRegen처럼 실제 런타임에 연결된 스탯도 같은 갱신 규칙으로 처리합니다. */
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
