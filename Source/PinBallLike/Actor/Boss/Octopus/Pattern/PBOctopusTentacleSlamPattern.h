#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Boss/Pattern/PBBossPatternBase.h"
#include "PBOctopusTentacleSlamPattern.generated.h"

class APBOctopusTentacle;
class APBOctopusTentacleSlamTelegraph;
class UPrimitiveComponent;

UCLASS(Blueprintable)
class PINBALLLIKE_API UPBOctopusTentacleSlamPattern : public UPBBossPatternBase
{
	GENERATED_BODY()

protected:
	virtual bool CanExecute_Implementation(APBBossBase* Boss) const override;
	virtual void StartPattern_Implementation(APBBossBase* Boss) override;
	virtual void ExecutePattern_Implementation(APBBossBase* Boss) override;
	virtual void CancelPatternInternal_Implementation(APBBossBase* Boss) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Pattern|Slam", meta = (ClampMin = "0"))
	int32 TentacleIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Pattern|Slam", meta = (ClampMin = "0.01"))
	float SlamDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Pattern|Slam Damage", meta = (ClampMin = "0"))
	int32 SlamDamage = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Pattern|Slam Damage", meta = (ClampMin = "0.0"))
	float DamageWindowStart = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Pattern|Slam Damage", meta = (ClampMin = "0.0"))
	float DamageWindowDuration = 0.2f;

private:
	void BeginSlamAfterTelegraph();
	float SpawnSlamTelegraph(APBBossBase* Boss, const FVector& SlamDirection);
	const FPBBossPatternTelegraphData* FindSlamTelegraphData() const;
	void StartDamageWindow();
	void FinishDamageWindow();
	void CompleteSlamPattern();
	void CleanupSlamPattern();
	void ClearPatternTimers();

	UFUNCTION()
	void HandleTentacleHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse,
		const FHitResult& Hit);

	UFUNCTION()
	void HandleTentacleDestroyed();

	UPROPERTY(Transient)
	TObjectPtr<APBOctopusTentacle> ActiveTentacle;

	UPROPERTY(Transient)
	TObjectPtr<AActor> TargetBall;

	UPROPERTY(Transient)
	TObjectPtr<APBOctopusTentacleSlamTelegraph> ActiveTelegraph;

	FTimerHandle DamageWindowStartTimerHandle;
	FTimerHandle DamageWindowFinishTimerHandle;
	FTimerHandle PatternFinishTimerHandle;
	FTimerHandle TelegraphFinishTimerHandle;
	bool IsDamageWindowActive = false;
	bool IsDamageApplied = false;
};
