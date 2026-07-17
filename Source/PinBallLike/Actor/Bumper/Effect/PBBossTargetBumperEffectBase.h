#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperEffectBase.h"
#include "PBBossTargetBumperEffectBase.generated.h"

class APBBumperProjectile;
enum class EPBBumperProjectilePayload : uint8;

/** 현재 전투의 보스 본체를 찾고, 보스 표적 투사체를 생성하는 범퍼 효과 공통 베이스입니다. */
UCLASS(Abstract)
class PINBALLLIKE_API UPBBossTargetBumperEffectBase : public UPBBumperEffectBase
{
	GENERATED_BODY()

protected:
	AActor* FindBossTarget(const UObject* WorldContext) const;
	bool TryResolvePositivePower(int32& OutPower) const;
	bool SpawnBossProjectile(
		APBModularBumperBase* Bumper,
		AActor* BossTarget,
		EPBBumperProjectilePayload Payload,
		int32 Power,
		float PayloadDuration = 0.0f) const;

	/** 시각 자산은 Blueprint가 지정하고, 명중 판정과 payload 적용은 C++이 담당합니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Projectile")
	TSubclassOf<APBBumperProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Projectile")
	FVector ProjectileSpawnOffset = FVector(0.0f, 0.0f, 40.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Projectile",
		meta = (ClampMin = "0.1", ClampMax = "10.0", Units = "s"))
	float ProjectileLifetime = 3.0f;
};
