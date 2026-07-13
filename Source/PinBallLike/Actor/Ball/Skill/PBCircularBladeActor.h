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

	UFUNCTION(BlueprintCallable, Category = "Ball|Skill|Circular Blade")
	void InitializeBlade(
		APBBallBase* InOwnerBall,
		int32 InDamageAmount,
		float InDuration,
		int32 InDamageCount,
		float InAttackRadius);

protected:
	virtual bool ActivateEffectInternal() override;
	virtual void DeactivateEffectInternal() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball|Skill|Circular Blade")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball|Skill|Circular Blade")
	TObjectPtr<USphereComponent> AttackSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball|Skill|Circular Blade")
	TObjectPtr<UPBTimedAreaDamageComponent> TimedAreaDamageComponent;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Ball|Skill|Circular Blade")
	float AttackRadius = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ball|Skill|Circular Blade", meta = (ClampMin = "0"))
	float TargetAcceleration = 4500.0f;

private:
	void ApplyTargetAcceleration();

	TWeakObjectPtr<UPBBallPhysicsComponent> BallPhysicsComponent;
	TWeakObjectPtr<AActor> TargetActor;
};
