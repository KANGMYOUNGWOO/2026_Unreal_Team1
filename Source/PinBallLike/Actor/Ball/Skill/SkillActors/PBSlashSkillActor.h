#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Ball/Skill/PBBallSkillActorBase.h"
#include "PBSlashSkillActor.generated.h"

class APBBallBase;
class UBoxComponent;
class UPBBallPhysicsComponent;
class UPBInstantDamageComponent;
class UPrimitiveComponent;
class USceneComponent;

UCLASS(Blueprintable)
class PINBALLLIKE_API APBSlashSkillActor : public APBBallSkillActorBase
{
	GENERATED_BODY()

public:
	APBSlashSkillActor();

	virtual void InitializeSkill(
		APBBallBase* InOwnerBall,
		const FPBBallSkillTableRow& InSkillData) override;

	UFUNCTION(BlueprintCallable, Category = "Slash")
	void StartCharge();

	UFUNCTION(BlueprintCallable, Category = "Slash")
	bool StartAttack();

	UFUNCTION(BlueprintCallable, Category = "Slash")
	void EndAttack();

	UFUNCTION(BlueprintPure, Category = "Slash")
	int32 GetAttackCount() const { return AttackCount; }

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void EnterPreparingState() override;
	virtual void EnterFinishingState() override;
	virtual void EnterStoppingState() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Slash")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Slash")
	TObjectPtr<USceneComponent> Pivot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Slash")
	TObjectPtr<UBoxComponent> AttackBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Slash")
	TObjectPtr<UPBInstantDamageComponent> DamageComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Slash", meta = (ClampMin = "0.0"))
	float DashVelocity = 500.0f;

private:
	UFUNCTION()
	void HandleAttackOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	void ApplyDamageToTarget(AActor* Target);
	FVector CalculateHitLocation(const AActor* Target) const;
	void RestoreGravity();

	TWeakObjectPtr<UPBBallPhysicsComponent> BallPhysicsComponent;
	TSet<TWeakObjectPtr<AActor>> DamagedTargets;
	int32 AttackCount = 1;
};
