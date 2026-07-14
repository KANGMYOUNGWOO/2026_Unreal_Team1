#pragma once

#include "CoreMinimal.h"
#include "PBBallSkillActorBase.h"
#include "PBCircularBladeActor.generated.h"

class APBBallBase;
class USceneComponent;
class USphereComponent;
class UPBBallPhysicsComponent;
class UPBTimedAreaDamageComponent;

UCLASS(Blueprintable)
class PINBALLLIKE_API APBCircularBladeActor : public APBBallSkillActorBase
{
	GENERATED_BODY()

public:
	APBCircularBladeActor();

	virtual void InitializeSkill(
		APBBallBase* InOwnerBall,
		const FPBBallSkillTableRow& InSkillData) override;

protected:
	virtual void EnterActiveState() override;
	virtual void EnterFinishingState() override;
	virtual void EnterStoppingState() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball|Skill|Circular Blade")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball|Skill|Circular Blade")
	TObjectPtr<USphereComponent> AttackSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball|Skill|Circular Blade")
	TObjectPtr<UPBTimedAreaDamageComponent> TimedAreaDamageComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ball|Skill|Circular Blade", meta = (ClampMin = "0"))
	float TargetAcceleration = 4500.0f;

private:
	void DeactivateBlade();
	void ApplyTargetAcceleration();

	TWeakObjectPtr<UPBBallPhysicsComponent> BallPhysicsComponent;
	TWeakObjectPtr<AActor> TargetActor;
};
