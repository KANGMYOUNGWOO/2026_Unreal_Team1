#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Table/Ball/Struct/PBBallSkillTableRow.h"
#include "Components/ActorComponent.h"
#include "PBBallSkillComponent.generated.h"

class APBBallBase;
class APBBallSkillActorBase;
class AActor;

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

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ball|Skill")
	FPBBallSkillTableRow SkillData;

private:
	int32 CalculateFinalDamage(const APBBallBase* OwnerBall) const;

	UFUNCTION()
	void HandleActiveSkillActorDestroyed(AActor* DestroyedActor);

	UPROPERTY(Transient)
	TObjectPtr<APBBallSkillActorBase> ActiveSkillActor;
};
