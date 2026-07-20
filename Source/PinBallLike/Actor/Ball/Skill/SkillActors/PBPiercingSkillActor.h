#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Ball/Skill/PBBallSkillActorBase.h"
#include "PBPiercingSkillActor.generated.h"

class UPBStatusEffectComponent;
class UPBInstantDamageComponent;
class UPrimitiveComponent;
class USceneComponent;

UCLASS(Blueprintable)
class PINBALLLIKE_API APBPiercingSkillActor : public APBBallSkillActorBase
{
	GENERATED_BODY()

public:
	APBPiercingSkillActor();
	virtual void InitializeSkill(
		APBBallBase* InOwnerBall,
		const FPBBallSkillTableRow& InSkillData) override;
	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void EnterActiveState() override;
	virtual void EnterFinishingState() override;
	virtual void EnterStoppingState() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball|Skill|Piercing Buff")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ball|Skill|Piercing Buff", meta = (ClampMin = "0.0"))
	float DashSpeed = 2500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ball|Skill|Piercing Attack")
	FName AttackColliderTag = TEXT("PiercingAttack");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ball|Skill|Piercing Effect")
	FRotator EffectRotationOffset = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball|Skill|Piercing Attack")
	TObjectPtr<UPBInstantDamageComponent> DamageComponent;

private:
	APBBallBase* ResolveLeaderBall() const;
	UPrimitiveComponent* ResolveAttackCollider() const;
	bool ActivateAttackCollider();
	void DeactivateAttackCollider();
	void ApplyDamageToTarget(AActor* Target);
	FVector CalculateHitLocation(const AActor* Target) const;
	void DashLeaderBallToBoss();
	void UpdateEffectDirection();
	void UnbindStatusEffectEvent();

	UFUNCTION()
	void HandleAttackOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void HandleStatusEffectRemoved(FName RemovedStatusEffectId, int32 StackCount);

	TWeakObjectPtr<UPBStatusEffectComponent> AppliedStatusEffectComponent;
	TWeakObjectPtr<APBBallBase> PiercingBall;

	UPROPERTY(Transient)
	TObjectPtr<UPrimitiveComponent> AttackCollider;

	TSet<TWeakObjectPtr<AActor>> DamagedTargets;
};
