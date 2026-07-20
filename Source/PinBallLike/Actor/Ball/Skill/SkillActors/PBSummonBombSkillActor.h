#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Ball/Skill/PBBallSkillActorBase.h"
#include "PBSummonBombSkillActor.generated.h"

class APBBallBase;
class APBBombActor;
class USceneComponent;

UCLASS(Blueprintable)
class PINBALLLIKE_API APBSummonBombSkillActor : public APBBallSkillActorBase
{
	GENERATED_BODY()

public:
	APBSummonBombSkillActor();

	virtual void InitializeSkill(
		APBBallBase* InOwnerBall,
		const FPBBallSkillTableRow& InSkillData) override;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void EnterActiveState() override;
	virtual void EnterStoppingState() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bomb")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bomb")
	TSubclassOf<APBBombActor> BombClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bomb", meta = (ClampMin = "0.0"))
	float SpawnRadius = 60.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bomb", meta = (ClampMin = "0.0"))
	float OutwardVelocity = 300.0f;

private:
	bool SpawnBombs();
	void DestroyBombs();

	UFUNCTION()
	void HandleBombFinished(AActor* BombActor);

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<APBBombActor>> SpawnedBombs;

	int32 BombDamage = 0;
	int32 BombGroggy = 0;
	int32 BombCount = 0;
};
