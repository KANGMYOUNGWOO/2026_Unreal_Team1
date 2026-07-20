#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Ball/Skill/PBBallSkillActorBase.h"
#include "PBSawBladeSkillActor.generated.h"

class APBBallBase;
class APBSawBladeActor;
class USceneComponent;

UCLASS(Blueprintable)
class PINBALLLIKE_API APBSawBladeSkillActor : public APBBallSkillActorBase
{
	GENERATED_BODY()

public:
	APBSawBladeSkillActor();

	virtual void InitializeSkill(
		APBBallBase* InOwnerBall,
		const FPBBallSkillTableRow& InSkillData) override;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void EnterActiveState() override;
	virtual void EnterStoppingState() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SawBlade")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SawBlade")
	TSubclassOf<APBSawBladeActor> SummonActorClass;

private:
	bool SpawnSummonActors();
	void DestroySummonActors();

	UFUNCTION()
	void HandleSummonFinished(AActor* SummonActor);

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<APBSawBladeActor>> SpawnedSummonActors;

	int32 EffectDamage = 0;
	int32 EffectGroggy = 0;
	int32 BounceCount = 1;
	int32 SummonCount = 1;
	float EffectDuration = 0.0f;
};
