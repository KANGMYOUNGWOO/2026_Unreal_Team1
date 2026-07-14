#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Ball/Skill/PBBallSkillActorBase.h"
#include "PBSummonBallSkillActor.generated.h"

class APBBallBase;
class APBSummonedBallActor;
class USceneComponent;

UCLASS(Blueprintable)
class PINBALLLIKE_API APBSummonBallSkillActor : public APBBallSkillActorBase
{
	GENERATED_BODY()

public:
	APBSummonBallSkillActor();

	virtual void InitializeSkill(
		APBBallBase* InOwnerBall,
		const FPBBallSkillTableRow& InSkillData) override;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void EnterActiveState() override;
	virtual void EnterStoppingState() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball|Skill|Summon")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ball|Skill|Summon")
	TSubclassOf<APBSummonedBallActor> SummonedBallClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ball|Skill|Summon", meta = (ClampMin = "0.0", Units = "cm"))
	float SpawnRadius = 60.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ball|Skill|Summon", meta = (ClampMin = "0.0", Units = "cm/s"))
	float OutwardVelocity = 300.0f;

private:
	bool SpawnSummonedBalls();
	void DestroySummonedBalls();
	void ClearDurationTimer();
	void HandleDurationFinished();

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<APBSummonedBallActor>> SummonedBalls;

	FTimerHandle DurationTimerHandle;
	int32 SummonedBallDamage = 0;
	float SummonedBallDuration = 0.0f;
	int32 SummonedBallCount = 0;
};
