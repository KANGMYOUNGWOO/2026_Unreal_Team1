#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperEffectBase.h"
#include "PBBossTargetBumperEffectBase.generated.h"

class APBBumperProjectile;
enum class EPBBumperProjectilePayload : uint8;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Projectile")
	TSubclassOf<APBBumperProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Projectile")
	FVector ProjectileSpawnOffset = FVector(0.0f, 0.0f, 40.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Effect|Projectile",
		meta = (ClampMin = "0.1", ClampMax = "10.0", Units = "s"))
	float ProjectileLifetime = 3.0f;
};
