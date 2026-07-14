#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PBBallSkillActorBase.generated.h"

class APBBallBase;
class UPBTimedAreaDamageComponent;

UCLASS(Abstract, Blueprintable)
class PINBALLLIKE_API APBBallSkillActorBase : public AActor
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Ball|Skill|Effect")
	void ActivateEffect();

	UFUNCTION(BlueprintCallable, Category = "Ball|Skill|Effect")
	void DeactivateEffect();

	UFUNCTION(BlueprintCallable, Category = "Ball|Skill|Effect")
	void FinishEffect();

	UFUNCTION(BlueprintCallable, Category = "Ball|Skill|Effect")
	void StopEffect();

	UFUNCTION(BlueprintCallable, Category = "Ball|Skill|Effect")
	void CompleteEffect();

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual bool ActivateEffectInternal();
	virtual void DeactivateEffectInternal();
	virtual AActor* FindTarget() const;
	virtual bool IsTargetValid(const AActor* Target) const;
	virtual bool ShouldWaitForVisualCompletion() const;

	void BindDamageEvents(UPBTimedAreaDamageComponent* DamageComponent);

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Ball|Skill|Effect")
	TObjectPtr<APBBallBase> OwnerBall;

	UFUNCTION(BlueprintImplementableEvent, Category = "Ball|Skill|Effect")
	void BP_OnActivated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ball|Skill|Effect")
	void BP_OnHit(AActor* Target, int32 AppliedDamage, FVector HitLocation);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ball|Skill|Effect")
	void BP_OnFinished();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ball|Skill|Effect")
	void BP_OnStopped();

	UFUNCTION()
	void HandleDamageApplied(AActor* Target, int32 AppliedDamage, FVector HitLocation);

	UFUNCTION()
	void HandleAreaDamageFinished();

	UFUNCTION()
	void HandleOwnerBallDestroyed(AActor* DestroyedActor);

private:
	void UnbindDamageEvents();

	UPROPERTY(Transient)
	TObjectPtr<UPBTimedAreaDamageComponent> BoundDamageComponent;

	bool bIsCompleted = false;
	bool bIsEnding = false;
};
