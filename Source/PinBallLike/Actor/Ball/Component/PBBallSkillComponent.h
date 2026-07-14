#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBBallSkillComponent.generated.h"

class APBBallSkillActorBase;
class APBCircularBladeActor;
class AActor;
class UPBCircularBladeSkill;

UCLASS(ClassGroup=(PinBall), meta=(BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBallSkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBBallSkillComponent();

	UFUNCTION(BlueprintCallable, Category = "Ball|Skill")
	bool TryActivateSkill();

	UFUNCTION(BlueprintCallable, Category = "Ball|Skill")
	APBBallSkillActorBase* ActivateSkillActor();

	UFUNCTION(BlueprintPure, Category = "Ball|Skill")
	APBBallSkillActorBase* GetActiveSkillActor() const { return ActiveSkillActor; }

	UFUNCTION(BlueprintPure, Category = "Ball|Skill")
	UPBCircularBladeSkill* GetSkill() const { return Skill; }

protected:
	// TODO: Move this test-only skill class selection to skill data.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ball|Skill")
	TSubclassOf<APBCircularBladeActor> CircularBladeActorClass;

private:
	UPBCircularBladeSkill* GetOrCreateSkill();

	UFUNCTION()
	void HandleActiveSkillActorDestroyed(AActor* DestroyedActor);

	UPROPERTY(Transient)
	TObjectPtr<UPBCircularBladeSkill> Skill;

	UPROPERTY(Transient)
	TObjectPtr<APBBallSkillActorBase> ActiveSkillActor;
};
