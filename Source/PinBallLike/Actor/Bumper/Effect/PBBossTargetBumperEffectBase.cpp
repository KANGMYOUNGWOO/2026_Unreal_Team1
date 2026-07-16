#include "PBBossTargetBumperEffectBase.h"

#include "EngineUtils.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/Actor/Boss/PBBossSpawner.h"

namespace
{
	constexpr float MaxBossEffectPower = 1000000.0f;
}

AActor* UPBBossTargetBumperEffectBase::FindBossTarget(const UObject* WorldContext) const
{
	UWorld* World = IsValid(WorldContext) ? WorldContext->GetWorld() : nullptr;
	if (!IsValid(World))
	{
		return nullptr;
	}

	for (TActorIterator<APBBossSpawner> It(World); It; ++It)
	{
		if (APBBossSpawner* BossSpawner = *It)
		{
			if (APBBossBase* SpawnedBoss = BossSpawner->GetSpawnedBoss())
			{
				return SpawnedBoss;
			}
		}
	}

	// 에디터 단독 테스트처럼 Spawner 없이 배치된 보스 본체도 지원한다.
	for (TActorIterator<APBBossBase> It(World); It; ++It)
	{
		if (IsValid(*It))
		{
			return *It;
		}
	}

	return nullptr;
}

bool UPBBossTargetBumperEffectBase::TryResolvePositivePower(int32& OutPower) const
{
	if (!FMath::IsFinite(EffectData.Power) || EffectData.Power <= 0.0f)
	{
		OutPower = 0;
		return false;
	}

	OutPower = FMath::Max(
		FMath::RoundToInt(FMath::Min(EffectData.Power, MaxBossEffectPower)),
		1);
	return true;
}
