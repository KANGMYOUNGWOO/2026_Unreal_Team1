#pragma once

#include "CoreMinimal.h"
#include "PBBossPatternBase.h"
#include "TimerManager.h"
#include "PBBossProjectilePattern.generated.h"

class APBBossProjectile;

UCLASS(Blueprintable)
class PINBALLLIKE_API UPBBossProjectilePattern : public UPBBossPatternBase
{
	GENERATED_BODY()

public:
	UPBBossProjectilePattern();

protected:
	virtual void StartPattern_Implementation(APBBossBase* Boss) override;
	virtual void CancelPattern_Implementation(APBBossBase* Boss) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Projectile")
	TSubclassOf<APBBossProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Projectile", meta = (ClampMin = "0"))
	float ProjectileSpeed = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Projectile", meta = (ClampMin = "0"))
	int32 ProjectileCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Projectile", meta = (ClampMin = "0"))
	float FireIntervalSeconds = 0.2f;

private:
	void StartFireSequence();
	void FireProjectile();
	void ClearFireTimer();
	void ClearTelegraphTimer();
	FVector GetProjectileSpawnLocation() const;
	FRotator GetProjectileSpawnRotation(const FVector& SpawnLocation) const;
	AActor* FindPinballActor() const;

	UPROPERTY(Transient)
	TObjectPtr<APBBossBase> OwnerBoss;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Projectile", meta = (AllowPrivateAccess = "true"))
	FVector SpawnOffset = FVector::ZeroVector;

	int32 FiredProjectileCount = 0;
	FTimerHandle TelegraphTimerHandle;
	FTimerHandle FireTimerHandle;
};
