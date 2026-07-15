#pragma once

#include "CoreMinimal.h"
#include "PBGolemBossPatternBase.h"
#include "PBGolemLaserPattern.generated.h"

class APBGolemLaserWall;

UCLASS(Blueprintable)
class PINBALLLIKE_API UPBGolemLaserPattern : public UPBGolemBossPatternBase
{
	GENERATED_BODY()

protected:
	virtual bool CanExecute_Implementation(APBBossBase* Boss) const override;
	virtual void StartPattern_Implementation(APBBossBase* Boss) override;
	virtual void CancelPatternInternal_Implementation(APBBossBase* Boss) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Pattern|Laser")
	TSubclassOf<APBGolemLaserWall> LaserWallClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Pattern|Laser")
	FVector LaserWallOffset = FVector(300.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Pattern|Laser")
	FRotator LaserWallRotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Pattern|Laser", meta = (ClampMin = "0.0"))
	float LaserWallDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Pattern|Laser", meta = (ClampMin = "0.0", Units = "cm/s"))
	float BounceVelocity = 1500.0f;

private:
	void HandleLaserWallFinished();
	void ClearLaserWallTimer();
	void DestroyLaserWall();
	FVector GetLaserDirection(APBBossBase* Boss) const;
	FTransform GetLaserWallSpawnTransform(APBBossBase* Boss) const;

	UPROPERTY(Transient)
	TObjectPtr<APBGolemLaserWall> SpawnedLaserWall;

	FTimerHandle LaserWallTimerHandle;
};
