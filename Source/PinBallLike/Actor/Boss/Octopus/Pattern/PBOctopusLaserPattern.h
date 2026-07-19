#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Boss/Pattern/PBBossPatternBase.h"
#include "PBOctopusLaserPattern.generated.h"

class APBGolemLaserWall;

UCLASS(Blueprintable)
class PINBALLLIKE_API UPBOctopusLaserPattern : public UPBBossPatternBase
{
	GENERATED_BODY()

public:
	UPBOctopusLaserPattern();

protected:
	virtual bool CanExecute_Implementation(APBBossBase* Boss) const override;
	virtual void ExecutePattern_Implementation(APBBossBase* Boss) override;
	virtual void CancelPatternInternal_Implementation(APBBossBase* Boss) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Pattern|Laser")
	TSubclassOf<APBGolemLaserWall> LaserWallClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Pattern|Laser")
	FVector LaserOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Pattern|Laser", meta = (ClampMin = "0.0", Units = "cm"))
	float LaserSpawnDistance = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Pattern|Laser")
	FRotator LaserRotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Pattern|Laser", meta = (ClampMin = "0.0"))
	float LaserDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Pattern|Laser", meta = (ClampMin = "0.0", Units = "cm/s"))
	float BounceVelocity = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Pattern|Laser", meta = (ClampMin = "0.0", Units = "s"))
	float LaserGrowthDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Pattern|Laser", meta = (ClampMin = "0.01"))
	float InitialLengthScale = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Pattern|Laser", meta = (ClampMin = "0.01"))
	float MaximumLengthScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Pattern|Laser", meta = (Units = "deg/s"))
	float RotationSpeed = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Pattern|Laser", meta = (ClampMin = "0.01", Units = "s"))
	float UpdateInterval = 0.02f;

private:
	void CompleteLaserPattern();
	void UpdateLaserSweep();
	void ClearLaserTimers();
	void DestroyLaserWall();
	FTransform GetLaserSpawnTransform(APBBossBase* Boss) const;

	UPROPERTY(Transient)
	TObjectPtr<APBGolemLaserWall> SpawnedLaserWall;

	FVector InitialLaserScale = FVector::OneVector;
	FRotator InitialLaserRotation = FRotator::ZeroRotator;
	float ElapsedPatternTime = 0.0f;

	FTimerHandle LaserTimerHandle;
	FTimerHandle UpdateTimerHandle;
};
