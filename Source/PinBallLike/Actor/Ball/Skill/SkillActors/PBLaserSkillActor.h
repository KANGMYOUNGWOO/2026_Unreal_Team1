#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Ball/Skill/PBBallSkillActorBase.h"
#include "PBLaserSkillActor.generated.h"

class APBBallBase;
class UBoxComponent;
class USceneComponent;
class UPBTimedAreaDamageComponent;

UCLASS(Blueprintable)
class PINBALLLIKE_API APBLaserSkillActor : public APBBallSkillActorBase
{
	GENERATED_BODY()

public:
	APBLaserSkillActor();

	virtual void InitializeSkill(
		APBBallBase* InOwnerBall,
		const FPBBallSkillTableRow& InSkillData) override;

protected:
	virtual void Tick(float DeltaTime) override;
	virtual void EnterActiveState() override;
	virtual void EnterFinishingState() override;
	virtual void EnterStoppingState() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball|Skill|Laser")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball|Skill|Laser")
	TObjectPtr<UBoxComponent> LaserCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball|Skill|Laser")
	TObjectPtr<UPBTimedAreaDamageComponent> TimedAreaDamageComponent;

private:
	void DeactivateLaser();
	void UpdateTargetTracking();

	TWeakObjectPtr<AActor> TargetActor;
};
