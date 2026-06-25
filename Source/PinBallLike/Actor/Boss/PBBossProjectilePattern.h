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
	virtual bool CanExecute_Implementation(APBBossBase* Boss) const override;
	virtual void ExecutePattern_Implementation(APBBossBase* Boss) override;
	virtual void CancelPatternInternal_Implementation(APBBossBase* Boss) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Projectile")
	TSubclassOf<APBBossProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Projectile", meta = (ClampMin = "0"))
	float ProjectileSpeed = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Projectile", meta = (ClampMin = "0"))
	int32 ProjectileCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Projectile", meta = (ClampMin = "0"))
	float FireIntervalSeconds = 0.2f;

private:
	void FireProjectile();
	void ClearFireTimer();
	FVector GetProjectileSpawnLocation() const;
	FRotator GetProjectileSpawnRotation(const FVector& SpawnLocation) const;
	AActor* FindPinballActor() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Projectile", meta = (AllowPrivateAccess = "true"))
	FVector SpawnOffset = FVector::ZeroVector;

	int32 FiredProjectileCount = 0;
	FTimerHandle FireTimerHandle;
};
