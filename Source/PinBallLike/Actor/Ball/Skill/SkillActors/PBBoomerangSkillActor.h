#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Ball/Skill/PBBallSkillActorBase.h"
#include "PBBoomerangSkillActor.generated.h"

class APBBallBase;
class APBBoomerangActor;
class USceneComponent;

UCLASS(Blueprintable)
class PINBALLLIKE_API APBBoomerangSkillActor : public APBBallSkillActorBase
{
	GENERATED_BODY()

public:
	APBBoomerangSkillActor();

	virtual void InitializeSkill(
		APBBallBase* InOwnerBall,
		const FPBBallSkillTableRow& InSkillData) override;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void EnterActiveState() override;
	virtual void EnterStoppingState() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boomerang")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boomerang")
		TSubclassOf<APBBoomerangActor> SummonActorClass;

private:
	bool SpawnSummonActors();
	void DestroySummonActors();

	UFUNCTION()
	void HandleSummonFinished(AActor* SummonActor);

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<APBBoomerangActor>> SpawnedSummonActors;

	int32 EffectDamage = 0;
	int32 EffectGroggy = 0;
	int32 BounceCount = 1;
	int32 SummonCount = 1;
	float EffectDuration = 1.0f;
};
